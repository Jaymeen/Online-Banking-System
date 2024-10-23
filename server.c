#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#define PORT 65000

void GetWelcomeMenuResponse(int socketfd);
void GetCredentialsFromClient(int socketfd, Credentials *creds);
LoginResult VerifyAdminCredentials(int socketfd, Credentials *creds);
void GetAdminMenuResponse(int socketfd);
void AddNewEmployee(int socketfd);
int GetNewIndex(char * filePath);
LoginResult VerifyEmployeeCredentialsAndRole(int socketfd, Credentials * creds, ClientType employeeType);
void GetManagerMenuResponse(int socketfd);
void GetEmployeeMenuResponse(int socketfd);
void AddNewCustomer(int socketfd);
LoginResult VerifyCustomerCredentials(int socketfd, Credentials * creds);
void GetCustomerMenuResponse(int socketfd);
void SendAccountBalance(int socketfd);
void Deposit(int socketfd);
void Withdraw(int socketfd);
void SendTransactionsToCustomer(int socketfd);
void TransferFunds(int socketfd);
void LogTransaction(char * filePath, char * customerId, double amount, TransactionType type);
void ChangePasswordForClient(int socketfd);
void ChangeCustomerDetails(int socketfd);
void ChangeEmployeeDetails(int socketfd);
void ViewCustomerTransactions(int socketfd);
void AddFeedback(int socketfd);
void ViewFeedbacks(int socketfd);
void ApplyForLoan(int socketfd);
void AssignLoansToEmployees(int socketfd);
void ProcessAssignedLoans(int socketfd);
void ViewLoanApplications(int socketfd);

int main() {
	init();

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);

	inet_pton(AF_INET, serveripAddress, &address.sin_addr);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));

    listen(server_fd, 3);

	signal(SIGCHLD, SIG_IGN);

    printf("Waiting for connections on %s:%d...\n", serveripAddress, PORT);

	while(1) {
		new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);

		int processId = fork();

		if(processId == 0) {
			GetWelcomeMenuResponse(new_socket);
			close(new_socket);
			close(server_fd);
			exit(0);
		}
		else {
			close(new_socket);
		}
	}

    return 0;
}

void GetWelcomeMenuResponse(int socketfd) {
	// Waiting for the response from the client
	int responseValue;

	read(socketfd, &responseValue, sizeof(responseValue));

	responseValue = ntohl(responseValue);

	if(responseValue == 5)
		return;

	Credentials creds;
	GetCredentialsFromClient(socketfd, &creds);

	// Responding to the client as per the Selected Menu Item
	switch (responseValue)
	{
		case 1: 
			LoginResult result = VerifyCustomerCredentials(socketfd, &creds);

			if(result != LOGIN_SUCCESSFUL) {
				GetWelcomeMenuResponse(socketfd);
			}
			else {
				GetCustomerMenuResponse(socketfd);
			}

			break;
		
		case 2: 
		{
			LoginResult result = VerifyEmployeeCredentialsAndRole(socketfd, &creds, EMPLOYEE);

			if(result != LOGIN_SUCCESSFUL) {
				GetWelcomeMenuResponse(socketfd);
			}
			else {
				GetEmployeeMenuResponse(socketfd);
			}
		}
		break;
		
		case 3: 
		{
			LoginResult result = VerifyEmployeeCredentialsAndRole(socketfd, &creds, MANAGER);

			if(result != LOGIN_SUCCESSFUL) {
				GetWelcomeMenuResponse(socketfd);
			}
			else {
				GetManagerMenuResponse(socketfd);
			}
		}
		break;
		
		case 4: 
		{
			LoginResult result = VerifyAdminCredentials(socketfd, &creds);

			if(result != LOGIN_SUCCESSFUL) {
				GetWelcomeMenuResponse(socketfd);
			}
			else {
				GetAdminMenuResponse(socketfd);
			}
		}
		break;
		
		default: GetWelcomeMenuResponse(socketfd);
			break;
	}
}

void GetCredentialsFromClient(int socketfd, Credentials *creds) {
	read(socketfd, creds, sizeof(Credentials));
}

LoginResult VerifyAdminCredentials(int socketfd, Credentials *creds) {
	Credentials actualCredentials;

	int fd = open(adminCredentialsFilePath, O_RDONLY);

	read(fd, &actualCredentials, sizeof(actualCredentials));

	close(fd);

	LoginResult result = LOGIN_ID_NOT_FOUND;

	if(strcmp(creds->loginId, actualCredentials.loginId) == 0) {
		if(strcmp(creds->password, actualCredentials.password) == 0) {
			result = LOGIN_SUCCESSFUL;
		}
		else {
			result = PASSWORD_MISMATCH;
		}
	}

	send(socketfd, &result, sizeof(result), 0);

	return result; 
}

void GetAdminMenuResponse(int socketfd) {
	// Waiting for the response from the client
	int responseValue;

	read(socketfd, &responseValue, sizeof(responseValue));

	responseValue = ntohl(responseValue);

	switch(responseValue) {
		case 1: 
			AddNewEmployee(socketfd);
			GetAdminMenuResponse(socketfd);
			break;
		
		case 2:
			ChangeCustomerDetails(socketfd);
			GetAdminMenuResponse(socketfd);
			break;
		
		case 3:
			ChangeEmployeeDetails(socketfd);
			GetAdminMenuResponse(socketfd);
			break;
		
		case 4:
			ChangePasswordForClient(socketfd);
			GetAdminMenuResponse(socketfd);
			break;
		
		default:
			GetWelcomeMenuResponse(socketfd);
			break;
	}
}

void AddNewEmployee(int socketfd) {
	EmployeeInformation employee;

	read(socketfd, &employee, sizeof(EmployeeInformation));

	int currentIndexValue = GetNewIndex(employeeIndexesFilePath);
	
	char currentEmployeeId[14] = "emp-";
	char buffer[10];

	snprintf(buffer, sizeof(buffer), "%d", currentIndexValue);

	strcat(currentEmployeeId, buffer);

	strcpy(employee.userid, currentEmployeeId);

	char currEmployeeDirectoryPath[230];

	strcpy(currEmployeeDirectoryPath, employeesDirectoryPath);
	strcat(currEmployeeDirectoryPath, "/");
	strcat(currEmployeeDirectoryPath, currentEmployeeId);

	mkdir(currEmployeeDirectoryPath, 0755);

	char currEmployeeDetailsFilePath[237];
	char currEmployeeAssignedLoansFilePath[235];

	strcpy(currEmployeeDetailsFilePath, currEmployeeDirectoryPath);
	strcpy(currEmployeeAssignedLoansFilePath, currEmployeeDirectoryPath);

	strcat(currEmployeeDetailsFilePath, "/details");
	strcat(currEmployeeAssignedLoansFilePath, "/loans");

	int fd1 = open(currEmployeeDetailsFilePath, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	int fd2 = creat(currEmployeeAssignedLoansFilePath, S_IRUSR | S_IWUSR);

	AcquireWriteLock(fd1);

	write(fd1, &employee, sizeof(EmployeeInformation));

	UnLockFile(fd1);

	close(fd2);
	close(fd1);

	send(socketfd, employee.userid, 14, 0);
}

int GetNewIndex(char * filePath) {
	int fd = open(filePath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

	AcquireWriteLock(fd);

	int currentIndexValue;

	read(fd, &currentIndexValue, sizeof(currentIndexValue));

	int result = currentIndexValue;

	currentIndexValue = currentIndexValue + 1;

	lseek(fd, 0, SEEK_SET);

	write(fd, &currentIndexValue, sizeof(currentIndexValue));

	UnLockFile(fd);

	close(fd);

	return result;
}

LoginResult VerifyEmployeeCredentialsAndRole(int socketfd, Credentials * creds, ClientType employeeType) {
	char currEmployeeDirectoryPath[230];

	strcpy(currEmployeeDirectoryPath, employeesDirectoryPath);
	strcat(currEmployeeDirectoryPath, "/");
	strcat(currEmployeeDirectoryPath, creds->loginId);

	LoginResult result;
	
	if(access(currEmployeeDirectoryPath, F_OK) == -1) {
		result = LOGIN_ID_NOT_FOUND;

		send(socketfd, &result, sizeof(result), 0);

		return result;
	}

	EmployeeInformation employee;

	char currEmployeeDetailsFilePath[237];

	strcpy(currEmployeeDetailsFilePath, currEmployeeDirectoryPath);
	strcat(currEmployeeDetailsFilePath, "/details");

	int fd = open(currEmployeeDetailsFilePath, O_RDONLY, S_IRUSR | S_IWUSR);

	AcquireReadLock(fd);

	read(fd, &employee, sizeof(EmployeeInformation));

	UnLockFile(fd);

	close(fd);

	if(strcmp(creds->password, employee.password) != 0) {
		result = PASSWORD_MISMATCH;
	}
	else if(employee.employeetype != employeeType) {
		result = ROLE_MISMATCH;
	}
	else {
		result = LOGIN_SUCCESSFUL;
	}

	send(socketfd, &result, sizeof(result), 0);

	return result;
}

void GetManagerMenuResponse(int socketfd) {
	// Waiting for the response from the client
	int responseValue;

	read(socketfd, &responseValue, sizeof(responseValue));

	responseValue = ntohl(responseValue);

	switch(responseValue) {
		case 1: 
			break;
		
		case 2:
			break;
		
		case 3:
			AssignLoansToEmployees(socketfd);
			GetManagerMenuResponse(socketfd);
			break;
		
		case 4:
			ViewFeedbacks(socketfd);
			GetManagerMenuResponse(socketfd);
			break;
		
		case 5:
			ChangePasswordForClient(socketfd);
			GetManagerMenuResponse(socketfd);
			break;
		
		default: 
			GetWelcomeMenuResponse(socketfd);
			break;
	}
}

void GetEmployeeMenuResponse(int socketfd) {
	// Waiting for the response from the client
	int responseValue;

	read(socketfd, &responseValue, sizeof(responseValue));

	responseValue = ntohl(responseValue);

	switch(responseValue) {
		case 1: 
			AddNewCustomer(socketfd);
			GetEmployeeMenuResponse(socketfd);
			break;
		
		case 2:
			ChangeCustomerDetails(socketfd);
			GetEmployeeMenuResponse(socketfd);
			break;
		
		case 4:
			ProcessAssignedLoans(socketfd);
			GetEmployeeMenuResponse(socketfd);
			break;
		
		case 5:
			ViewCustomerTransactions(socketfd);
			GetEmployeeMenuResponse(socketfd);
			break;
		
		case 6:
			ChangePasswordForClient(socketfd);
			GetEmployeeMenuResponse(socketfd);
			break;
		
		default: 
			GetWelcomeMenuResponse(socketfd);
			break;
	}
}

void AddNewCustomer(int socketfd) {
	CustomerInformation customer;

	read(socketfd, &customer, sizeof(CustomerInformation));

	int currentIndexValue = GetNewIndex(customerIndexesFilePath);
	
	char currentCustomerId[14] = "cus-";
	char buffer[10];

	snprintf(buffer, sizeof(buffer), "%d", currentIndexValue);

	strcat(currentCustomerId, buffer);

	strcpy(customer.userid, currentCustomerId);

	char currCustomerDirectoryPath[230];

	strcpy(currCustomerDirectoryPath, customersDirectoryPath);
	strcat(currCustomerDirectoryPath, "/");
	strcat(currCustomerDirectoryPath, currentCustomerId);

	mkdir(currCustomerDirectoryPath, 0755);

	char currCustomerDetailsFilePath[237];
	char currCustomerRequestedLoansFilePath[235];
	char currCustomerTransactionsFilePath[243];

	strcpy(currCustomerDetailsFilePath, currCustomerDirectoryPath);
	strcpy(currCustomerRequestedLoansFilePath, currCustomerDirectoryPath);
	strcpy(currCustomerTransactionsFilePath, currCustomerDirectoryPath);

	strcat(currCustomerDetailsFilePath, "/details");
	strcat(currCustomerRequestedLoansFilePath, "/loans");
	strcat(currCustomerTransactionsFilePath, "/transactions");

	int fd1 = open(currCustomerDetailsFilePath, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	int fd2 = creat(currCustomerRequestedLoansFilePath, S_IRUSR | S_IWUSR);
	int fd3 = creat(currCustomerTransactionsFilePath, S_IRUSR | S_IWUSR);

	AcquireWriteLock(fd1);

	write(fd1, &customer, sizeof(CustomerInformation));

	UnLockFile(fd1);

	close(fd3);
	close(fd2);
	close(fd1);

	send(socketfd, customer.userid, 14, 0);
}

LoginResult VerifyCustomerCredentials(int socketfd, Credentials * creds) {
	char currCustomerDirectoryPath[230];

	strcpy(currCustomerDirectoryPath, customersDirectoryPath);
	strcat(currCustomerDirectoryPath, "/");
	strcat(currCustomerDirectoryPath, creds->loginId);

	LoginResult result;
	
	if(access(currCustomerDirectoryPath, F_OK) == -1) {
		result = LOGIN_ID_NOT_FOUND;

		send(socketfd, &result, sizeof(result), 0);

		return result;
	}

	CustomerInformation customer;

	char currCustomerDetailsFilePath[237];

	strcpy(currCustomerDetailsFilePath, currCustomerDirectoryPath);
	strcat(currCustomerDetailsFilePath, "/details");

	int fd = open(currCustomerDetailsFilePath, O_RDONLY, S_IRUSR | S_IWUSR);

	AcquireReadLock(fd);

	read(fd, &customer, sizeof(CustomerInformation));

	UnLockFile(fd);

	close(fd);

	if(strcmp(creds->password, customer.password) != 0) {
		result = PASSWORD_MISMATCH;
	}
	else {
		result = LOGIN_SUCCESSFUL;
	}

	send(socketfd, &result, sizeof(result), 0);

	return result;
}

void GetCustomerMenuResponse(int socketfd) {
	int responseValue;

	read(socketfd, &responseValue, sizeof(responseValue));

	responseValue = ntohl(responseValue);

	switch(responseValue) {
		case 1: 
			SendAccountBalance(socketfd);
			GetCustomerMenuResponse(socketfd);
			break;
		
		case 2:
			Deposit(socketfd);
			GetCustomerMenuResponse(socketfd);
			break;
		
		case 3:
			Withdraw(socketfd);
			GetCustomerMenuResponse(socketfd);
			break;
		
		case 4:
			TransferFunds(socketfd);
			GetCustomerMenuResponse(socketfd);
			break;
		
		case 5:
			ApplyForLoan(socketfd);
			GetCustomerMenuResponse(socketfd);
			break;
		
		case 6:
			ViewLoanApplications(socketfd);
			GetCustomerMenuResponse(socketfd);
			break;
		
		case 7:
			ChangePasswordForClient(socketfd);
			GetCustomerMenuResponse(socketfd);
			break;
		
		case 8:
			AddFeedback(socketfd);
			GetCustomerMenuResponse(socketfd);
			break;

		case 9:
			SendTransactionsToCustomer(socketfd);
			GetCustomerMenuResponse(socketfd);
			break;
		
		default: 
			GetWelcomeMenuResponse(socketfd);
			break;
	}
}

void SendAccountBalance(int socketfd) {
	char currCustomerDetailsFilePath[237];
	char currentCustomerId[14];
	CustomerInformation customer;

	read(socketfd, currentCustomerId, 14);

	strcpy(currCustomerDetailsFilePath, customersDirectoryPath);
	strcat(currCustomerDetailsFilePath, "/");
	strcat(currCustomerDetailsFilePath, currentCustomerId);
	strcat(currCustomerDetailsFilePath, "/details");

	int fd = open(currCustomerDetailsFilePath, O_RDONLY, S_IRUSR | S_IWUSR);

	AcquireReadLock(fd);

	read(fd, &customer, sizeof(CustomerInformation));

	UnLockFile(fd);

	close(fd);

	send(socketfd, &customer.balance, sizeof(double), 0);
}

void Deposit(int socketfd) {
	char currentCustomerId[14];
	char currCustomerDirectoryPath[230];
	char currCustomerDetailsFilePath[237];
	char currCustomerTransationsFilePath[243];
	CustomerInformation customer;
	double depositAmount;

	read(socketfd, currentCustomerId, 14);

	strcpy(currCustomerDirectoryPath, customersDirectoryPath);
	strcat(currCustomerDirectoryPath, "/");
	strcat(currCustomerDirectoryPath, currentCustomerId);

	strcpy(currCustomerDetailsFilePath, currCustomerDirectoryPath);
	strcat(currCustomerDetailsFilePath, "/details");

	strcpy(currCustomerTransationsFilePath, currCustomerDirectoryPath);
	strcat(currCustomerTransationsFilePath, "/transactions");

	read(socketfd, &depositAmount, sizeof(depositAmount));

	int fd1 = open(currCustomerDetailsFilePath, O_RDWR, S_IRUSR | S_IWUSR);

	AcquireWriteLock(fd1);

	read(fd1, &customer, sizeof(CustomerInformation));

	customer.balance += depositAmount;

	lseek(fd1, 0, SEEK_SET);

	write(fd1, &customer, sizeof(CustomerInformation));
	UnLockFile(fd1);
	close(fd1);

	LogTransaction(currCustomerTransationsFilePath, currentCustomerId, depositAmount, CREDIT);

	send(socketfd, &customer.balance, sizeof(double), 0);	
}

void Withdraw(int socketfd) {
	char currentCustomerId[14];
	char currCustomerDirectoryPath[230];
	char currCustomerDetailsFilePath[237];
	char currCustomerTransationsFilePath[243];
	CustomerInformation customer;
	double withdrawAmount;

	read(socketfd, currentCustomerId, 14);

	strcpy(currCustomerDirectoryPath, customersDirectoryPath);
	strcat(currCustomerDirectoryPath, "/");
	strcat(currCustomerDirectoryPath, currentCustomerId);

	strcpy(currCustomerDetailsFilePath, currCustomerDirectoryPath);
	strcat(currCustomerDetailsFilePath, "/details");

	strcpy(currCustomerTransationsFilePath, currCustomerDirectoryPath);
	strcat(currCustomerTransationsFilePath, "/transactions");

	read(socketfd, &withdrawAmount, sizeof(withdrawAmount));

	int fd1 = open(currCustomerDetailsFilePath, O_RDWR, S_IRUSR | S_IWUSR);

	AcquireWriteLock(fd1);

	read(fd1, &customer, sizeof(CustomerInformation));

	send(socketfd, &customer.balance, sizeof(double), 0);

	if(customer.balance < withdrawAmount) {
		UnLockFile(fd1);
		close(fd1);
		return;
	}

	customer.balance -= withdrawAmount;

	lseek(fd1, 0, SEEK_SET);

	write(fd1, &customer, sizeof(CustomerInformation));

	UnLockFile(fd1);
	close(fd1);

	LogTransaction(currCustomerTransationsFilePath, currentCustomerId, withdrawAmount, DEBIT);

	send(socketfd, &customer.balance, sizeof(customer.balance), 0);
}

void SendTransactionsToCustomer(int socketfd) {
	char currentCustomerId[14];
	char currCustomerTransationsFilePath[243];

	read(socketfd, currentCustomerId, 14);

	strcpy(currCustomerTransationsFilePath, customersDirectoryPath);
	strcat(currCustomerTransationsFilePath, "/");
	strcat(currCustomerTransationsFilePath, currentCustomerId);
	strcat(currCustomerTransationsFilePath, "/transactions");

	int fd = open(currCustomerTransationsFilePath, O_RDONLY, S_IRUSR | S_IWUSR);

	AcquireReadLock(fd);

	int filesize = lseek(fd, 0, SEEK_END);
	int totalTransactions = filesize/sizeof(Transaction);

	lseek(fd, 0, SEEK_SET);

	send(socketfd, &totalTransactions, sizeof(totalTransactions), 0);

	for(int i = 0; i < totalTransactions; i++) {
		Transaction transaction;
		read(fd, &transaction, sizeof(Transaction));
		send(socketfd, &transaction, sizeof(Transaction), 0);
	}

	UnLockFile(fd);
	close(fd);
}

void TransferFunds(int socketfd) {
	char currentCustomerId[14];
	char currCustomerDirectoryPath[230];
	char currCustomerDetailsFilePath[237];
	char currCustomerTransationsFilePath[243];
	char payeeCustomerId[14];
	char payeeCustomerDirectoryPath[230];
	char payeeCustomerDetailsFilePath[243];
	char payeeCustomerTransactionsFilePath[243];
	CustomerInformation customer;
	CustomerInformation payee;
	double transferAmount;
	EntityExistenceResult result;

	read(socketfd, currentCustomerId, 14);

	read(socketfd, payeeCustomerId, 14);

	strcpy(payeeCustomerDirectoryPath, customersDirectoryPath);
	strcat(payeeCustomerDirectoryPath, "/");
	strcat(payeeCustomerDirectoryPath, payeeCustomerId);

	result = DOES_NOT_EXIST;

	if(access(payeeCustomerDirectoryPath, F_OK) == 0) {
		result = EXISTS;
	}

	send(socketfd, &result, sizeof(result), 0);

	if(result != EXISTS) {
		return;
	}

	strcpy(currCustomerDirectoryPath, customersDirectoryPath);
	strcat(currCustomerDirectoryPath, "/");
	strcat(currCustomerDirectoryPath, currentCustomerId);

	strcpy(currCustomerDetailsFilePath, currCustomerDirectoryPath);
	strcat(currCustomerDetailsFilePath, "/details");

	strcpy(currCustomerTransationsFilePath, currCustomerDirectoryPath);
	strcat(currCustomerTransationsFilePath, "/transactions");

	read(socketfd, &transferAmount, sizeof(transferAmount));

	int fd1 = open(currCustomerDetailsFilePath, O_RDWR, S_IRUSR | S_IWUSR);

	AcquireWriteLock(fd1);

	read(fd1, &customer, sizeof(CustomerInformation));

	send(socketfd, &customer.balance, sizeof(double), 0);

	if(customer.balance < transferAmount) {
		UnLockFile(fd1);
		close(fd1);
		return;
	}

	customer.balance -= transferAmount;

	lseek(fd1, 0, SEEK_SET);

	write(fd1, &customer, sizeof(CustomerInformation));

	UnLockFile(fd1);
	close(fd1);

	strcpy(payeeCustomerDirectoryPath, customersDirectoryPath);
	strcat(payeeCustomerDirectoryPath, "/");
	strcat(payeeCustomerDirectoryPath, payeeCustomerId);

	strcpy(payeeCustomerDetailsFilePath, payeeCustomerDirectoryPath);
	strcat(payeeCustomerDetailsFilePath, "/details");

	strcpy(payeeCustomerTransactionsFilePath, payeeCustomerDirectoryPath);
	strcat(payeeCustomerTransactionsFilePath, "/transactions");

	int fd2 = open(payeeCustomerDetailsFilePath, O_RDWR, S_IRUSR | S_IWUSR);

	AcquireWriteLock(fd2);

	read(fd2, &payee, sizeof(CustomerInformation));

	payee.balance += transferAmount;

	lseek(fd2, 0, SEEK_SET);

	write(fd2, &payee, sizeof(CustomerInformation));

	UnLockFile(fd2);
	
	close(fd2);

	LogTransaction(currCustomerTransationsFilePath, payeeCustomerId, transferAmount, DEBIT);
	LogTransaction(payeeCustomerTransactionsFilePath, currentCustomerId, transferAmount, CREDIT);
}

void LogTransaction(char * filePath, char * customerId, double amount, TransactionType type) {
	Transaction transaction;
	long currentTime;
	
	time(&currentTime);
	char * tempString = ctime(&currentTime);

	transaction.transferamount = amount;
	strcpy(transaction.secondparty, customerId);
	strcpy(transaction.time, tempString);
	transaction.type = type;
	
	int fd = open(filePath, O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);

	AcquireWriteLock(fd);

	write(fd, &transaction, sizeof(Transaction));

	UnLockFile(fd);
	close(fd);
}

void ChangePasswordForClient(int socketfd) {
	ClientType clientType;
	char detailsFilePath[237];
	char clientId[14];
	char newPassword[14];

	read(socketfd, &clientType, sizeof(ClientType));

	read(socketfd, clientId, 14);

	read(socketfd, newPassword, 14);

	if(clientType == ADMIN) {
		strcpy(detailsFilePath, adminCredentialsFilePath);

		Credentials creds;

		int fd = open(detailsFilePath, O_RDWR, S_IRUSR | S_IWUSR);

		AcquireWriteLock(fd);

		read(fd, &creds, sizeof(Credentials));

		lseek(fd, 0, SEEK_SET);

		strcpy(creds.password, newPassword);

		write(fd, &creds, sizeof(Credentials));

		UnLockFile(fd);

		close(fd);

		return;
	}

	if(clientType == CUSTOMER) {
		strcpy(detailsFilePath, customersDirectoryPath);
		strcat(detailsFilePath, "/");
		strcat(detailsFilePath, clientId);
		strcat(detailsFilePath, "/");
		strcat(detailsFilePath, "details");

		int fd = open(detailsFilePath, O_RDWR, S_IRUSR | S_IWUSR);

		AcquireWriteLock(fd);

		CustomerInformation customer;

		read(fd, &customer, sizeof(CustomerInformation));

		lseek(fd, 0, SEEK_SET);

		strcpy(customer.password, newPassword);

		write(fd, &customer, sizeof(customer));

		UnLockFile(fd);

		close(fd);
	}
	else if(clientType == MANAGER || clientType == EMPLOYEE) {
		strcpy(detailsFilePath, employeesDirectoryPath);

		strcat(detailsFilePath, "/");
		strcat(detailsFilePath, clientId);
		strcat(detailsFilePath, "/");
		strcat(detailsFilePath, "details");

		int fd = open(detailsFilePath, O_RDWR, S_IRUSR | S_IWUSR);

		AcquireWriteLock(fd);

		EmployeeInformation employee;

		read(fd, &employee, sizeof(EmployeeInformation));

		lseek(fd, 0, SEEK_SET);

		strcpy(employee.password, newPassword);

		write(fd, &employee, sizeof(employee));

		UnLockFile(fd);

		close(fd);		
	}
}

void ChangeCustomerDetails(int socketfd) {
	char customerId[14];
	CustomerInformation customer;
	CustomerInformation oldInfo;
	EntityExistenceResult result = DOES_NOT_EXIST;

	read(socketfd, customerId, 14);

	char customerDetailsFilePath[237];

	strcpy(customerDetailsFilePath, customersDirectoryPath);
	strcat(customerDetailsFilePath, "/");
	strcat(customerDetailsFilePath, customerId);
	strcat(customerDetailsFilePath, "/details");

	if(access(customerDetailsFilePath, F_OK) == -1) {
		send(socketfd, &result, sizeof(EntityExistenceResult), 0);
		return;
	}

	result = EXISTS;

	send(socketfd, &result, sizeof(EntityExistenceResult), 0);

	read(socketfd, &customer, sizeof(CustomerInformation));

	int fd = open(customerDetailsFilePath, O_RDWR, S_IRUSR | S_IWUSR);

	AcquireWriteLock(fd);

	read(fd, &oldInfo, sizeof(CustomerInformation));

	strcpy(oldInfo.personalinformation.fullname, customer.personalinformation.fullname);
	strcpy(oldInfo.personalinformation.email, customer.personalinformation.email);
	strcpy(oldInfo.personalinformation.contact, customer.personalinformation.contact);
	strcpy(oldInfo.password, customer.password);

	lseek(fd, 0, SEEK_SET);

	write(fd, &oldInfo, sizeof(CustomerInformation));

	UnLockFile(fd);

	close(fd);
}

void ChangeEmployeeDetails(int socketfd) {
	char employeeId[14];
	EmployeeInformation employee;
	EmployeeInformation oldInfo;
	EntityExistenceResult result = DOES_NOT_EXIST;

	read(socketfd, employeeId, 14);

	char employeeDetailsFilePath[237];

	strcpy(employeeDetailsFilePath, employeesDirectoryPath);
	strcat(employeeDetailsFilePath, "/");
	strcat(employeeDetailsFilePath, employeeId);
	strcat(employeeDetailsFilePath, "/details");

	if(access(employeeDetailsFilePath, F_OK) == -1) {
		send(socketfd, &result, sizeof(EntityExistenceResult), 0);
		return;
	}

	result = EXISTS;

	send(socketfd, &result, sizeof(EntityExistenceResult), 0);

	read(socketfd, &employee, sizeof(EmployeeInformation));

	int fd = open(employeeDetailsFilePath, O_RDWR, S_IRUSR | S_IWUSR);

	AcquireWriteLock(fd);

	read(fd, &oldInfo, sizeof(EmployeeInformation));

	strcpy(oldInfo.personalinformation.fullname, employee.personalinformation.fullname);
	strcpy(oldInfo.personalinformation.email, employee.personalinformation.email);
	strcpy(oldInfo.personalinformation.contact, employee.personalinformation.contact);
	strcpy(oldInfo.password, employee.password);

	lseek(fd, 0, SEEK_SET);

	write(fd, &oldInfo, sizeof(EmployeeInformation));

	UnLockFile(fd);

	close(fd);
}

void ViewCustomerTransactions(int socketfd) {
	char currentCustomerId[14];
	char currCustomerTransationsFilePath[243];
	EntityExistenceResult result = DOES_NOT_EXIST;

	read(socketfd, currentCustomerId, 14);

	strcpy(currCustomerTransationsFilePath, customersDirectoryPath);
	strcat(currCustomerTransationsFilePath, "/");
	strcat(currCustomerTransationsFilePath, currentCustomerId);
	strcat(currCustomerTransationsFilePath, "/transactions");

	if(access(currCustomerTransationsFilePath, F_OK) == -1) {
		send(socketfd, &result, sizeof(EntityExistenceResult), 0);
		return;
	}

	result = EXISTS;

	send(socketfd, &result, sizeof(EntityExistenceResult), 0);

	int fd = open(currCustomerTransationsFilePath, O_RDONLY, S_IRUSR | S_IWUSR);

	AcquireReadLock(fd);

	int filesize = lseek(fd, 0, SEEK_END);
	int totalTransactions = filesize/sizeof(Transaction);

	lseek(fd, 0, SEEK_SET);

	send(socketfd, &totalTransactions, sizeof(totalTransactions), 0);

	for(int i = 0; i < totalTransactions; i++) {
		Transaction transaction;
		read(fd, &transaction, sizeof(Transaction));
		send(socketfd, &transaction, sizeof(Transaction), 0);
	}

	UnLockFile(fd);
	close(fd);
}

void AddFeedback(int socketfd) {
	Feedback feedback;

	read(socketfd, &feedback, sizeof(Feedback));

	int fd = open(feedbacksFilePath, O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);

	AcquireWriteLock(fd);

	lseek(fd, 0, SEEK_END);

	write(fd, &feedback, sizeof(Feedback));

	UnLockFile(fd);

	close(fd);
}

void ViewFeedbacks(int socketfd) {
	int fd = open(feedbacksFilePath, O_RDONLY, S_IRUSR | S_IWUSR);

	AcquireReadLock(fd);

	int filesize = lseek(fd, 0, SEEK_END);
	int totalFeedbacks = filesize/sizeof(Feedback);

	lseek(fd, 0, SEEK_SET);

	send(socketfd, &totalFeedbacks, sizeof(totalFeedbacks), 0);

	Feedback feedback;

	for(int i = 0; i < totalFeedbacks; i++) {
		read(fd, &feedback, sizeof(Feedback));
		send(socketfd, &feedback, sizeof(Feedback), 0);
	}

	UnLockFile(fd);
	close(fd);
}

void ApplyForLoan(int socketfd) {
	char customerRequestedLoansFilePath[235];
	Loan loan;

	read(socketfd, &loan, sizeof(Loan));

	strcpy(customerRequestedLoansFilePath, customersDirectoryPath);
	strcat(customerRequestedLoansFilePath, "/");
	strcat(customerRequestedLoansFilePath, loan.customerId);
	strcat(customerRequestedLoansFilePath, "/loans");

	int currentIndexValue = GetNewIndex(loanIndexesFilePath);
	
	char currentLoanId[14] = "loan-";
	char buffer[10];

	snprintf(buffer, sizeof(buffer), "%d", currentIndexValue);

	strcat(currentLoanId, buffer);

	strcpy(loan.loanId, currentLoanId);
	long currentTime;
	
	time(&currentTime);
	char * tempString = ctime(&currentTime);

	strcpy(loan.applicationDate, tempString);

	int fd1 = open(allLoansPath, O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);

	AcquireWriteLock(fd1);

	lseek(fd1, 0, SEEK_END);

	write(fd1, &loan, sizeof(Loan));

	UnLockFile(fd1);

	close(fd1);
}

void AssignLoansToEmployees(int socketfd) {
	int fd = open(allLoansPath, O_RDWR, S_IRUSR | S_IWUSR);

	AcquireWriteLock(fd);

	int filesize = lseek(fd, 0, SEEK_END);
	int totalLoans = filesize/sizeof(Loan);

	lseek(fd, 0, SEEK_SET);

	send(socketfd, &totalLoans, sizeof(totalLoans), 0);

	Loan loan;
	char employeeId[14];
	char currEmployeeLoansFilePath[235];
	EntityExistenceResult result;

	for(int i = 0; i < totalLoans; i++) {
		read(fd, &loan, sizeof(Loan));
		send(socketfd, &loan, sizeof(Loan), 0);

		do {
			read(socketfd, employeeId, 14);

			strcpy(currEmployeeLoansFilePath, employeesDirectoryPath);
			strcat(currEmployeeLoansFilePath, "/");
			strcat(currEmployeeLoansFilePath, employeeId);
			strcat(currEmployeeLoansFilePath, "/loans");

			result = DOES_NOT_EXIST;

			if(access(currEmployeeLoansFilePath, F_OK) == -1) {
				send(socketfd, &result, sizeof(EntityExistenceResult), 0);
			}
			else {
				result = EXISTS;
				send(socketfd, &result, sizeof(EntityExistenceResult), 0);
			}
		}
		while(result != EXISTS);

		int fd1 = open(currEmployeeLoansFilePath, O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);

		AcquireWriteLock(fd1);

		lseek(fd1, 0, SEEK_END);

		write(fd1, &loan, sizeof(Loan));

		UnLockFile(fd1);

		close(fd1);
	}

	ftruncate(fd, 0);

	UnLockFile(fd);

	close(fd);
}

void ProcessAssignedLoans(int socketfd) {
	char employeeId[14];
	char employeeLoansFilePath[235];

	read(socketfd, employeeId, 14);

	strcpy(employeeLoansFilePath, employeesDirectoryPath);
	strcat(employeeLoansFilePath, "/");
	strcat(employeeLoansFilePath, employeeId);
	strcat(employeeLoansFilePath, "/loans");

	int fd = open(employeeLoansFilePath, O_RDWR, S_IRUSR | S_IWUSR);

	AcquireWriteLock(fd);

	int filesize = lseek(fd, 0, SEEK_END);
	int totalLoans = filesize/sizeof(Loan);

	lseek(fd, 0, SEEK_SET);

	send(socketfd, &totalLoans, sizeof(totalLoans), 0);

	Loan loan[totalLoans];

	for(int i = 0; i < totalLoans; i++) {
		read(fd, &loan[i], sizeof(Loan));
		send(socketfd, &loan[i], sizeof(Loan), 0);
		read(socketfd, &loan[i], sizeof(Loan));
	}

	ftruncate(fd, 0);

	UnLockFile(fd);

	close(fd);

	char customerDetailsFilePath[235];
	char customerLoansFilePath[235];
	char customerTransactionsFilePath[235];

	for(int i = 0; i < totalLoans; i++) {
		strcpy(customerDetailsFilePath, customersDirectoryPath);
		strcat(customerDetailsFilePath, "/");
		strcat(customerDetailsFilePath, loan[i].customerId);
		strcpy(customerLoansFilePath, customerDetailsFilePath);
		strcpy(customerTransactionsFilePath, customerDetailsFilePath);
		strcat(customerDetailsFilePath, "/details");
		strcat(customerLoansFilePath, "/loans");
		strcat(customerTransactionsFilePath, "/transactions");

		int fd = open(customerLoansFilePath, O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);

		AcquireWriteLock(fd);

		lseek(fd, 0, SEEK_END);

		write(fd, &loan[i], sizeof(Loan));

		UnLockFile(fd);

		close(fd);

		if(loan[i].result == APPROVED) {
			CustomerInformation customer;
			int fd2 = open(customerDetailsFilePath, O_RDWR, S_IRUSR | S_IWUSR);

			AcquireWriteLock(fd2);

			read(fd2, &customer, sizeof(CustomerInformation));

			lseek(fd2, 0, SEEK_SET);

			customer.balance += loan[i].amount;

			write(fd2, &customer, sizeof(CustomerInformation));

			UnLockFile(fd2);

			close(fd2);

			LogTransaction(customerTransactionsFilePath, loan[i].customerId, customer.balance, LOAN);
		}
	}
}

void ViewLoanApplications(int socketfd) {
	char customerId[14];
	char customerLoansFilePath[235];

	read(socketfd, customerId, 14);

	strcpy(customerLoansFilePath, customersDirectoryPath);
	strcat(customerLoansFilePath, "/");
	strcat(customerLoansFilePath, customerId);
	strcat(customerLoansFilePath, "/loans");

	int fd = open(customerLoansFilePath, O_RDONLY, S_IRUSR | S_IWUSR);

	AcquireReadLock(fd);

	int filesize = lseek(fd, 0, SEEK_END);
	int totalLoans = filesize/sizeof(Loan);

	lseek(fd, 0, SEEK_SET);

	send(socketfd, &totalLoans, sizeof(totalLoans), 0);

	Loan loan;

	for(int i = 0; i < totalLoans; i++) {
		read(fd, &loan, sizeof(Loan));
		send(socketfd, &loan, sizeof(Loan), 0);
	}

	UnLockFile(fd);

	close(fd);
}

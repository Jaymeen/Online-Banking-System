#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PORT 65000
#define BUFFER_SIZE 1024

char buffer[BUFFER_SIZE];

void GetWelcomeMenuResponse(int socketfd);
void GetCredentialsFromClient(int socketfd, Credentials *creds);
LoginResult VerifyAdminCredentials(int socketfd, Credentials *creds);
void GetAdminMenuResponse(int socketfd);
void AddNewEmployee(int socketfd);
int GetNewIndex(char * filePath);
LoginResult VerifyEmployeeCredentialsAndRole(int socketfd, Credentials * creds, EmployeeType employeeType);
void GetManagerMenuResponse(int socketfd);
void GetEmployeeMenuResponse(int socketfd);
void AddNewCustomer(int socketfd);
LoginResult VerifyCustomerCredentials(int socketfd, Credentials * creds);
void GetCustomerMenuResponse(int socketfd);

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

	send(socketfd, employee.userid, strlen(employee.userid), 0);
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

LoginResult VerifyEmployeeCredentialsAndRole(int socketfd, Credentials * creds, EmployeeType employeeType) {
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

	strcpy(currCustomerDetailsFilePath, currCustomerDirectoryPath);
	strcpy(currCustomerRequestedLoansFilePath, currCustomerDirectoryPath);

	strcat(currCustomerDetailsFilePath, "/details");
	strcat(currCustomerRequestedLoansFilePath, "/loans");

	int fd1 = open(currCustomerDetailsFilePath, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	int fd2 = creat(currCustomerRequestedLoansFilePath, S_IRUSR | S_IWUSR);

	AcquireWriteLock(fd1);

	write(fd1, &customer, sizeof(CustomerInformation));

	UnLockFile(fd1);

	close(fd2);
	close(fd1);

	send(socketfd, customer.userid, strlen(customer.userid), 0);
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
			break;
		
		case 2:
			break;
		
		default: 
			GetWelcomeMenuResponse(socketfd);
			break;
	}
}

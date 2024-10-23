#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 65000
#define BUFFER_SIZE 14

char buffer[BUFFER_SIZE];

void SendWelcomeMenuResponse(int socketfd);
LoginResult SendCredentialsToServer(int socketfd, Credentials * creds);
void SendAdminMenuResponse(int socketfd);
void AddNewEmployee(int socketfd);
void SendManagerMenuResponse(int socketfd);
void SendEmployeeMenuResponse(int socketfd);
void AddNewCustomer(int socketfd);
void SendCustomerMenuResponse(int socketfd);
void GetAccountBalance(int socketfd);
void DepositMoney(int socketfd);
void WithdrawMoney(int socketfd);
void ViewTransactionHistory(int socketfd);
void TransferFunds(int socketfd);
void ChangePassword(int socketfd, ClientType clientType);
void ChangeCustomerDetails(int socketfd);
void ChangeEmployeeDetails(int socketfd);

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
	const char *ip_address = "127.0.0.1";

	sock = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	inet_pton(AF_INET, ip_address, &serv_addr.sin_addr);

    int success = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

	if(success == -1) {
		printf("Issues while connecting to the server.\n");
		close(sock);
		return 0;
	}
    
	SendWelcomeMenuResponse(sock);

    close(sock);
    return 0;
}

void SendWelcomeMenuResponse(int socketfd) {
	printf("%s", clientMenu);

	int val, response;
	scanf("%d", &val);
	getchar();

	response = htonl(val);

	send(socketfd, &response, sizeof(response), 0);

	if(val == 5)
		return;

	Credentials creds;
	LoginResult result = SendCredentialsToServer(socketfd, &creds);

	if(result == LOGIN_ID_NOT_FOUND) {
		printf("\nNo Such Login ID found. Please try again.\n");
		SendWelcomeMenuResponse(socketfd);
		return;
	}
	else if(result == PASSWORD_MISMATCH) {
		printf("\nIncorrect Password. Please try again.\n");
		SendWelcomeMenuResponse(socketfd);
		return;
	}
	else if(result == ROLE_MISMATCH) {
		printf("\nRole Mismatch. Please try again.\n");
		SendWelcomeMenuResponse(socketfd);
		return;
	}
	else {
		strcpy(buffer, creds.loginId);
		printf("\nLogged in Successfully.\n");
	}

	switch (val)
	{
		case 1:
			SendCustomerMenuResponse(socketfd);
			break;
		
		case 2: 
			SendEmployeeMenuResponse(socketfd);
			break;
		
		case 3: 
			SendManagerMenuResponse(socketfd);
			break;
		
		case 4: 
			SendAdminMenuResponse(socketfd);
			break;

		default:
			SendWelcomeMenuResponse(socketfd);
			break;
	}
}

LoginResult SendCredentialsToServer(int socketfd, Credentials * creds) {
	printf("\nEnter login Id : ");
	fgets(creds->loginId, sizeof(creds->loginId), stdin);
	creds->loginId[strcspn(creds->loginId, "\n")] = '\0';

	printf("\nEnter Password : ");
	fgets(creds->password, sizeof(creds->password), stdin);
	creds->password[strcspn(creds->password, "\n")] = '\0';

	send(socketfd, creds, sizeof(Credentials), 0);

	LoginResult result;

	read(socketfd, &result, sizeof(result));

	return result;
}

void SendAdminMenuResponse(int socketfd) {
	printf("%s", adminMenu);

	int val, response;

	scanf("%d", &val);
	getchar();

	response = htonl(val);

	send(socketfd, &response, sizeof(response), 0);

	switch (val)
	{
	case 1: 
		AddNewEmployee(socketfd);
		SendAdminMenuResponse(socketfd);
		break;
	
	case 2:
		ChangeCustomerDetails(socketfd);
		SendAdminMenuResponse(socketfd);
		break;
	
	case 3:
		ChangeEmployeeDetails(socketfd);
		SendAdminMenuResponse(socketfd);
		break;
	
	case 4:
		ChangePassword(socketfd, ADMIN);
		SendAdminMenuResponse(socketfd);
		break;

	default:
		SendWelcomeMenuResponse(socketfd);
		break;
	}
}

void AddNewEmployee(int socketfd) {
	EmployeeInformation employee;

	employee.status = ACTIVE;
	
	printf("\nEnter Full Name : ");
	fgets(employee.personalinformation.fullname, 25, stdin);
	employee.personalinformation.fullname[strcspn(employee.personalinformation.fullname, "\n")] = '\0';

	printf("\nEnter contact : ");
	fgets(employee.personalinformation.contact, 15, stdin);
	employee.personalinformation.contact[strcspn(employee.personalinformation.contact, "\n")] = '\0';

	printf("\nEnter Email : ");
	fgets(employee.personalinformation.email, 50, stdin);
	employee.personalinformation.email[strcspn(employee.personalinformation.email, "\n")] = '\0';

	printf("\nEnter Password : ");
	fgets(employee.password, 14, stdin);
	employee.password[strcspn(employee.password, "\n")] = '\0';

	char employeetype;

	printf("\nEnter (y) if the employee is manager, else type (n): ");
	scanf("%c", &employeetype);

	if(employeetype == 'y') {
		employee.employeetype = MANAGER;
	}
	else {
		employee.employeetype = EMPLOYEE;
	}

	send(socketfd, &employee, sizeof(EmployeeInformation), 0);

	read(socketfd, employee.userid, 14);

	printf("\nEmployee Id of the new Employee is : %s\n", employee.userid);
}

void SendManagerMenuResponse(int socketfd) {
	printf("%s", managerMenu);

	int val, response;

	scanf("%d", &val);
	getchar();

	response = htonl(val);

	send(socketfd, &response, sizeof(response), 0);

	switch (val)
	{
		case 1: 
			break;

		case 5:
			ChangePassword(socketfd, MANAGER);
			SendManagerMenuResponse(socketfd);
			break;
		
		default: 
			SendWelcomeMenuResponse(socketfd);
			break;
	}
}

void SendEmployeeMenuResponse(int socketfd) {
	printf("%s", employeeMenu);

	int val, response;

	scanf("%d", &val);
	getchar();

	response = htonl(val);

	send(socketfd, &response, sizeof(response), 0);

	switch (val)
	{
		case 1:
			AddNewCustomer(socketfd);
			SendEmployeeMenuResponse(socketfd);
			break;
		
		case 5:
			ChangePassword(socketfd, EMPLOYEE);
			SendEmployeeMenuResponse(socketfd);
			break;
		
		default:
			SendWelcomeMenuResponse(socketfd);
			break;
	}
}

void AddNewCustomer(int socketfd) {
	CustomerInformation customer;

	customer.status = ACTIVE;
	customer.balance = 0.0;
	
	printf("\nEnter Full Name : ");
	fgets(customer.personalinformation.fullname, 25, stdin);
	customer.personalinformation.fullname[strcspn(customer.personalinformation.fullname, "\n")] = '\0';

	printf("\nEnter contact : ");
	fgets(customer.personalinformation.contact, 15, stdin);
	customer.personalinformation.contact[strcspn(customer.personalinformation.contact, "\n")] = '\0';

	printf("\nEnter Email : ");
	fgets(customer.personalinformation.email, 50, stdin);
	customer.personalinformation.email[strcspn(customer.personalinformation.email, "\n")] = '\0';

	printf("\nEnter Password : ");
	fgets(customer.password, 14, stdin);
	customer.password[strcspn(customer.password, "\n")] = '\0';

	send(socketfd, &customer, sizeof(CustomerInformation), 0);

	read(socketfd, customer.userid, 14);

	printf("\nEmployee Id of the new Employee is : %s\n", customer.userid);
}

void SendCustomerMenuResponse(int socketfd) {
	printf("%s", customerMenu);

	int val, response;

	scanf("%d", &val);
	getchar();

	response = htonl(val);

	send(socketfd, &response, sizeof(response), 0);

	switch (val)
	{
		case 1:
			GetAccountBalance(socketfd);
			SendCustomerMenuResponse(socketfd);
			break;
		
		case 2:
			DepositMoney(socketfd);
			SendCustomerMenuResponse(socketfd);
			break;
		
		case 3:
			WithdrawMoney(socketfd);
			SendCustomerMenuResponse(socketfd);
			break;
		
		case 4:
			TransferFunds(socketfd);
			SendCustomerMenuResponse(socketfd);
			break;

		case 5:
			ChangePassword(socketfd, CUSTOMER);
			SendCustomerMenuResponse(socketfd);
			break;
		
		case 7:
			ViewTransactionHistory(socketfd);
			SendCustomerMenuResponse(socketfd);
			break;
		
		default:
			SendWelcomeMenuResponse(socketfd);
			break;
	}
}

void GetAccountBalance(int socketfd) {
	send(socketfd, buffer, BUFFER_SIZE, 0);

	double balance;

	read(socketfd, &balance, sizeof(double));

	printf("\nYour account Balance is : %lf\n", balance);
}

void DepositMoney(int socketfd) {
	double depositAmount;
	double balance;

	printf("\nEnter the amount to be deposited : ");
	scanf("%lf", &depositAmount);
	getchar();

	send(socketfd, buffer, BUFFER_SIZE, 0);
	send(socketfd, &depositAmount, sizeof(depositAmount), 0);

	read(socketfd, &balance, sizeof(balance));

	printf("\nYour Account's Current Balance is : %lf\n", balance);
}

void WithdrawMoney(int socketfd) {
	double withdrawAmount;
	double balance;

	printf("\nEnter the amount to be withdrawn : ");
	scanf("%lf", &withdrawAmount);
	getchar();

	send(socketfd, buffer, BUFFER_SIZE, 0);

	send(socketfd, &withdrawAmount, sizeof(withdrawAmount), 0);

	read(socketfd, &balance, sizeof(double));

	if(balance < withdrawAmount) {
		printf("\nIn-Sufficient Balance. Transaction Incomplete\n");
		return;
	}

	read(socketfd, &balance, sizeof(double));

	printf("\nYour Account's Current Balance is : %lf\n", balance);
}

void ViewTransactionHistory(int socketfd) {
	int totalTransactions;
	
	send(socketfd, buffer, BUFFER_SIZE, 0);

	read(socketfd, &totalTransactions, sizeof(totalTransactions));

	printf("\n######################################\n");
	printf("\nTotal Transactions : %d\n", totalTransactions);

	for(int i = 0; i < totalTransactions; i++) {
		Transaction transaction;
		read(socketfd, &transaction, sizeof(Transaction));
		printf("\n*********************************");
		printf("\nTransaction Amount : %lf", transaction.transferamount);
		printf("\nTo/From : %s", transaction.secondparty);
		printf("\nTransaction Time : %s", transaction.time);
		if(transaction.type == CREDIT)
			printf("Transaction Type : CREDIT\n");
		else
			printf("Transaction Type : DEBIT\n");
		
		printf("*********************************");
	}

	printf("\n######################################");
}

void TransferFunds(int socketfd) {
	double transferAmount;
	double balance;
	char payeeId[14];
	EntityExistenceResult result;

	printf("\nEnter Payee Id : ");
	fgets(payeeId, BUFFER_SIZE, stdin);
	payeeId[strcspn(payeeId, "\n")] = '\0';

	printf("\nEnter the amount to be transferred : ");
	scanf("%lf", &transferAmount);
	getchar();

	send(socketfd, buffer, BUFFER_SIZE, 0);

	send(socketfd, payeeId, BUFFER_SIZE, 0);

	read(socketfd, &result, sizeof(result));

	if(result != EXISTS) {
		printf("\nNo such Payee Exists. Try Again\n");
		return;
	}

	send(socketfd, &transferAmount, sizeof(transferAmount), 0);

	read(socketfd, &balance, sizeof(double));

	if(balance < transferAmount) {
		printf("\nIn-Sufficient Balance. Transaction Incomplete\n");
		return;
	}

	printf("\nFunds transferred successfully.\n");
}

void ChangePassword(int socketfd, ClientType clientType) {
	char newPassword[14];

	send(socketfd, &clientType, sizeof(ClientType), 0);

	send(socketfd, buffer, 14, 0);

	printf("\nEnter new Password : ");
	fgets(newPassword, 14, stdin);
	newPassword[strcspn(newPassword, "\n")] = '\0';

	send(socketfd, newPassword, 14, 0);

	printf("\nPassword updated Successfully.");
}

void ChangeCustomerDetails(int socketfd) {
	CustomerInformation customer;
	char customerId[14];
	EntityExistenceResult result;

	printf("\nEnter Customer Id : ");
	fgets(customerId, 14, stdin);
	customerId[strcspn(customerId, "\n")] = '\0';

	send(socketfd, customerId, BUFFER_SIZE, 0);

	read(socketfd, &result, sizeof(result));

	if(result != EXISTS) {
		printf("\nNo such Customer Exists. Try Again\n");
		return;
	}

	printf("\nEnter Customer Name : ");
	fgets(customer.personalinformation.fullname, 25, stdin);

	printf("\nEnter Customer Email : ");
	fgets(customer.personalinformation.email, 50, stdin);

	printf("\nEnter Customer Contact : ");
	fgets(customer.personalinformation.contact, 15, stdin);

	printf("\nEnter Customer Password : ");
	fgets(customer.password, 14, stdin);
	customer.password[strcspn(customer.password, "\n")] = '\0';

	send(socketfd, &customer, sizeof(CustomerInformation), 0);

	printf("\nCustomer Details Updated Successfully");
}

void ChangeEmployeeDetails(int socketfd) {
	EmployeeInformation employee;
	char employeeId[14];
	EntityExistenceResult result;

	printf("\nEnter Employee Id : ");
	fgets(employeeId, 14, stdin);
	employeeId[strcspn(employeeId, "\n")] = '\0';

	send(socketfd, employeeId, BUFFER_SIZE, 0);

	read(socketfd, &result, sizeof(result));

	if(result != EXISTS) {
		printf("\nNo such Employee Exists. Try Again\n");
		return;
	}

	printf("\nEnter Employee Name : ");
	fgets(employee.personalinformation.fullname, 25, stdin);

	printf("\nEnter Employee Email : ");
	fgets(employee.personalinformation.email, 50, stdin);

	printf("\nEnter Employee Contact : ");
	fgets(employee.personalinformation.contact, 15, stdin);

	printf("\nEnter Employee Password : ");
	fgets(employee.password, 14, stdin);
	employee.password[strcspn(employee.password, "\n")] = '\0';

	char type;
	printf("\nEnter (y) if the employee is manager, else type (n): ");
	scanf("%c", &type);
	getchar();

	if(type == 'y')
		employee.employeetype = MANAGER;
	else
		employee.employeetype = EMPLOYEE;

	send(socketfd, &employee, sizeof(EmployeeInformation), 0);

	printf("\nEmployee Details Updated Successfully");
}

#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 65000
#define BUFFER_SIZE 1024

char buffer[BUFFER_SIZE];

void SendWelcomeMenuResponse(int socketfd);
LoginResult SendCredentialsToServer(int socketfd);
void SendAdminMenuResponse(int socketfd);
void AddNewEmployee(int socketfd);
void SendManagerMenuResponse(int socketfd);
void SendEmployeeMenuResponse(int socketfd);
void AddNewCustomer(int socketfd);

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

	LoginResult result = SendCredentialsToServer(socketfd);

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
		printf("\nLogged in Successfully.\n");
	}

	switch (val)
	{
		case 1:
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

LoginResult SendCredentialsToServer(int socketfd) {
	Credentials creds;

	printf("\nEnter login Id : ");
	fgets(creds.loginId, sizeof(creds.loginId), stdin);
	creds.loginId[strcspn(creds.loginId, "\n")] = '\0';

	printf("\nEnter Password : ");
	fgets(creds.password, sizeof(creds.password), stdin);
	creds.password[strcspn(creds.password, "\n")] = '\0';

	send(socketfd, &creds, sizeof(Credentials), 0);

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
	
	default:
		SendWelcomeMenuResponse(socketfd);
		break;
	}
}

void AddNewEmployee(int socketfd) {
	EmployeeInformation employee;

	employee.status = ACTIVE;
	
	printf("\nEnter Full Name : ");
	strcpy(employee.personalinformation.fullname, "Full Name\n");
	printf("%s", employee.personalinformation.fullname);
	//fgets(employee.personalinformation.fullname, sizeof(employee.personalinformation.fullname), stdin);

	printf("\nEnter contact : ");
	strcpy(employee.personalinformation.contact, "9999999999\n");
	printf("%s", employee.personalinformation.contact);
	//fgets(employee.personalinformation.contact, sizeof(employee.personalinformation.contact), stdin);

	printf("\nEnter Email : ");
	strcpy(employee.personalinformation.email, "some-email@gmail.com\n");
	printf("%s", employee.personalinformation.email);
	//fgets(employee.personalinformation.email, sizeof(employee.personalinformation.email), stdin);

	printf("\nEnter Password : ");
	strcpy(employee.password, "password");
	printf("%s", employee.password);
	//fgets(employee.password, sizeof(employee.password), stdin);

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

	int bytesread = read(socketfd, employee.userid, 14);
	employee.userid[bytesread] = '\0';

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
		
		default:
			SendWelcomeMenuResponse(socketfd);
			break;
	}
}

void AddNewCustomer(int socketfd) {
	CustomerInformation customer;

	customer.status = ACTIVE;
	
	printf("\nEnter Full Name : ");
	strcpy(customer.personalinformation.fullname, "Full Name\n");
	printf("%s", customer.personalinformation.fullname);
	//fgets(employee.personalinformation.fullname, sizeof(employee.personalinformation.fullname), stdin);

	printf("\nEnter contact : ");
	strcpy(customer.personalinformation.contact, "9999999999\n");
	printf("%s", customer.personalinformation.contact);
	//fgets(employee.personalinformation.contact, sizeof(employee.personalinformation.contact), stdin);

	printf("\nEnter Email : ");
	strcpy(customer.personalinformation.email, "some-email@gmail.com\n");
	printf("%s", customer.personalinformation.email);
	//fgets(employee.personalinformation.email, sizeof(employee.personalinformation.email), stdin);

	printf("\nEnter Password : ");
	strcpy(customer.password, "password");
	printf("%s", customer.password);
	//fgets(employee.password, sizeof(employee.password), stdin);

	send(socketfd, &customer, sizeof(CustomerInformation), 0);

	int bytesRead = read(socketfd, customer.userid, 13);
	customer.userid[bytesRead] = '\0';

	printf("\nEmployee Id of the new Employee is : %s\n", customer.userid);
}
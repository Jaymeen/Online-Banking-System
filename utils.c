#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

const char *serveripAddress = "127.0.0.1";

const char *clientMenu = "\nEnter log-in type: \n 1. Customer \n 2. Bank Employee \n 3. Manager \n 4. Administrator \n 5. Exit \n Enter your choice : ";
const char *adminMenu = "\nPlease select a option - \n 1. Add New Bank Employee.\n 2. Modify Customer Details.\n 3. Modify Employee Details.\n 4. Change Password.\n 5. Exit.\n Enter your choice : ";
const char *managerMenu = "\nPlease select a option - \n 1. Activate Bank Account.\n 2. Deactivate Bank Account.\n 3. Assign Loan Application to a Employee.\n 4. Review Customer Feedback.\n 5. Change Password.\n 6. Logout.\n 7. Exit.\n Enter your choice : ";
const char *employeeMenu = "\nPlease select a option - \n 1. Add a new Customer.\n 2. Process Loan Application.\n 3. Approve/Reject Particular Loan Applications.\n 4. View Customer Transactions.\n 5. Change Password.\n 6. Logout.\n 7. Exit.\n Enter your choice : ";
const char *customerMenu = "\nPlease select a option - \n 1. View Account Balance.\n 2. Deposit Money.\n 3. Withdraw Money.\n 4. Transfer Funds.\n 5. Change Password.\n 6. Add Feedback\n 7. View Transactions History.\n 8. Logout.\n Enter your choice : ";

char adminCredentialsFilePath[212];
char customerIndexesFilePath[216];
char employeeIndexesFilePath[216];
char allLoansPath[208];

char dataDirectoryPath[200];
char employeesDirectoryPath[210]; 

void init() {
	getcwd(dataDirectoryPath, 200);
	strcat(dataDirectoryPath, "/data");

	mkdir(dataDirectoryPath, 0755);

	strcpy(adminCredentialsFilePath, dataDirectoryPath);
	strcat(adminCredentialsFilePath, "/credentials");

	strcpy(employeesDirectoryPath, dataDirectoryPath);
	strcat(employeesDirectoryPath, "/employees");

	mkdir(employeesDirectoryPath, 0755);

	strcpy(customerIndexesFilePath, dataDirectoryPath);
	strcat(customerIndexesFilePath, "/customerindexes");

	strcpy(employeeIndexesFilePath, dataDirectoryPath);
	strcat(employeeIndexesFilePath, "/employeeindexes");

	strcpy(allLoansPath, dataDirectoryPath);
	strcat(allLoansPath, "/allloans");
}

void AcquireWriteLock(int fd) {
    struct flock flock;
    memset(&flock, 0, sizeof(flock));
    flock.l_type = F_WRLCK;
    flock.l_whence = SEEK_SET;
    flock.l_start = 0;
    flock.l_len = 0;

    if (fcntl(fd, F_SETLKW, &flock) == -1) {
        printf("\nError Acquiring WRITE File Lock.\n");
		printf("Failed to acquire lock: %s\n", strerror(errno));
    }
}

void AcquireReadLock(int fd) {
	struct flock flock;
    memset(&flock, 0, sizeof(flock));
    flock.l_type = F_RDLCK;
    flock.l_whence = SEEK_SET;
    flock.l_start = 0;
    flock.l_len = 0;

    if (fcntl(fd, F_SETLKW, &flock) == -1) {
        printf("\nError Acquiring WRITE File Lock.\n");
		printf("Failed to acquire lock: %s\n", strerror(errno));
    }
}

void UnLockFile(int fd) {
    struct flock flock;
    memset(&flock, 0, sizeof(flock));
    flock.l_type = F_UNLCK;
    flock.l_whence = SEEK_SET;
    flock.l_start = 0;
    flock.l_len = 0;

    if (fcntl(fd, F_SETLK, &flock) == -1) {
        printf("\nError Releasing File Lock.\n");
		printf("Failed to acquire lock: %s\n", strerror(errno));
    }
}

/* 
	-------------------------------------------------------------------------------------------------------------- 
									Debugging / Single Time Use / Rare Use
	--------------------------------------------------------------------------------------------------------------
*/

void PrintEmployeeInformation(EmployeeInformation * employee) {
	printf("\nEmployee Name : %s", employee->personalinformation.fullname);
	printf("\nEmployee Contact : %s", employee->personalinformation.contact);
	printf("\nEmployee Email : %s", employee->personalinformation.email);
	printf("\nEmployee Password : %s", employee->password);
	printf("\nEmployee Type : %d", employee->employeetype);
	printf("\nEmployee Status : %d\n", employee->status);
}

void ResetIndexValues() {
	int value;

	value = 0;

	int fd1 = open("/home/jarvis/Desktop/Banking-System/data/customerindexes", O_TRUNC | O_WRONLY | O_CREAT);
	int fd2 = open("/home/jarvis/Desktop/Banking-System/data/employeeindexes", O_TRUNC | O_WRONLY | O_CREAT);

	write(fd1, &value, sizeof(value));
	write(fd2, &value, sizeof(value));

	close(fd1);
	close(fd2);
}
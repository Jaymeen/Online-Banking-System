#ifndef GLOBALS_H
#define GLOBALS_H

extern const char *serveripAddress;

extern const char *clientMenu;
extern const char *adminMenu;
extern const char *managerMenu;
extern const char *employeeMenu;
extern const char *customerMenu;

extern char adminCredentialsFilePath[212];
extern char customerIndexesFilePath[216];
extern char employeeIndexesFilePath[216];
extern char allLoansPath[208];

extern char dataDirectoryPath[200];
extern char employeesDirectoryPath[210];
extern char customersDirectoryPath[210];

typedef enum {
	ACTIVE,
	INACTIVE
} Status;

typedef enum {
	CUSTOMER,
	MANAGER,
	EMPLOYEE,
	ADMIN
} ClientType;

typedef enum {
	LOGIN_SUCCESSFUL,
	LOGIN_ID_NOT_FOUND,
	PASSWORD_MISMATCH,
	ROLE_MISMATCH
} LoginResult;

typedef enum {
	EXISTS,
	DOES_NOT_EXIST
} EnityExistenceResult;

typedef enum {
	CREDIT,
	DEBIT
} TransactionType;

typedef struct {
	char loginId[14];
    char password[14];
} Credentials;

typedef struct {
	char fullname[25];
	char email[50];
	char contact[11];
} PersonalInformation;

typedef struct {
	PersonalInformation personalinformation;
	char userid[14];
	char password[20];
	ClientType employeetype;
	Status status;
} EmployeeInformation;

typedef struct {
	PersonalInformation personalinformation;
	double balance;
	char userid[14];
	char password[20];
	Status status;
} CustomerInformation;

typedef struct {
	double transferamount;
	char secondparty[14];
	char time[30];
	TransactionType type;
} Transaction;

void init();
void AcquireWriteLock(int fd);
void AcquireReadLock(int fd);
void UnLockFile(int fd);

void PrintEmployeeInformation(EmployeeInformation * employee);

#endif 
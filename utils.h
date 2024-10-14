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

typedef enum {
	ACTIVE,
	INACTIVE
} Status;

typedef enum {
	MANAGER,
	EMPLOYEE
} EmployeeType;

typedef enum {
	LOGIN_SUCCESSFUL,
	LOGIN_ID_NOT_FOUND,
	PASSWORD_MISMATCH,
	ROLE_MISMATCH
} LoginResult;

typedef struct {
	char loginId[25];
    char password[25];
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
	EmployeeType employeetype;
	Status status;
} EmployeeInformation;

void init();
void AcquireWriteLock(int fd);
void AcquireReadLock(int fd);
void UnLockFile(int fd);

void PrintEmployeeInformation(EmployeeInformation * employee);

#endif 
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

void ResetIndexValues(char * directoryPath);
void CreateAllLoansFile(char * directoryPath);
void CreateAdminCredentialsFile(char * directoryPath);
void CreateFeedbacksFile(char * directoryPath);

void main() {
	char dataDirectoryPath[200];
	getcwd(dataDirectoryPath, 200);
	strcat(dataDirectoryPath, "/data");

	ResetIndexValues(dataDirectoryPath);
	CreateAllLoansFile(dataDirectoryPath);
	CreateAdminCredentialsFile(dataDirectoryPath);
	CreateFeedbacksFile(dataDirectoryPath);
}

void ResetIndexValues(char * directoryPath) {
	char customerIndexesFilePath[216];
	char employeeIndexesFilePath[216];

	strcpy(customerIndexesFilePath, directoryPath);
	strcat(customerIndexesFilePath, "/customerindexes");
	strcpy(employeeIndexesFilePath, directoryPath);
	strcat(employeeIndexesFilePath, "/employeeindexes");

	int value;

	value = 1;

	int fd1 = open(customerIndexesFilePath, O_TRUNC | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	int fd2 = open(employeeIndexesFilePath, O_TRUNC | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

	write(fd1, &value, sizeof(value));
	write(fd2, &value, sizeof(value));

	close(fd1);
	close(fd2);
}

void CreateAllLoansFile(char * directoryPath) {
	char allLoansFilePath[216];
	strcpy(allLoansFilePath, directoryPath);
	strcat(allLoansFilePath, "/allloans");

	int fd = creat(allLoansFilePath, S_IRUSR | S_IWUSR);
	close(fd);
}

void CreateAdminCredentialsFile(char * directoryPath) {
	char adminCredentialsFilePath[212];
	strcpy(adminCredentialsFilePath, directoryPath);
	strcat(adminCredentialsFilePath, "/credentials");

	Credentials creds;

	strcpy(creds.loginId, "admin");
	strcpy(creds.password, "123");

	int fd = open(adminCredentialsFilePath, O_TRUNC | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

	write(fd, &creds, sizeof(Credentials));

	close(fd);
}

void CreateFeedbacksFile(char * directoryPath) {
	char feedbacksFilePath[212];
	strcpy(feedbacksFilePath, directoryPath);
	strcat(feedbacksFilePath, "/feedbacks");

	int fd = creat(feedbacksFilePath, S_IRUSR | S_IWUSR);
	close(fd);
}
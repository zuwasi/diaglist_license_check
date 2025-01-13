#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>
#include <windows.h>

#define MAX_PATH_LEN 260
#define LINE_LEN 512
#define CONFIG_FILE "stored_path.txt"
#define RESULTS_FILE "results.txt"

// Function prototypes
int listLicenseFiles(const char* directory, char files[][MAX_PATH_LEN], int maxFiles);
void chooseLicenseFile(char* filePath, const char* directory);
bool isDirectory(const char* path);
void checkDiaglistSection(const char* filePath);
int monthToNumber(const char* month);
int daysUntil(int day, int month, int year);
void writeResultsToFile(const char* results);
void manageStoredPath(char* filePath);

// Function to check if a path is a directory
bool isDirectory(const char* path) {
    DWORD attributes = GetFileAttributesA(path);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return false; // Path does not exist
    }
    return (attributes & FILE_ATTRIBUTE_DIRECTORY);
}

// Function to map month abbreviation to numerical value
int monthToNumber(const char* month) {
    static const char* months[] = {
        "jan", "feb", "mar", "apr", "may", "jun",
        "jul", "aug", "sep", "oct", "nov", "dec"
    };

    char monthLower[4];
    for (int i = 0; i < 3; i++) {
        monthLower[i] = tolower(month[i]);
    }
    monthLower[3] = '\0'; // Null-terminate the string

    for (int i = 0; i < 12; i++) {
        if (strcmp(monthLower, months[i]) == 0) {
            return i; // Return the zero-based index for the month
        }
    }

    return -1; // Invalid month
}

// Function to calculate the number of days until a future date
int daysUntil(int day, int month, int year) {
    struct tm future = { 0 };

    future.tm_mday = day;
    future.tm_mon = month;
    future.tm_year = year - 1900;
    future.tm_isdst = -1; // Ignore daylight saving time

    time_t now = time(NULL);
    time_t futureTime = mktime(&future);

    if (futureTime == -1) {
        fprintf(stderr, "Error: Invalid future date\n");
        return -1;
    }

    return (int)((futureTime - now) / (60 * 60 * 24));
}

// Function to write results to a file
void writeResultsToFile(const char* results) {
    FILE* resultsFile = fopen(RESULTS_FILE, "a"); // Append to results file
    if (!resultsFile) {
        fprintf(stderr, "Error: Unable to create results file.\n");
        return;
    }

    fprintf(resultsFile, "%s", results);
    fclose(resultsFile);
    printf("Results saved to '%s'.\n", RESULTS_FILE);
}

// Function to check for "# Diaglist" section and process dates
void checkDiaglistSection(const char* filePath) {
    FILE* file = fopen(filePath, "r");
    if (!file) {
        fprintf(stderr, "Error: Unable to open file at %s\n", filePath);
        return;
    }

    char line[LINE_LEN];
    bool diaglistFound = false;
    char resultsBuffer[8192] = ""; // Buffer to store results for writing to file
    char licenseType[LINE_LEN] = ""; // To store LICENSE information
    char customerName[LINE_LEN] = ""; // To store customer name

    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "# Diaglist") != NULL) {
            diaglistFound = true;
            break;
        }

        // Extract LICENSE type
        if (strstr(line, "LICENSE") != NULL) {
            sscanf(line, "LICENSE %511[^\n]", licenseType);
        }

        // Extract customer name
        if (strstr(line, "customer=") != NULL) {
            char* start = strstr(line, "customer=\"");
            if (start) {
                sscanf(start, "customer=\"%511[^\"]\"", customerName);
            }
        }
    }

    if (!diaglistFound) {
        snprintf(resultsBuffer, sizeof(resultsBuffer), "Diaglist feature doesn't exist.\n");
        printf("%s", resultsBuffer);
        writeResultsToFile(resultsBuffer);
        fclose(file);
        return;
    }

    snprintf(resultsBuffer, sizeof(resultsBuffer), "Diaglist section found. Checking for dates...\n");
    printf("%s", resultsBuffer);
    writeResultsToFile(resultsBuffer);

    // Add LICENSE and customer information to results
    snprintf(resultsBuffer, sizeof(resultsBuffer), "License Type: %s\nCustomer Name: %s\n", licenseType, customerName);
    printf("%s", resultsBuffer);
    writeResultsToFile(resultsBuffer);

    // Process lines for dates in the "# Diaglist" section
    while (fgets(line, sizeof(line), file)) {
        int day, month, year;
        char* datePos = strstr(line, " "); // Start looking for dates after spaces

        while (datePos) {
            char resultsLine[1024] = "";

            if (sscanf(datePos, "%2d-%*3s-%4d", &day, &year) == 2) {
                char monthStr[4];
                sscanf(datePos, "%*2d-%3s-%*4d", monthStr);
                month = monthToNumber(monthStr);

                if (month != -1) {
                    int daysLeft = daysUntil(day, month, year);
                    snprintf(resultsLine, sizeof(resultsLine),
                        "Expiration date: %02d-%s-%04d, Days left: %d\n",
                        day, monthStr, year, daysLeft);
                }
                else {
                    snprintf(resultsLine, sizeof(resultsLine), "Invalid month in date: %s\n", datePos);
                }

                // Print and save results
                printf("%s", resultsLine);
                writeResultsToFile(resultsLine);
            }
            datePos = strstr(datePos + 1, " "); // Move to the next space
        }
    }

    fclose(file);
}

// Function to list all `.lic` files in a directory
int listLicenseFiles(const char* directory, char files[][MAX_PATH_LEN], int maxFiles) {
    WIN32_FIND_DATAA findFileData;
    HANDLE hFind;
    char searchPath[MAX_PATH_LEN];

    snprintf(searchPath, MAX_PATH_LEN, "%s\\*.lic", directory);
    hFind = FindFirstFileA(searchPath, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "No .lic files found in directory: %s\n", directory);
        return 0;
    }

    int count = 0;
    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            snprintf(files[count], MAX_PATH_LEN, "%s\\%s", directory, findFileData.cFileName);
            count++;
        }
    } while (FindNextFileA(hFind, &findFileData) != 0 && count < maxFiles);

    FindClose(hFind);
    return count;
}

// Function to allow the user to choose a license file
void chooseLicenseFile(char* filePath, const char* directory) {
    char files[100][MAX_PATH_LEN];
    int fileCount = listLicenseFiles(directory, files, 100);

    if (fileCount == 0) {
        fprintf(stderr, "No license files found in the specified directory.\n");
        exit(1);
    }

    printf("Found %d license file(s):\n", fileCount);
    for (int i = 0; i < fileCount; i++) {
        printf("%d: %s\n", i + 1, files[i]);
    }

    printf("Select a file by number: ");
    int choice;
    if (scanf("%d", &choice) != 1 || choice < 1 || choice > fileCount) {
        fprintf(stderr, "Error: Invalid selection.\n");
        exit(1);
    }

    strncpy(filePath, files[choice - 1], MAX_PATH_LEN);
    printf("You selected: %s\n", filePath);

    FILE* config = fopen(CONFIG_FILE, "w");
    if (!config) {
        fprintf(stderr, "Error: Unable to create configuration file.\n");
        exit(1);
    }
    fprintf(config, "%s\n", filePath);
    fclose(config);
}

// Function to manage stored path logic
void manageStoredPath(char* filePath) {
    FILE* config = fopen(CONFIG_FILE, "r");
    if (config) {
        fgets(filePath, MAX_PATH_LEN, config);
        filePath[strcspn(filePath, "\r\n")] = '\0'; // Remove newline
        fclose(config);

        printf("Stored path found: %s\n", filePath);
        printf("Do you want to use the stored path? (y/n): ");
        char choice;
        scanf(" %c", &choice);

        if (choice == 'n' || choice == 'N') {
            remove(CONFIG_FILE);
            printf("Enter the directory containing license files: ");
            char directory[MAX_PATH_LEN];
            scanf(" %259s", directory);

            if (!isDirectory(directory)) {
                fprintf(stderr, "Error: %s is not a valid directory.\n", directory);
                exit(1);
            }

            chooseLicenseFile(filePath, directory);
        }
    }
    else {
        printf("Enter the directory containing license files: ");
        char directory[MAX_PATH_LEN];
        scanf(" %259s", directory);

        if (!isDirectory(directory)) {
            fprintf(stderr, "Error: %s is not a valid directory.\n", directory);
            exit(1);
        }

        chooseLicenseFile(filePath, directory);
    }
}

int main() {
    char filePath[MAX_PATH_LEN] = { 0 };

    manageStoredPath(filePath);
    printf("Checking license file: %s\n", filePath);

    checkDiaglistSection(filePath);

    return 0;
}

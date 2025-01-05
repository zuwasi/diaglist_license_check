// version with file path logic

#define _CRT_SECURE_NO_WARNINGS //added just for VS2022 build 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>
#include <windows.h> //added to solve possible race conftion at FILE* config = fopen(CONFIG_FILE, "w");

#define MAX_PATH_LEN 260
#define LINE_LEN 512
#define CONFIG_FILE "stored_path.txt"

// Map month abbreviation to numerical value
#if 0 // testing gsoultion for solving Do not subtract two pointers that do not address elements of the same array [BD-PB-PTRSUB] messege 
int monthToNumber(const char* month) {
    static const char* months = "janfebmaraprmayjunjulaugsepoctnovdec";
    char monthLower[4];

    // Convert the input month to lowercase
    for (int i = 0; i < 3; i++) {
        monthLower[i] = tolower(month[i]);
    }
    monthLower[3] = '\0'; // Null-terminate the string

    const char* monthPos = strstr(months, monthLower);

    if (!monthPos) {
        return -1; // Invalid month
    }

    return (monthPos - months) / 3;
}
#endif 

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



// Function to parse a date in "DD-MMM-YYYY" format
bool parseDate(const char* dateStr, int* day, int* month, int* year) {
    char monthStr[4];
    if (sscanf(dateStr, "%2d-%3s-%4d", day, monthStr, year) == 3) {
        *month = monthToNumber(monthStr);
        return *month != -1;
    }
    return false;
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

// Function to check for "# Diaglist" section and process dates
void checkDiaglistSection(const char* filePath) {
    FILE* file = fopen(filePath, "r");
    if (!file) {
        fprintf(stderr, "Error: Unable to open file at %s\n", filePath);
        return;
    }

    char line[LINE_LEN];
    bool diaglistFound = false;

    // Search for "# Diaglist" section
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "# Diaglist") != NULL) {
            diaglistFound = true;
            break;
        }
    }

    if (!diaglistFound) {
        printf("Diaglist feature doesn't exist.\n");
        fclose(file);
        return;
    }

    printf("Diaglist section found. Checking for dates...\n");

    // Process lines for dates in the "# Diaglist" section
    while (fgets(line, sizeof(line), file)) {
        int day, month, year;
        char* datePos = strstr(line, " "); // Start looking for dates after spaces

        while (datePos) {
            if (sscanf(datePos, "%2d-%*3s-%4d", &day, &year) == 2) {
                char monthStr[4];
                sscanf(datePos, "%*2d-%3s-%*4d", monthStr);
                month = monthToNumber(monthStr);

                if (month != -1) {
                    int daysLeft = daysUntil(day, month, year);
                    if (daysLeft >= 0) {
                        printf("Expiration date: %02d-%s-%04d, Days left: %d\n", day, monthStr, year, daysLeft);
                    }
                    else {
                        printf("Expiration date: %02d-%s-%04d is in the past.\n", day, monthStr, year);
                    }
                }
                else {
                    printf("Invalid month in date: %s\n", datePos);
                }
            }
            datePos = strstr(datePos + 1, " "); // Move to the next space
        }
    }

    fclose(file);
}

// Function to load the stored file path
bool loadStoredPath(char* filePath) {
    FILE* config = fopen(CONFIG_FILE, "r");
    if (!config) {
        return false;
    }
    if (fgets(filePath, MAX_PATH_LEN, config) != NULL) {
        // Remove newline character
        filePath[strcspn(filePath, "\n")] = '\0';
        fclose(config);
        return true;
    }
    fclose(config);
    return false;
}

// Function to store the file path
//possible race condtion at FILE* config = fopen(CONFIG_FILE, "w"); if stored_path.txt is opened more than once 
// windows soultion
#if 0
void storeFilePath(const char* filePath) {
    FILE* config = fopen(CONFIG_FILE, "w");
    if (config) {
        fprintf(config, "%s\n", filePath);
        fclose(config);
        printf("Stored file path successfully.\n");
    }
    else {
        fprintf(stderr, "Error: Unable to store file path.\n");
    }
}
#endif
//for Linux use that code 
#if 0
#include <fcntl.h>
#include <unistd.h>

void storeFilePath(const char* filePath) {
    int fd = open(CONFIG_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        fprintf(stderr, "Error: Unable to store file path.\n");
        return;
    }

    // Lock the file for exclusive access
    if (flock(fd, LOCK_EX) == -1) {
        fprintf(stderr, "Error: Unable to lock the file for writing.\n");
        close(fd);
        return;
    }

    dprintf(fd, "%s\n", filePath);

    // Unlock and close the file
    flock(fd, LOCK_UN);
    close(fd);

    printf("Stored file path successfully.\n");
}
#endif


void storeFilePath(const char* filePath) {
    HANDLE hFile = CreateFile(
        CONFIG_FILE,
        GENERIC_WRITE,
        0, // No sharing
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error: Unable to store file path.\n");
        return;
    }

    DWORD bytesWritten;
    if (!WriteFile(hFile, filePath, strlen(filePath), &bytesWritten, NULL)) {
        fprintf(stderr, "Error: Failed to write to the file.\n");
    }
    else {
        printf("Stored file path successfully.\n");
    }

    CloseHandle(hFile);
}


// Function to delete the stored file path
void deleteStoredFilePath() {
    if (remove(CONFIG_FILE) == 0) {
        printf("Stored file path deleted successfully.\n");
    }
    else {
        fprintf(stderr, "Error: No stored file path to delete.\n");
    }
}

#if 0//commented out to create a diffrent version to test fix  for parasoft rules Avoid overflow due to reading a not zero terminated string [BD-PB-OVERFNZT]
int main() {
    char filePath[MAX_PATH_LEN];

    if (loadStoredPath(filePath)) {
        printf("Stored file path found: %s\n", filePath);
        printf("Do you want to delete the stored file path? (y/n): ");
        char choice;
        scanf(" %c", &choice);

        if (choice == 'y' || choice == 'Y') {
            deleteStoredFilePath();
            printf("Enter the license file path: ");
            scanf(" %s", filePath);
            storeFilePath(filePath);
        }
    }
    else {
        printf("No stored file path found. Please enter the license file path: ");
        scanf(" %s", filePath);
        storeFilePath(filePath);
    }

    checkDiaglistSection(filePath);

    return 0;
}
#endif 
int main() {
    char filePath[MAX_PATH_LEN] = { 0 }; // Initialize with zeros

    if (loadStoredPath(filePath)) {
        printf("Stored file path found: %s\n", filePath);
        printf("Do you want to delete the stored file path? (y/n): ");
        char choice;
        scanf(" %c", &choice);

        if (choice == 'y' || choice == 'Y') {
            deleteStoredFilePath();
            printf("Enter the license file path: ");
            if (scanf(" %259s", filePath) != 1) { // Limit input length to prevent overflow
                fprintf(stderr, "Error: Invalid file path input.\n");
                return 1;
            }
            storeFilePath(filePath);
        }
    }
    else {
        printf("No stored file path found. Please enter the license file path: ");
        if (scanf(" %259s", filePath) != 1) { // Limit input length to prevent overflow
            fprintf(stderr, "Error: Invalid file path input.\n");
            return 1;
        }
        storeFilePath(filePath);
    }

    checkDiaglistSection(filePath);

    return 0;
}

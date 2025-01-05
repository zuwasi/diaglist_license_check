// version with file path logic

#define _CRT_SECURE_NO_WARNINGS //added just for VS2022 build 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>

#define MAX_PATH_LEN 260
#define LINE_LEN 512
#define CONFIG_FILE "stored_path.txt"

// Map month abbreviation to numerical value
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

// Function to delete the stored file path
void deleteStoredFilePath() {
    if (remove(CONFIG_FILE) == 0) {
        printf("Stored file path deleted successfully.\n");
    }
    else {
        fprintf(stderr, "Error: No stored file path to delete.\n");
    }
}

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
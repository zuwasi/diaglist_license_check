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

    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "# Diaglist") != NULL) {
            diaglistFound = true;
            break;
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
            printf("Enter the license file path: ");
            scanf(" %259s", filePath);

            FILE* newConfig = fopen(CONFIG_FILE, "w");
            if (!newConfig) {
                fprintf(stderr, "Error: Unable to create configuration file.\n");
                exit(1);
            }
            fprintf(newConfig, "%s\n", filePath);
            fclose(newConfig);
        }
    }
    else {
        printf("Enter the license file path: ");
        scanf(" %259s", filePath);

        FILE* configNew = fopen(CONFIG_FILE, "w");
        if (!configNew) {
            fprintf(stderr, "Error: Unable to create configuration file.\n");
            exit(1);
        }
        fprintf(configNew, "%s\n", filePath);
        fclose(configNew);
    }
}

int main() {
    char filePath[MAX_PATH_LEN] = { 0 };

    manageStoredPath(filePath);
    printf("Checking license file: %s\n", filePath);

    checkDiaglistSection(filePath); // Ensure file is processed after managing path

    return 0;
}

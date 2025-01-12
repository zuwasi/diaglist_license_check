#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>
#include <windows.h>
#include <locale.h>

#define MAX_PATH_LEN 260
#define CONFIG_FILE L"stored_path.txt" // Wide-character string

void validateFileName(const wchar_t* fileName) {
    for (size_t i = 0; i < wcslen(fileName); i++) {
        if (!iswprint(fileName[i])) {
            fwprintf(stderr, L"Error: Non-printable character in file name: %ls\n", fileName);
            exit(1);
        }
    }
}

void storeFilePath(const char* filePath) {
    printf("DEBUG: Attempting to create file with name: '%ls'\n", CONFIG_FILE);
    validateFileName(CONFIG_FILE);

    HANDLE hFile = CreateFileW(
        CONFIG_FILE,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error: Unable to store file path. Error code: %lu\n", GetLastError());
        return;
    }

    DWORD bytesWritten;
    if (!WriteFile(hFile, filePath, strlen(filePath), &bytesWritten, NULL)) {
        fprintf(stderr, "Error: Failed to write to the file. Error code: %lu\n", GetLastError());
    }
    else if (bytesWritten != strlen(filePath)) {
        fprintf(stderr, "Error: Partial write. Bytes written: %lu\n", bytesWritten);
    }
    else {
        printf("Stored file path successfully: %s\n", filePath);
    }

    CloseHandle(hFile);
}

int main() {
    setlocale(LC_ALL, "C"); // Force consistent locale
    printf("DEBUG: Current locale: %s\n", setlocale(LC_ALL, NULL));

    char filePath[MAX_PATH_LEN] = { 0 };
    printf("Enter the license file path: ");
    if (scanf(" %259s", filePath) != 1) {
        fprintf(stderr, "Error: Invalid input.\n");
        return 1;
    }

    storeFilePath(filePath);

    return 0;
}

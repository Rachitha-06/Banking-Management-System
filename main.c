#include "bank.h"

#include <stdio.h>
#include <string.h>

static void saveAfterChange(const Bank *bank) {
    if (saveBank(bank)) {
        printf("Data saved.\n");
    } else {
        printf("The change remains in memory, but it was not saved to disk.\n");
    }
}

int main(void) {
    Bank bank;
    char choice[20];
    int loadResult;

    initializeBank(&bank);
    loadResult = loadBank(&bank);
    if (loadResult == 1) {
        printf("Saved BankCore data loaded successfully.\n");
    } else if (loadResult == 0) {
        printf("No saved data found. Starting a new bank.\n");
    } else {
        printf("Warning: saved data could not be read. Starting with an empty bank.\n");
    }

    while (1) {
        displayMainMenu();
        if (!readLine(choice, sizeof(choice))) {
            printf("\nInput ended. Saving current data before exit.\n");
            saveBank(&bank);
            break;
        }

        if (strcmp(choice, "1") == 0) {
            if (createAccount(&bank)) {
                saveAfterChange(&bank);
            }
        } else if (strcmp(choice, "2") == 0) {
            viewAccount(&bank);
        } else if (strcmp(choice, "3") == 0) {
            searchAccounts(&bank);
        } else if (strcmp(choice, "4") == 0) {
            if (updateAccount(&bank)) {
                saveAfterChange(&bank);
            }
        } else if (strcmp(choice, "5") == 0) {
            if (closeAccount(&bank)) {
                saveAfterChange(&bank);
            }
        } else if (strcmp(choice, "6") == 0) {
            if (depositMoney(&bank)) {
                saveAfterChange(&bank);
            }
        } else if (strcmp(choice, "7") == 0) {
            if (withdrawMoney(&bank)) {
                saveAfterChange(&bank);
            }
        } else if (strcmp(choice, "8") == 0) {
            if (transferMoney(&bank)) {
                saveAfterChange(&bank);
            }
        } else if (strcmp(choice, "9") == 0) {
            checkBalance(&bank);
        } else if (strcmp(choice, "10") == 0) {
            displayTransactionHistory(&bank);
        } else if (strcmp(choice, "11") == 0) {
            listAccounts(&bank);
        } else if (strcmp(choice, "12") == 0) {
            showStatistics(&bank);
        } else if (strcmp(choice, "13") == 0) {
            if (saveBank(&bank)) {
                printf("Data saved successfully.\n");
            } else {
                printf("Save failed.\n");
            }
        } else if (strcmp(choice, "14") == 0) {
            loadResult = loadBank(&bank);
            if (loadResult == 1) {
                printf("Data loaded successfully.\n");
            } else if (loadResult == 0) {
                printf("No saved data file exists yet.\n");
            } else {
                printf("Load failed. Existing data in memory was kept.\n");
            }
        } else if (strcmp(choice, "15") == 0) {
            showActivitySummary(&bank);
        } else if (strcmp(choice, "0") == 0) {
            if (saveBank(&bank)) {
                printf("Data saved. Goodbye.\n");
            } else {
                printf("Save failed. Goodbye.\n");
            }
            break;
        } else {
            printf("Invalid menu choice. Enter a number from 0 to 15.\n");
        }
    }

    return 0;
}

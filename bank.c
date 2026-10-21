#include "bank.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static int findAccountIndex(const Bank *bank, int accountNumber) {
    int index;

    for (index = 0; index < bank->accountCount; index++) {
        if (bank->accounts[index].accountNumber == accountNumber) {
            return index;
        }
    }
    return -1;
}

static const char *transactionTypeName(TransactionType type) {
    switch (type) {
        case INITIAL_DEPOSIT:
            return "INITIAL";
        case DEPOSIT:
            return "DEPOSIT";
        case WITHDRAWAL:
            return "WITHDRAW";
        case TRANSFER_OUT:
            return "TRANSFER OUT";
        case TRANSFER_IN:
            return "TRANSFER IN";
        default:
            return "UNKNOWN";
    }
}

static void recordTransaction(Bank *bank, int accountNumber,
                              TransactionType type, long long amount,
                              int relatedAccountNumber) {
    Transaction *transaction = &bank->transactions[bank->transactionCount];
    time_t now = time(NULL);
    struct tm *local = localtime(&now);

    transaction->transactionId = bank->nextTransactionId;
    transaction->accountNumber = accountNumber;
    transaction->relatedAccountNumber = relatedAccountNumber;
    transaction->type = type;
    transaction->amountInPaise = amount;
    if (local == NULL || strftime(transaction->timestamp,
                                  sizeof(transaction->timestamp),
                                  "%Y-%m-%d %H:%M:%S", local) == 0) {
        strcpy(transaction->timestamp, "time unavailable");
    }

    bank->transactionCount++;
    bank->nextTransactionId++;
}

static void displayAccount(const Account *account) {
    printf("----------------------------------------\n");
    printf("Account number : %d\n", account->accountNumber);
    printf("Account holder : %s\n", account->name);
    printf("Phone          : %s\n", account->phone);
    printf("Account type   : %s\n", accountTypeName(account->type));
    printf("Status         : %s\n", account->active ? "Active" : "Closed");
    printf("Balance        : ");
    printMoney(account->balanceInPaise);
    printf("\n----------------------------------------\n");
}

static int selectExistingAccount(const Bank *bank, int *index) {
    int accountNumber;

    if (bank->accountCount == 0) {
        printf("No accounts have been created yet.\n");
        return 0;
    }
    if (!promptAccountNumber(&accountNumber)) {
        return 0;
    }
    *index = findAccountIndex(bank, accountNumber);
    if (*index == -1) {
        printf("No account exists with number %d.\n", accountNumber);
        return 0;
    }
    return 1;
}

static int selectActiveAccount(const Bank *bank, int *index) {
    if (!selectExistingAccount(bank, index)) {
        return 0;
    }
    if (!bank->accounts[*index].active) {
        printf("This account is closed. Financial operations are not allowed.\n");
        return 0;
    }
    return 1;
}

void initializeBank(Bank *bank) {
    bank->accountCount = 0;
    bank->transactionCount = 0;
    bank->nextAccountNumber = FIRST_ACCOUNT_NUMBER;
    bank->nextTransactionId = FIRST_TRANSACTION_ID;
}

void displayMainMenu(void) {
    printf("\n========================================\n");
    printf("              BANKCORE\n");
    printf("       Banking Management System\n");
    printf("========================================\n");
    printf(" 1. Create Account\n");
    printf(" 2. View Account\n");
    printf(" 3. Search Accounts\n");
    printf(" 4. Update Customer Information\n");
    printf(" 5. Close Account\n");
    printf("\n 6. Deposit Money\n");
    printf(" 7. Withdraw Money\n");
    printf(" 8. Transfer Money\n");
    printf(" 9. Check Balance\n");
    printf("\n10. Transaction History\n");
    printf("11. List All Accounts\n");
    printf("12. Bank Statistics\n");
    printf("13. Save Data\n");
    printf("14. Load Data\n");
    printf("15. Account Activity Summary\n");
    printf("\n 0. Exit\n");
    printf("========================================\n");
    printf("Choose an option: ");
}

int createAccount(Bank *bank) {
    Account account;
    long long initialDeposit;

    if (bank->accountCount >= MAX_ACCOUNTS) {
        printf("Cannot create an account: account capacity reached.\n");
        return 0;
    }
    if (bank->transactionCount >= MAX_TRANSACTIONS) {
        printf("Cannot create an account: transaction capacity reached.\n");
        return 0;
    }

    printf("\n--- Create Account ---\n");
    do {
        printf("Customer name: ");
        if (!readLine(account.name, sizeof(account.name))) {
            return 0;
        }
        if (!isValidName(account.name)) {
            printf("Name cannot be empty.\n");
        }
    } while (!isValidName(account.name));

    do {
        printf("Phone number (10 digits): ");
        if (!readLine(account.phone, sizeof(account.phone))) {
            return 0;
        }
        if (!isValidPhone(account.phone)) {
            printf("Phone number must contain exactly 10 digits.\n");
        }
    } while (!isValidPhone(account.phone));

    if (!promptAccountType(&account.type,
                           "Account type (S = Savings, C = Current): ") ||
        !promptAmount(&initialDeposit,
                      "Initial deposit (for example, 100.50): Rs. ")) {
        return 0;
    }

    account.accountNumber = bank->nextAccountNumber;
    account.balanceInPaise = initialDeposit;
    account.active = 1;
    bank->accounts[bank->accountCount] = account;
    bank->accountCount++;
    bank->nextAccountNumber++;
    recordTransaction(bank, account.accountNumber, INITIAL_DEPOSIT,
                      initialDeposit, 0);

    printf("Account created successfully.\n");
    displayAccount(&account);
    return 1;
}

void viewAccount(const Bank *bank) {
    int index;

    printf("\n--- View Account ---\n");
    if (selectExistingAccount(bank, &index)) {
        displayAccount(&bank->accounts[index]);
    }
}

void searchAccounts(const Bank *bank) {
    char choice[20];
    char input[NAME_LENGTH];
    AccountType type;
    int accountNumber;
    int index;
    int found;

    if (bank->accountCount == 0) {
        printf("No accounts have been created yet.\n");
        return;
    }

    while (1) {
        printf("\n--- Search Accounts ---\n");
        printf("1. Account Number\n2. Exact Name\n3. Phone Number\n4. Account Type\n0. Return\n");
        printf("Choose a search method: ");
        if (!readLine(choice, sizeof(choice))) {
            return;
        }

        if (strcmp(choice, "0") == 0) {
            return;
        }
        if (strcmp(choice, "1") == 0) {
            if (!promptAccountNumber(&accountNumber)) {
                return;
            }
            index = findAccountIndex(bank, accountNumber);
            if (index == -1) {
                printf("No account exists with number %d.\n", accountNumber);
            } else {
                displayAccount(&bank->accounts[index]);
            }
        } else if (strcmp(choice, "2") == 0) {
            printf("Exact customer name: ");
            if (!readLine(input, sizeof(input))) {
                return;
            }
            if (!isValidName(input)) {
                printf("Name cannot be empty.\n");
                continue;
            }
            found = 0;
            for (index = 0; index < bank->accountCount; index++) {
                if (strcmp(bank->accounts[index].name, input) == 0) {
                    displayAccount(&bank->accounts[index]);
                    found = 1;
                }
            }
            if (!found) {
                printf("No account matches that name.\n");
            }
        } else if (strcmp(choice, "3") == 0) {
            printf("Phone number (10 digits): ");
            if (!readLine(input, sizeof(input))) {
                return;
            }
            if (!isValidPhone(input)) {
                printf("Phone number must contain exactly 10 digits.\n");
                continue;
            }
            found = 0;
            for (index = 0; index < bank->accountCount; index++) {
                if (strcmp(bank->accounts[index].phone, input) == 0) {
                    displayAccount(&bank->accounts[index]);
                    found = 1;
                }
            }
            if (!found) {
                printf("No account matches that phone number.\n");
            }
        } else if (strcmp(choice, "4") == 0) {
            if (!promptAccountType(&type, "Account type (S or C): ")) {
                return;
            }
            found = 0;
            for (index = 0; index < bank->accountCount; index++) {
                if (bank->accounts[index].type == type) {
                    displayAccount(&bank->accounts[index]);
                    found = 1;
                }
            }
            if (!found) {
                printf("No accounts match that type.\n");
            }
        } else {
            printf("Invalid search choice.\n");
        }
    }
}

int updateAccount(Bank *bank) {
    Account *account;
    char input[NAME_LENGTH];
    int index;
    int changed = 0;

    printf("\n--- Update Customer Information ---\n");
    if (!selectExistingAccount(bank, &index)) {
        return 0;
    }
    account = &bank->accounts[index];
    displayAccount(account);

    printf("New name (blank keeps current value): ");
    if (!readLine(input, sizeof(input))) {
        return 0;
    }
    if (input[0] != '\0') {
        if (!isValidName(input)) {
            printf("Name cannot contain only spaces; name was not changed.\n");
        } else {
            strcpy(account->name, input);
            changed = 1;
        }
    }

    printf("New phone (10 digits, blank keeps current value): ");
    if (!readLine(input, sizeof(input))) {
        return 0;
    }
    if (input[0] != '\0') {
        if (!isValidPhone(input)) {
            printf("Invalid phone number; phone was not changed.\n");
        } else {
            strcpy(account->phone, input);
            changed = 1;
        }
    }

    printf("New type (S/C, blank keeps current value): ");
    if (!readLine(input, sizeof(input))) {
        return 0;
    }
    if (input[0] != '\0') {
        if (strcmp(input, "S") == 0 || strcmp(input, "s") == 0) {
            account->type = SAVINGS;
            changed = 1;
        } else if (strcmp(input, "C") == 0 || strcmp(input, "c") == 0) {
            account->type = CURRENT;
            changed = 1;
        } else {
            printf("Invalid account type; type was not changed.\n");
        }
    }

    if (changed) {
        printf("Account information updated.\n");
    } else {
        printf("No account information was changed.\n");
    }
    return changed;
}

int closeAccount(Bank *bank) {
    int index;

    printf("\n--- Close Account ---\n");
    if (!selectExistingAccount(bank, &index)) {
        return 0;
    }
    if (!bank->accounts[index].active) {
        printf("This account is already closed.\n");
        return 0;
    }

    bank->accounts[index].active = 0;
    printf("Account %d is now closed. Its history remains available.\n",
           bank->accounts[index].accountNumber);
    return 1;
}

int depositMoney(Bank *bank) {
    int index;
    long long amount;

    printf("\n--- Deposit Money ---\n");
    if (!selectActiveAccount(bank, &index)) {
        return 0;
    }
    if (bank->transactionCount >= MAX_TRANSACTIONS) {
        printf("Cannot record another transaction: capacity reached.\n");
        return 0;
    }
    if (!promptAmount(&amount, "Deposit amount: Rs. ")) {
        return 0;
    }
    if (bank->accounts[index].balanceInPaise > LLONG_MAX - amount) {
        printf("Deposit rejected: balance would be too large.\n");
        return 0;
    }

    bank->accounts[index].balanceInPaise += amount;
    recordTransaction(bank, bank->accounts[index].accountNumber, DEPOSIT,
                      amount, 0);
    printf("Deposit successful. New balance: ");
    printMoney(bank->accounts[index].balanceInPaise);
    printf("\n");
    return 1;
}

int withdrawMoney(Bank *bank) {
    int index;
    long long amount;

    printf("\n--- Withdraw Money ---\n");
    if (!selectActiveAccount(bank, &index)) {
        return 0;
    }
    if (bank->transactionCount >= MAX_TRANSACTIONS) {
        printf("Cannot record another transaction: capacity reached.\n");
        return 0;
    }
    if (!promptAmount(&amount, "Withdrawal amount: Rs. ")) {
        return 0;
    }
    if (amount > bank->accounts[index].balanceInPaise) {
        printf("Withdrawal rejected: insufficient balance.\n");
        return 0;
    }

    bank->accounts[index].balanceInPaise -= amount;
    recordTransaction(bank, bank->accounts[index].accountNumber, WITHDRAWAL,
                      amount, 0);
    printf("Withdrawal successful. New balance: ");
    printMoney(bank->accounts[index].balanceInPaise);
    printf("\n");
    return 1;
}

int transferMoney(Bank *bank) {
    int senderIndex;
    int receiverIndex;
    int receiverNumber;
    long long amount;

    printf("\n--- Transfer Money ---\n");
    printf("Sender details\n");
    if (!selectActiveAccount(bank, &senderIndex)) {
        return 0;
    }
    printf("Receiver details\n");
    if (!promptAccountNumber(&receiverNumber)) {
        return 0;
    }
    receiverIndex = findAccountIndex(bank, receiverNumber);
    if (receiverIndex == -1) {
        printf("No receiver account exists with number %d.\n", receiverNumber);
        return 0;
    }
    if (receiverIndex == senderIndex) {
        printf("Transfer rejected: sender and receiver cannot be the same.\n");
        return 0;
    }
    if (!bank->accounts[receiverIndex].active) {
        printf("Transfer rejected: receiver account is closed.\n");
        return 0;
    }
    if (bank->transactionCount > MAX_TRANSACTIONS - 2) {
        printf("Cannot record this transfer: transaction capacity reached.\n");
        return 0;
    }
    if (!promptAmount(&amount, "Transfer amount: Rs. ")) {
        return 0;
    }
    if (amount > bank->accounts[senderIndex].balanceInPaise) {
        printf("Transfer rejected: insufficient balance.\n");
        return 0;
    }
    if (bank->accounts[receiverIndex].balanceInPaise > LLONG_MAX - amount) {
        printf("Transfer rejected: receiver balance would be too large.\n");
        return 0;
    }

    /* All checks are complete before either balance is changed. */
    bank->accounts[senderIndex].balanceInPaise -= amount;
    bank->accounts[receiverIndex].balanceInPaise += amount;
    recordTransaction(bank, bank->accounts[senderIndex].accountNumber,
                      TRANSFER_OUT, amount,
                      bank->accounts[receiverIndex].accountNumber);
    recordTransaction(bank, bank->accounts[receiverIndex].accountNumber,
                      TRANSFER_IN, amount,
                      bank->accounts[senderIndex].accountNumber);
    printf("Transfer successful. Sender balance: ");
    printMoney(bank->accounts[senderIndex].balanceInPaise);
    printf("\n");
    return 1;
}

void checkBalance(const Bank *bank) {
    int index;

    printf("\n--- Check Balance ---\n");
    if (selectExistingAccount(bank, &index)) {
        printf("Account %d balance: ", bank->accounts[index].accountNumber);
        printMoney(bank->accounts[index].balanceInPaise);
        printf("\n");
    }
}

void displayTransactionHistory(const Bank *bank) {
    int accountNumber;
    int index;
    int found = 0;

    printf("\n--- Transaction History ---\n");
    if (!selectExistingAccount(bank, &index)) {
        return;
    }
    accountNumber = bank->accounts[index].accountNumber;
    printf("Txn ID    Date/Time           Type           Amount       Related\n");
    printf("----------------------------------------------------------------\n");
    for (index = 0; index < bank->transactionCount; index++) {
        const Transaction *transaction = &bank->transactions[index];

        if (transaction->accountNumber == accountNumber) {
            printf("%-9ld %-19s %-14s ", transaction->transactionId,
                   transaction->timestamp, transactionTypeName(transaction->type));
            printMoney(transaction->amountInPaise);
            if (transaction->relatedAccountNumber == 0) {
                printf("  -\n");
            } else {
                printf("  %d\n", transaction->relatedAccountNumber);
            }
            found = 1;
        }
    }
    if (!found) {
        printf("No transactions found.\n");
    }
    printf("Current balance: ");
    printMoney(bank->accounts[findAccountIndex(bank, accountNumber)].balanceInPaise);
    printf("\n");
}

static int comesBefore(const Account *left, const Account *right, int criterion) {
    if (criterion == 1) {
        return left->accountNumber < right->accountNumber;
    }
    if (criterion == 2) {
        return strcmp(left->name, right->name) < 0;
    }
    return left->balanceInPaise < right->balanceInPaise;
}

static void selectionSortAccounts(Account accounts[], int count, int criterion) {
    int start;

    for (start = 0; start < count - 1; start++) {
        int smallest = start;
        int current;
        Account temporary;

        for (current = start + 1; current < count; current++) {
            if (comesBefore(&accounts[current], &accounts[smallest], criterion)) {
                smallest = current;
            }
        }
        if (smallest != start) {
            temporary = accounts[start];
            accounts[start] = accounts[smallest];
            accounts[smallest] = temporary;
        }
    }
}

void listAccounts(const Bank *bank) {
    Account copy[MAX_ACCOUNTS];
    char choice[20];
    int index;
    int criterion = 0;

    if (bank->accountCount == 0) {
        printf("No accounts have been created yet.\n");
        return;
    }
    printf("\nSort list by: 1. Account number  2. Name  3. Balance  0. No sorting\n");
    printf("Choose an option: ");
    if (!readLine(choice, sizeof(choice))) {
        return;
    }
    if (strcmp(choice, "1") == 0) {
        criterion = 1;
    } else if (strcmp(choice, "2") == 0) {
        criterion = 2;
    } else if (strcmp(choice, "3") == 0) {
        criterion = 3;
    } else if (strcmp(choice, "0") != 0) {
        printf("Invalid sorting choice.\n");
        return;
    }

    for (index = 0; index < bank->accountCount; index++) {
        copy[index] = bank->accounts[index];
    }
    if (criterion != 0) {
        selectionSortAccounts(copy, bank->accountCount, criterion);
    }

    printf("\nAccount No.  Name                 Type      Status    Balance\n");
    printf("----------------------------------------------------------------\n");
    for (index = 0; index < bank->accountCount; index++) {
        printf("%-12d %-20.20s %-9s %-8s ", copy[index].accountNumber,
               copy[index].name, accountTypeName(copy[index].type),
               copy[index].active ? "Active" : "Closed");
        printMoney(copy[index].balanceInPaise);
        printf("\n");
    }
}

void showStatistics(const Bank *bank) {
    int index;
    int active = 0;
    int closed = 0;
    int savings = 0;
    int current = 0;
    int overflow = 0;
    long long totalBalance = 0;

    for (index = 0; index < bank->accountCount; index++) {
        const Account *account = &bank->accounts[index];

        if (account->active) {
            active++;
        } else {
            closed++;
        }
        if (account->type == SAVINGS) {
            savings++;
        } else {
            current++;
        }
        if (totalBalance > LLONG_MAX - account->balanceInPaise) {
            overflow = 1;
        } else if (!overflow) {
            totalBalance += account->balanceInPaise;
        }
    }

    printf("\n========================================\n");
    printf("           BANK STATISTICS\n");
    printf("========================================\n");
    printf("Total Accounts       : %d\n", bank->accountCount);
    printf("Active Accounts      : %d\n", active);
    printf("Closed Accounts      : %d\n", closed);
    printf("Savings Accounts     : %d\n", savings);
    printf("Current Accounts     : %d\n", current);
    if (overflow) {
        printf("Total Bank Balance   : too large to display\n");
    } else {
        printf("Total Bank Balance   : ");
        printMoney(totalBalance);
        printf("\n");
        if (bank->accountCount > 0) {
            printf("Average Balance      : ");
            printMoney(totalBalance / bank->accountCount);
            printf("\n");
        }
    }
    printf("Total Transactions   : %d\n", bank->transactionCount);
    printf("========================================\n");
}

void showActivitySummary(const Bank *bank) {
    int index;
    int accountNumber;
    int deposits = 0;
    int withdrawals = 0;
    int transfers = 0;
    long long totalDeposited = 0;
    long long totalWithdrawn = 0;
    int activityCount;
    const char *level;

    printf("\n--- Account Activity Summary ---\n");
    if (!selectExistingAccount(bank, &index)) {
        return;
    }
    accountNumber = bank->accounts[index].accountNumber;

    for (index = 0; index < bank->transactionCount; index++) {
        const Transaction *transaction = &bank->transactions[index];

        if (transaction->accountNumber != accountNumber) {
            continue;
        }
        if (transaction->type == INITIAL_DEPOSIT || transaction->type == DEPOSIT) {
            deposits++;
            totalDeposited += transaction->amountInPaise;
        } else if (transaction->type == WITHDRAWAL) {
            withdrawals++;
            totalWithdrawn += transaction->amountInPaise;
        } else {
            transfers++;
        }
    }

    activityCount = deposits + withdrawals + transfers;
    if (activityCount >= 10) {
        level = "HIGH";
    } else if (activityCount >= 4) {
        level = "MEDIUM";
    } else {
        level = "LOW";
    }

    printf("Account number     : %d\n", accountNumber);
    printf("Current Balance    : ");
    printMoney(bank->accounts[findAccountIndex(bank, accountNumber)].balanceInPaise);
    printf("\nDeposits          : %d\n", deposits);
    printf("Withdrawals        : %d\n", withdrawals);
    printf("Transfers          : %d\n", transfers);
    printf("Total Deposited    : ");
    printMoney(totalDeposited);
    printf("\nTotal Withdrawn    : ");
    printMoney(totalWithdrawn);
    printf("\nActivity Level     : %s\n", level);
}

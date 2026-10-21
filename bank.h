#ifndef BANK_H
#define BANK_H

#define MAX_ACCOUNTS 100
#define MAX_TRANSACTIONS 1000
#define NAME_LENGTH 50
#define PHONE_LENGTH 15
#define TIMESTAMP_LENGTH 20
#define FIRST_ACCOUNT_NUMBER 100001
#define FIRST_TRANSACTION_ID 10001L

typedef enum {
    SAVINGS,
    CURRENT
} AccountType;

typedef enum {
    INITIAL_DEPOSIT,
    DEPOSIT,
    WITHDRAWAL,
    TRANSFER_OUT,
    TRANSFER_IN
} TransactionType;

typedef struct {
    int accountNumber;
    char name[NAME_LENGTH];
    char phone[PHONE_LENGTH];
    AccountType type;
    long long balanceInPaise;
    int active;
} Account;

typedef struct {
    long transactionId;
    int accountNumber;
    int relatedAccountNumber;
    TransactionType type;
    long long amountInPaise;
    char timestamp[TIMESTAMP_LENGTH];
} Transaction;

typedef struct {
    Account accounts[MAX_ACCOUNTS];
    Transaction transactions[MAX_TRANSACTIONS];
    int accountCount;
    int transactionCount;
    int nextAccountNumber;
    long nextTransactionId;
} Bank;

/* Input and validation functions. */
int readLine(char text[], int size);
int isValidName(const char name[]);
int isValidPhone(const char phone[]);
int promptAccountNumber(int *accountNumber);
int promptAmount(long long *amountInPaise, const char prompt[]);
int promptAccountType(AccountType *type, const char prompt[]);
const char *accountTypeName(AccountType type);
void printMoney(long long amountInPaise);

/* Banking functions. A return value of 1 means the Bank data changed. */
void initializeBank(Bank *bank);
void displayMainMenu(void);
int createAccount(Bank *bank);
void viewAccount(const Bank *bank);
void searchAccounts(const Bank *bank);
int updateAccount(Bank *bank);
int closeAccount(Bank *bank);
int depositMoney(Bank *bank);
int withdrawMoney(Bank *bank);
int transferMoney(Bank *bank);
void checkBalance(const Bank *bank);
void displayTransactionHistory(const Bank *bank);
void listAccounts(const Bank *bank);
void showStatistics(const Bank *bank);
void showActivitySummary(const Bank *bank);

/* Persistence functions. loadBank returns 1, 0 (files absent), or -1. */
int saveBank(const Bank *bank);
int loadBank(Bank *bank);

#endif

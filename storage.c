#include "bank.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#define DATA_FILE "bankcore.dat"
#define TEMP_FILE "bankcore.tmp"
#define BACKUP_FILE "bankcore.bak"
#define DATA_VERSION 1

typedef struct {
    char magic[8];
    int version;
    int accountCount;
    int transactionCount;
    long nextAccountNumber;
    long nextTransactionId;
} DataHeader;

static const char DATA_MAGIC[8] = {'B', 'A', 'N', 'K', 'C', 'O', 'R', 'E'};

static int writeBankFile(const Bank *bank) {
    DataHeader header;
    FILE *file;
    int writeSucceeded;

    memcpy(header.magic, DATA_MAGIC, sizeof(header.magic));
    header.version = DATA_VERSION;
    header.accountCount = bank->accountCount;
    header.transactionCount = bank->transactionCount;
    header.nextAccountNumber = bank->nextAccountNumber;
    header.nextTransactionId = bank->nextTransactionId;

    file = fopen(TEMP_FILE, "wb");
    if (file == NULL) {
        return 0;
    }
    writeSucceeded = fwrite(&header, sizeof(header), 1, file) == 1 &&
                     (bank->accountCount == 0 ||
                      fwrite(bank->accounts, sizeof(Account),
                             (size_t)bank->accountCount, file) ==
                          (size_t)bank->accountCount) &&
                     (bank->transactionCount == 0 ||
                      fwrite(bank->transactions, sizeof(Transaction),
                             (size_t)bank->transactionCount, file) ==
                          (size_t)bank->transactionCount);
    if (fclose(file) != 0) {
        writeSucceeded = 0;
    }
    if (!writeSucceeded) {
        remove(TEMP_FILE);
        return 0;
    }
    return 1;
}

static int replaceDataFile(void) {
    remove(BACKUP_FILE);
    rename(DATA_FILE, BACKUP_FILE); /* It is fine when no old file exists. */

    if (rename(TEMP_FILE, DATA_FILE) != 0) {
        rename(BACKUP_FILE, DATA_FILE);
        remove(TEMP_FILE);
        return 0;
    }
    remove(BACKUP_FILE);
    return 1;
}

int saveBank(const Bank *bank) {
    if (!writeBankFile(bank)) {
        printf("Save failed: could not write %s.\n", TEMP_FILE);
        return 0;
    }
    if (!replaceDataFile()) {
        printf("Save failed: could not replace %s.\n", DATA_FILE);
        return 0;
    }
    return 1;
}

static int isValidLoadedBank(const Bank *bank) {
    int index;
    int other;

    if (bank->accountCount < 0 || bank->accountCount > MAX_ACCOUNTS ||
        bank->transactionCount < 0 || bank->transactionCount > MAX_TRANSACTIONS ||
        bank->nextAccountNumber < FIRST_ACCOUNT_NUMBER ||
        bank->nextTransactionId < FIRST_TRANSACTION_ID) {
        return 0;
    }
    for (index = 0; index < bank->accountCount; index++) {
        const Account *account = &bank->accounts[index];

        if (account->accountNumber <= 0 || !isValidName(account->name) ||
            !isValidPhone(account->phone) ||
            (account->type != SAVINGS && account->type != CURRENT) ||
            account->balanceInPaise < 0 ||
            (account->active != 0 && account->active != 1) ||
            account->accountNumber >= bank->nextAccountNumber) {
            return 0;
        }
        for (other = index + 1; other < bank->accountCount; other++) {
            if (account->accountNumber == bank->accounts[other].accountNumber) {
                return 0;
            }
        }
    }
    for (index = 0; index < bank->transactionCount; index++) {
        const Transaction *transaction = &bank->transactions[index];
        int accountFound = 0;
        int relatedFound = transaction->relatedAccountNumber == 0;

        if (transaction->transactionId < FIRST_TRANSACTION_ID ||
            transaction->transactionId >= bank->nextTransactionId ||
            transaction->accountNumber <= 0 || transaction->amountInPaise <= 0 ||
            transaction->type < INITIAL_DEPOSIT || transaction->type > TRANSFER_IN ||
            memchr(transaction->timestamp, '\0', TIMESTAMP_LENGTH) == NULL) {
            return 0;
        }
        if ((transaction->type == TRANSFER_OUT ||
             transaction->type == TRANSFER_IN) &&
            transaction->relatedAccountNumber <= 0) {
            return 0;
        }
        if (transaction->type != TRANSFER_OUT && transaction->type != TRANSFER_IN &&
            transaction->relatedAccountNumber != 0) {
            return 0;
        }
        for (other = 0; other < bank->accountCount; other++) {
            if (bank->accounts[other].accountNumber == transaction->accountNumber) {
                accountFound = 1;
            }
            if (bank->accounts[other].accountNumber ==
                transaction->relatedAccountNumber) {
                relatedFound = 1;
            }
        }
        if (!accountFound || !relatedFound) {
            return 0;
        }
        for (other = index + 1; other < bank->transactionCount; other++) {
            if (transaction->transactionId ==
                bank->transactions[other].transactionId) {
                return 0;
            }
        }
    }
    return 1;
}

int loadBank(Bank *bank) {
    DataHeader header;
    Bank loaded;
    FILE *file = fopen(DATA_FILE, "rb");
    int readSucceeded;

    if (file == NULL) {
        return errno == ENOENT ? 0 : -1;
    }
    if (fread(&header, sizeof(header), 1, file) != 1 ||
        memcmp(header.magic, DATA_MAGIC, sizeof(header.magic)) != 0 ||
        header.version != DATA_VERSION || header.accountCount < 0 ||
        header.accountCount > MAX_ACCOUNTS || header.transactionCount < 0 ||
        header.transactionCount > MAX_TRANSACTIONS ||
        header.nextAccountNumber < FIRST_ACCOUNT_NUMBER ||
        header.nextAccountNumber > INT_MAX ||
        header.nextTransactionId < FIRST_TRANSACTION_ID) {
        fclose(file);
        return -1;
    }

    initializeBank(&loaded);
    loaded.accountCount = header.accountCount;
    loaded.transactionCount = header.transactionCount;
    loaded.nextAccountNumber = (int)header.nextAccountNumber;
    loaded.nextTransactionId = header.nextTransactionId;

    readSucceeded = (loaded.accountCount == 0 ||
                     fread(loaded.accounts, sizeof(Account),
                           (size_t)loaded.accountCount, file) ==
                         (size_t)loaded.accountCount) &&
                    (loaded.transactionCount == 0 ||
                     fread(loaded.transactions, sizeof(Transaction),
                           (size_t)loaded.transactionCount, file) ==
                         (size_t)loaded.transactionCount);
    if (fclose(file) != 0) {
        readSucceeded = 0;
    }
    if (!readSucceeded || !isValidLoadedBank(&loaded)) {
        return -1;
    }

    *bank = loaded;
    return 1;
}

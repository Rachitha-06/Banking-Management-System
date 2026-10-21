#include "bank.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

int readLine(char text[], int size) {
    int character;
    char *newline;

    if (fgets(text, size, stdin) == NULL) {
        return 0;
    }

    newline = strchr(text, '\n');
    if (newline != NULL) {
        *newline = '\0';
    } else {
        /* Discard remaining characters so they cannot affect the next prompt. */
        while ((character = getchar()) != '\n' && character != EOF) {
        }
    }

    return 1;
}

int isValidName(const char name[]) {
    int index = 0;

    while (name[index] != '\0') {
        if (!isspace((unsigned char)name[index])) {
            return 1;
        }
        index++;
    }

    return 0;
}

int isValidPhone(const char phone[]) {
    int index;

    if (strlen(phone) != 10) {
        return 0;
    }

    for (index = 0; index < 10; index++) {
        if (!isdigit((unsigned char)phone[index])) {
            return 0;
        }
    }

    return 1;
}

static int parsePositiveInt(const char text[], int *value) {
    long long result = 0;
    int index = 0;

    if (!isdigit((unsigned char)text[index])) {
        return 0;
    }

    while (isdigit((unsigned char)text[index])) {
        int digit = text[index] - '0';

        if (result > (INT_MAX - digit) / 10) {
            return 0;
        }
        result = result * 10 + digit;
        index++;
    }

    if (text[index] != '\0' || result <= 0) {
        return 0;
    }

    *value = (int)result;
    return 1;
}

int promptAccountNumber(int *accountNumber) {
    char input[30];

    while (1) {
        printf("Account number: ");
        if (!readLine(input, sizeof(input))) {
            return 0;
        }
        if (parsePositiveInt(input, accountNumber)) {
            return 1;
        }
        printf("Invalid account number. Enter positive digits only.\n");
    }
}

static int parseAmountInPaise(const char text[], long long *amountInPaise) {
    long long rupees = 0;
    int paise = 0;
    int decimalDigits = 0;
    int index = 0;

    if (!isdigit((unsigned char)text[index])) {
        return 0;
    }

    while (isdigit((unsigned char)text[index])) {
        int digit = text[index] - '0';

        if (rupees > (LLONG_MAX - digit) / 10) {
            return 0;
        }
        rupees = rupees * 10 + digit;
        index++;
    }

    if (text[index] == '.') {
        index++;
        while (isdigit((unsigned char)text[index]) && decimalDigits < 2) {
            paise = paise * 10 + (text[index] - '0');
            decimalDigits++;
            index++;
        }
        if (decimalDigits == 0 || isdigit((unsigned char)text[index])) {
            return 0;
        }
        if (decimalDigits == 1) {
            paise *= 10;
        }
    }

    if (text[index] != '\0' || rupees > (LLONG_MAX - paise) / 100) {
        return 0;
    }

    *amountInPaise = rupees * 100 + paise;
    return *amountInPaise > 0;
}

int promptAmount(long long *amountInPaise, const char prompt[]) {
    char input[40];

    while (1) {
        printf("%s", prompt);
        if (!readLine(input, sizeof(input))) {
            return 0;
        }
        if (parseAmountInPaise(input, amountInPaise)) {
            return 1;
        }
        printf("Invalid amount. Enter a positive value with at most two decimal places.\n");
    }
}

int promptAccountType(AccountType *type, const char prompt[]) {
    char input[10];

    while (1) {
        printf("%s", prompt);
        if (!readLine(input, sizeof(input))) {
            return 0;
        }
        if (strcmp(input, "S") == 0 || strcmp(input, "s") == 0) {
            *type = SAVINGS;
            return 1;
        }
        if (strcmp(input, "C") == 0 || strcmp(input, "c") == 0) {
            *type = CURRENT;
            return 1;
        }
        printf("Invalid account type. Enter S or C.\n");
    }
}

const char *accountTypeName(AccountType type) {
    return type == SAVINGS ? "Savings" : "Current";
}

void printMoney(long long amountInPaise) {
    printf("Rs. %lld.%02lld", amountInPaise / 100, amountInPaise % 100);
}

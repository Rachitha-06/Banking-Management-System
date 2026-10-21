# Banking-Management-System
# BankCore

BankCore is an educational, console-based banking simulation written in C. It is designed to practise structured programming, structures, arrays, safe string input, validation, file handling, searching, sorting, and defensive error handling. It is **not** a real banking system.

## Build and run

From this folder, compile with GCC:

```powershell
gcc -std=c11 -Wall -Wextra -Werror -pedantic main.c bank.c input.c storage.c -o bankcore.exe
.\bankcore.exe
```

## Features

- Automatically numbered Savings and Current accounts, beginning at `100001`.
- View, exact-name search, phone search, account-type search, and customer-information updates.
- Logical account closure: the account and its history remain visible, but no financial operation is allowed.
- Exact money storage in integer paise: `100.50` is stored as `10050`, avoiding floating-point rounding errors.
- Deposit, withdrawal, and transfer validation.
- Transfer validation happens before changing either balance; each successful transfer creates a `TRANSFER OUT` and `TRANSFER IN` record.
- Transaction IDs, timestamps, history, account activity summaries, sorting, and bank statistics.
- Automatic persistence after each successful change, plus explicit Save and Load menu options.

## Project structure

```text
BankCore/
  main.c       menu loop and application flow
  bank.h       shared structures, constants, and function declarations
  input.c      safe line input and validation functions
  bank.c       account, transaction, search, sort, and statistics logic
  storage.c    binary file save/load logic
  README.md
  bankcore.dat created automatically when data is saved
```

The delivered folder currently includes a small test data file (`bankcore.dat`) with two accounts and six transactions, so you can immediately inspect persistence and history. Delete that file before running the program if you prefer an empty bank.

## Important design choices

`Bank` is one structure that owns the fixed-size account and transaction arrays along with their counts and next IDs. This avoids mutable global variables and makes it clear which data a banking function can change.

`AccountType` and `TransactionType` are enums. They make invalid internal type values less likely than using arbitrary strings or characters.

Search uses a linear scan, which is intentionally easy to understand and suitable for the educational limit of 100 accounts. Listing can use selection sort by account number, name, or balance. Sorting operates on a copy, so it does not change the stored account order.

The program uses `fgets()` through `readLine()` rather than mixing `scanf()` and `fgets()`. `readLine()` removes the trailing newline and discards excess input when a line is too long.

## File handling

`storage.c` uses `fopen`, `fwrite`, `fread`, `fclose`, and `rename` with a temporary file and a backup file. The saved file contains a small header, followed by account records and transaction records. On load, the header, record counts, account fields, and transaction fields are checked before replacing the in-memory bank.

The current simple sequential format does not need `fseek()` or `rewind()`: it writes and reads each record in order. Those functions are useful in a future version that needs random access to a specific file record.

## Test checklist

1. Create two accounts. Verify account numbers are `100001` and `100002`.
2. Deposit `200.50`; verify both the balance and transaction history change.
3. Withdraw more than the balance; verify the request is rejected and no transaction is added.
4. Transfer `300` between two active accounts; verify both balances and both transaction records.
5. Try a self-transfer and a transfer to a closed account; both must fail without changing money.
6. Close an account, then try deposit, withdrawal, transfer sender, and transfer receiver operations; all must fail.
7. Exit, run the program again, and view an account to verify data reloads.
8. Enter invalid menu choices, blank names, invalid phone numbers, negative amounts, zero, letters for amounts, and values with three decimal places; all must be rejected.

## Known limitations

- This is a single-user local simulation, not production banking software.
- It has fixed capacities of 100 accounts and 1000 transactions.
- It has no authentication, encryption, database, concurrent-user control, audit security, or crash-proof multi-file transaction system.
- Binary files are intended for the same program version and environment; changing the structures may make old files incompatible.
- A sudden power failure during the small replace window can still leave a backup file. Real systems use a database or journaled transaction log.

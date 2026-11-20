#pragma once
#include "BankClasses.h"
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

namespace BankSystem
{
    enum class TransactionType
    {
        INITIAL_DEPOSIT = 1,
        TOP_UP = 2,
        WITHDRAWAL = 3,
        INTEREST_ACCRUAL = 4,
        CLOSURE = 5
    };

    class TransactionException : public BankException
    {
    public:
        explicit TransactionException(const string &msg)
            : BankException("Ошибка транзакции: " + msg) {}
    };

    class Transaction
    {
    private:
        int id;
        int accountId;
        string contractNumber;
        TransactionType type;
        double amount;
        double balanceAfter;
        string date;
        string description;

        static int nextTransactionId;

    public:
        Transaction();
        Transaction(int transId,
                    int accId,
                    const string &contractNum,
                    TransactionType transType,
                    double transAmount,
                    double newBalance,
                    const string &transDate,
                    const string &desc = "");

        int getId() const { return id; }
        int getAccountId() const { return accountId; }
        const string &getContractNumber() const { return contractNumber; }
        TransactionType getType() const { return type; }
        double getAmount() const { return amount; }
        double getBalanceAfter() const { return balanceAfter; }
        const string &getDate() const { return date; }
        const string &getDescription() const { return description; }

        void printInfo() const;
        void printTableRow() const;
        string toReportLine() const;

        void writeToBinary(ofstream &out) const;
        void readFromBinary(ifstream &in);

        static int generateNextId();
        static void syncNextId(int loadedId);
        static string typeToString(TransactionType type);
        static string getCurrentDate();

        bool operator==(const Transaction &other) const;
        bool operator<(const Transaction &other) const;
        friend ostream &operator<<(ostream &os, const Transaction &tr);
    };
}
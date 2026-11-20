#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <exception>
#include <cstring>
#include <algorithm>
#include <functional>
#include <iterator>

using namespace std;

namespace BankSystem
{
    class BankException : public exception
    {
    protected:
        string message;

    public:
        explicit BankException(const string &msg) : message(msg) {}

        virtual const char *what() const noexcept override
        {
            return message.c_str();
        }
    };

    class DepositException : public BankException
    {
    public:
        explicit DepositException(const string &msg) : BankException("Ошибка вклада: " + msg) {}
    };

    class AuthException : public BankException
    {
    public:
        explicit AuthException(const string &msg) : BankException("Ошибка доступа: " + msg) {}
    };

    class Account
    {
    protected:
        int id;
        string clientName;
        double balance;
        string contractNumber;
        static int totalAccounts;

    public:
        Account(int accountId, const string &name, double initialBalance);
        virtual ~Account() = default;

        virtual void printReport() const = 0;

        virtual void depositFunds(double amount);
        virtual void withdrawFunds(double amount);

        static int getTotalAccounts();
        static string generateContractNumber(int id);

        int getId() const { return id; }
        const string &getClientName() const { return clientName; }
        double getBalance() const { return balance; }
        const string &getContractNumber() const { return contractNumber; }
        void setClientName(const string &name) { clientName = name; }
    };

    class Deposit : public Account
    {
    protected:
        double interestRate;

    public:
        Deposit(int accountId, const string &name, double initialBalance, double rate);
        virtual ~Deposit() = default;

        virtual void calculateInterest() = 0;

        double getInterestRate() const { return interestRate; }
        void setInterestRate(double rate) { interestRate = rate; }

        friend ostream &operator<<(ostream &os, const Deposit &d);
    };

    class TermDeposit : public Deposit
    {
    private:
        int termMonths;
        char *specialNotes;

    public:
        TermDeposit(int accountId, const string &name, double initialBalance, double rate, int months, const char *notes);

        TermDeposit(const TermDeposit &other);
        TermDeposit &operator=(const TermDeposit &other);
        ~TermDeposit() override;

        int getTermMonths() const { return termMonths; }
        string getSpecialNotes() const { return specialNotes ? string(specialNotes) : ""; }

        void printReport() const override;
        void calculateInterest() override;
        void withdrawFunds(double amount) override;
    };

    class User
    {
    protected:
        string login;
        string encryptedPassword;

    public:
        User(const string &l, const string &p);
        virtual ~User() = default;

        virtual void showMenu() const = 0;

        const string &getLogin() const { return login; }
        const string &getEncryptedPassword() const { return encryptedPassword; }
        void setEncryptedPassword(const string &encPass) { encryptedPassword = encPass; }

        static string encryptPassword(const string &pass);

        friend bool authenticate(const User &user, const string &inputPass);
    };

    class Admin : public User
    {
    public:
        Admin(const string &l, const string &p);
        void showMenu() const override;
    };

    class Operator : public User
    {
    public:
        Operator(const string &l, const string &p);
        void showMenu() const override;
    };

    template <typename T>
    class DataManager
    {
    private:
        vector<shared_ptr<T>> records;

    public:
        void addRecord(const shared_ptr<T> &record)
        {
            if (!record)
            {
                throw BankException("Попытка добавить пустую запись");
            }
            records.push_back(record);
        }

        void removeRecord(int id)
        {
            for (auto it = records.begin(); it != records.end(); ++it)
            {
                if ((*it)->getId() == id)
                {
                    records.erase(it);
                    return;
                }
            }
            throw BankException("Запись с таким ID не найдена");
        }

        void clear()
        {
            records.clear();
        }

        size_t size() const
        {
            return records.size();
        }

        const vector<shared_ptr<T>> &getAllRecords() const
        {
            return records;
        }

        shared_ptr<T> findByCondition(function<bool(const shared_ptr<T> &)> condition) const
        {
            auto it = find_if(records.begin(), records.end(), condition);
            if (it != records.end())
            {
                return *it;
            }
            return nullptr;
        }

        vector<shared_ptr<T>> filterRecords(function<bool(const shared_ptr<T> &)> condition) const
        {
            vector<shared_ptr<T>> filtered;
            copy_if(records.begin(), records.end(), back_inserter(filtered), condition);
            return filtered;
        }

        void sortRecords(function<bool(const shared_ptr<T> &, const shared_ptr<T> &)> comparator)
        {
            sort(records.begin(), records.end(), comparator);
        }
    };
}
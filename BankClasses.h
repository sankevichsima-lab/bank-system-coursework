#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <exception>
#include <cstring>
#include <algorithm>
#include <functional>

using namespace std;
{

} // namespace std;

// Собственное пространство имен системы
namespace BankSystem
{
    // Базовый класс для всех ошибок приложения
    class BankException : public exception
    {
    protected:
        string message; // Текст ошибки

    public:
        explicit BankException(const std::string &msg) : message(msg) {}

        virtual const char *what() const noexcept override
        {
            return message.c_str();
        }
    };

    // Класс ошибок, связанных с операциями по вкладам
    class DepositException : public BankException
    {
    public:
        explicit DepositException(const string &msg) : BankException("Ошибка вклада: " + msg) {}
    };

    // Абстрактный базовый класс счета
    class Account
    {
    protected:
        int id;                   // Уникальный номер счета
        string clientName;        // ФИО владельца
        double balance;           // Текущий баланс
        static int totalAccounts; // Счетчик всех созданных счетов
        string contractNumber;    // Уникальный номер договора

    public:
        Account(int accountId, const string &name, double initialBalance);
        virtual ~Account() = default;

        virtual void printReport() const = 0;

        static int getTotalAccounts();
        int getId() const { return id; }

        virtual void depositFunds(double amount);
        virtual void withdrawFunds(double amount);

        static string generateContractNumber(int id);

        string getContractNumber() const { return contractNumber; }
    };

    // Базовый класс банковского вклада
    class Deposit : public Account
    {
    protected:
        double interestRate; // Процентная ставка

    public:
        Deposit(int accountId, const string &name, double initialBalance, double rate);
        virtual ~Deposit() = default;

        virtual void calculateInterest() = 0;

        friend ostream &operator<<(ostream &os, const Deposit &d);
    };

    // Класс срочного вклада (с фиксированным сроком)
    class TermDeposit : public Deposit
    {
    private:
        int termMonths;     // Срок вклада в месяцах
        char *specialNotes; // Динамическая строка: особые условия/примечания к договору

    public:
        TermDeposit(int accountId, const string &name, double initialBalance, double rate, int months, const char *notes);

        TermDeposit(const TermDeposit &other);
        TermDeposit &operator=(const TermDeposit &other);
        ~TermDeposit() override;

        void printReport() const override;
        void calculateInterest() override;
        void withdrawFunds(double amount) override;
    };

    // Шаблонный класс для управления коллекциями любых объектов
    template <typename T>
    class DataManager
    {
    private:
        vector<shared_ptr<T>> records;

    public:
        void addRecord(const shared_ptr<T> &record)
        {
            if (!record)
                throw BankException("Попытка добавить пустую запись");
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

        const vector<shared_ptr<T>> &getAllRecords() const
        {
            return records;
        }

        // 1. Поиск записи с использованием алгоритма STL
        shared_ptr<T> findByCondition(function<bool(const shared_ptr<T> &)> condition)
        {
            auto it = find_if(records.begin(), records.end(), condition);
            if (it != records.end())
            {
                return *it;
            }
            return nullptr; // Если ничего не найдено
        }

        // 2. Фильтрация записей
        vector<shared_ptr<T>> filterRecords(function<bool(const shared_ptr<T> &)> condition) const
        {
            vector<shared_ptr<T>> filtered;
            copy_if(records.begin(), records.end(), back_inserter(filtered), condition);
            return filtered;
        }

        // 3. Сортировка записей (std::sort)
        void sortRecords(function<bool(const shared_ptr<T> &, const shared_ptr<T> &)> comparator)
        {
            sort(records.begin(), records.end(), comparator);
        }
    };

    // Абстрактный базовый класс пользователя
    class User
    {
    protected:
        string login;
        string encryptedPassword; // Хранение пароля в зашифрованном виде

    public:
        User(const string &l, const string &p);
        virtual ~User() = default;

        // Чисто виртуальная функция (полиморфизм) для вывода меню
        virtual void showMenu() const = 0;

        // Статический метод шифрования
        static string encryptPassword(const std::string &pass);

        string getLogin() const { return login; }

        // Дружественная функция для проверки прав
        friend bool authenticate(const User &user, const string &inputPass);
    };

    // Класс Администратора
    class Admin : public User
    {
    public:
        Admin(const string &l, const string &p);
        void showMenu() const override;
    };

    // Класс Оператора
    class Operator : public User
    {
    public:
        Operator(const string &l, const string &p);
        void showMenu() const override;
    };
}
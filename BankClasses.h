#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <exception>
#include <cstring>
#include <algorithm>
#include <functional>

// Собственное пространство имен системы
namespace BankSystem
{
    // Базовый класс для всех ошибок приложения
    class BankException : public std::exception
    {
    protected:
        std::string message; // Текст ошибки

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
        explicit DepositException(const std::string &msg) : BankException("Ошибка вклада: " + msg) {}
    };

    // Абстрактный базовый класс счета
    class Account
    {
    protected:
        int id;                     // Уникальный номер счета
        std::string clientName;     // ФИО владельца
        double balance;             // Текущий баланс
        static int totalAccounts;   // Счетчик всех созданных счетов
        std::string contractNumber; // Уникальный номер договора

    public:
        Account(int accountId, const std::string &name, double initialBalance);
        virtual ~Account() = default;

        virtual void printReport() const = 0;

        static int getTotalAccounts();
        int getId() const { return id; }

        virtual void depositFunds(double amount);
        virtual void withdrawFunds(double amount);

        static std::string generateContractNumber(int id);

        std::string getContractNumber() const { return contractNumber; }
    };

    // Базовый класс банковского вклада
    class Deposit : public Account
    {
    protected:
        double interestRate; // Процентная ставка

    public:
        Deposit(int accountId, const std::string &name, double initialBalance, double rate);
        virtual ~Deposit() = default;

        virtual void calculateInterest() = 0;

        friend std::ostream &operator<<(std::ostream &os, const Deposit &d);
    };

    // Класс срочного вклада (с фиксированным сроком)
    class TermDeposit : public Deposit
    {
    private:
        int termMonths;     // Срок вклада в месяцах
        char *specialNotes; // Динамическая строка: особые условия/примечания к договору

    public:
        TermDeposit(int accountId, const std::string &name, double initialBalance, double rate, int months, const char *notes);

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
        std::vector<std::shared_ptr<T>> records;

    public:
        void addRecord(const std::shared_ptr<T> &record)
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

        const std::vector<std::shared_ptr<T>> &getAllRecords() const
        {
            return records;
        }

        // 1. Поиск записи с использованием алгоритма STL
        std::shared_ptr<T> findByCondition(std::function<bool(const std::shared_ptr<T> &)> condition)
        {
            auto it = std::find_if(records.begin(), records.end(), condition);
            if (it != records.end())
            {
                return *it;
            }
            return nullptr; // Если ничего не найдено
        }

        // 2. Фильтрация записей
        std::vector<std::shared_ptr<T>> filterRecords(std::function<bool(const std::shared_ptr<T> &)> condition) const
        {
            std::vector<std::shared_ptr<T>> filtered;
            std::copy_if(records.begin(), records.end(), std::back_inserter(filtered), condition);
            return filtered;
        }

        // 3. Сортировка записей (std::sort)
        void sortRecords(std::function<bool(const std::shared_ptr<T> &, const std::shared_ptr<T> &)> comparator)
        {
            std::sort(records.begin(), records.end(), comparator);
        }
    };

    // Абстрактный базовый класс пользователя
    class User
    {
    protected:
        std::string login;
        std::string encryptedPassword; // Хранение пароля в зашифрованном виде

    public:
        User(const std::string &l, const std::string &p);
        virtual ~User() = default;

        // Чисто виртуальная функция (полиморфизм) для вывода меню
        virtual void showMenu() const = 0;

        // Статический метод шифрования
        static std::string encryptPassword(const std::string &pass);

        std::string getLogin() const { return login; }

        // Дружественная функция для проверки прав
        friend bool authenticate(const User &user, const std::string &inputPass);
    };

    // Класс Администратора
    class Admin : public User
    {
    public:
        Admin(const std::string &l, const std::string &p);
        void showMenu() const override;
    };

    // Класс Оператора
    class Operator : public User
    {
    public:
        Operator(const std::string &l, const std::string &p);
        void showMenu() const override;
    };
}
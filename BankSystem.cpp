#include "BankClasses.h"
#include <iostream>
#include <cstring>
#include <iomanip>
#include <sstream>

namespace BankSystem
{
    // Реализация класса Account
    int Account::totalAccounts = 0;

    Account::Account(int accountId, const std::string &name, double initialBalance)
        : id(accountId), clientName(name)
    {

        if (initialBalance < 0)
        {
            throw BankException("Ошибка валидации: Начальный баланс не может быть отрицательным");
        }

        balance = initialBalance;
        totalAccounts++;
        contractNumber = generateContractNumber(id);
    }

    void Account::depositFunds(double amount)
    {
        if (amount <= 0)
        {
            throw DepositException("Сумма пополнения должна быть больше нуля");
        }
        balance += amount;
        std::cout << "Счет " << contractNumber << " пополнен на " << amount << " BYN. Текущий баланс: " << balance << " BYN\n";
    }

    void Account::withdrawFunds(double amount)
    {
        if (amount <= 0)
        {
            throw DepositException("Сумма снятия должна быть больше нуля");
        }
        if (amount > balance)
        {
            throw DepositException("Недостаточно средств на счете для проведения операции");
        }
        balance -= amount;
        std::cout << "Со счета " << contractNumber << " снято " << amount << " BYN. Текущий баланс: " << balance << " BYN\n";
    }

    int Account::getTotalAccounts()
    {
        return totalAccounts;
    }

    // Реализация класса Deposit
    Deposit::Deposit(int accountId, const std::string &name, double initialBalance, double rate)
        : Account(accountId, name, initialBalance), interestRate(rate) {}

    std::ostream &operator<<(std::ostream &os, const Deposit &d)
    {
        os << "ID: " << d.id << " | Клиент: " << d.clientName
           << " | Баланс: " << d.balance << " BYN | Ставка: " << d.interestRate << "%";
        return os;
    }

    // Реализация класса TermDeposit
    TermDeposit::TermDeposit(int accountId, const std::string &name, double initialBalance, double rate, int months, const char *notes)
        : Deposit(accountId, name, initialBalance, rate), termMonths(months)
    {
        if (notes)
        {
            specialNotes = new char[std::strlen(notes) + 1];
            std::strcpy(specialNotes, notes);
        }
        else
        {
            specialNotes = new char[1];
            specialNotes[0] = '\0';
        }
    }

    void TermDeposit::withdrawFunds(double amount)
    {
        throw DepositException("Частичное снятие со срочного вклада (договор " + contractNumber + ") запрещено условиями банка!");
    }

    // Конструктор копирования
    TermDeposit::TermDeposit(const TermDeposit &other)
        : Deposit(other.id, other.clientName, other.balance, other.interestRate), termMonths(other.termMonths)
    {
        specialNotes = new char[std::strlen(other.specialNotes) + 1];
        std::strcpy(specialNotes, other.specialNotes);
    }

    // Перегрузка оператора присваивания
    TermDeposit &TermDeposit::operator=(const TermDeposit &other)
    {
        if (this != &other)
        { // Защита от самоприсваивания
            // Копируем базовые поля
            id = other.id;
            clientName = other.clientName;
            balance = other.balance;
            interestRate = other.interestRate;
            termMonths = other.termMonths;

            delete[] specialNotes;
            specialNotes = new char[std::strlen(other.specialNotes) + 1];
            std::strcpy(specialNotes, other.specialNotes);
        }
        return *this;
    }

    // Деструктор
    TermDeposit::~TermDeposit()
    {
        delete[] specialNotes;
    }

    // Переопределение виртуальных методов
    void TermDeposit::printReport() const
    {
        std::cout << "--- Отчет по срочному вкладу ---\n";
        std::cout << "Договор: " << contractNumber << "\n";
        std::cout << *this << "\n";
        std::cout << "Срок: " << termMonths << " мес. | Примечания: " << specialNotes << "\n";
        std::cout << "--------------------------------\n";
    }

    void TermDeposit::calculateInterest()
    {
        double earned = (balance * interestRate / 100) * (termMonths / 12.0);
        balance += earned;
        std::cout << "Проценты начислены. Новый баланс: " << balance << " BYN\n";
    }

    // Простой статический метод шифрования (сдвиг символов)
    std::string User::encryptPassword(const std::string &pass)
    {
        std::string encrypted = pass;
        for (char &c : encrypted)
        {
            c += 3;
        }
        return encrypted;
    }

    // Базовый конструктор пользователя с вызовом шифрования
    User::User(const std::string &l, const std::string &p)
        : login(l), encryptedPassword(encryptPassword(p)) {}

    // Реализация дружественной функции авторизации
    bool authenticate(const User &user, const std::string &inputPass)
    {
        return user.encryptedPassword == User::encryptPassword(inputPass);
    }

    // Конструкторы наследников
    Admin::Admin(const std::string &l, const std::string &p) : User(l, p) {}
    Operator::Operator(const std::string &l, const std::string &p) : User(l, p) {}

    // Переопределение виртуальных функций
    void Admin::showMenu() const
    {
        std::cout << "\n=== МЕНЮ АДМИНИСТРАТОРА ===\n";
        std::cout << "1. Просмотр всех вкладов\n";
        std::cout << "2. Добавление/Редактирование/Удаление записей\n";
        std::cout << "3. Управление пользователями системы\n";
        std::cout << "4. Выход\n";
        std::cout << "===========================\n";
    }

    void Operator::showMenu() const
    {
        std::cout << "\n=== МЕНЮ ОПЕРАТОРА ===\n";
        std::cout << "1. Оформление нового вклада\n";
        std::cout << "2. Поиск и фильтрация вкладов\n";
        std::cout << "3. Формирование отчета\n";
        std::cout << "4. Выход\n";
        std::cout << "======================\n";
    }

    std::string Account::generateContractNumber(int id)
    {
        std::ostringstream oss;
        oss << "DEP-" << std::setfill('0') << std::setw(6) << id;
        return oss.str();
    }
}
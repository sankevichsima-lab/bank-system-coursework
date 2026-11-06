#include "BankClasses.h"
#include <iostream>
#include <cstring>
#include <iomanip>
#include <sstream>

using namespace std;

namespace BankSystem
{
    // Реализация класса Account
    int Account::totalAccounts = 0;

    Account::Account(int accountId, const string &name, double initialBalance)
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
        cout << "Счет " << contractNumber << " пополнен на " << amount << " BYN. Текущий баланс: " << balance << " BYN\n";
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
        cout << "Со счета " << contractNumber << " снято " << amount << " BYN. Текущий баланс: " << balance << " BYN\n";
    }

    int Account::getTotalAccounts()
    {
        return totalAccounts;
    }

    // Реализация класса Deposit
    Deposit::Deposit(int accountId, const std::string &name, double initialBalance, double rate)
        : Account(accountId, name, initialBalance), interestRate(rate) {}

    ostream &operator<<(ostream &os, const Deposit &d)
    {
        os << "ID: " << d.id << " | Клиент: " << d.clientName
           << " | Баланс: " << d.balance << " BYN | Ставка: " << d.interestRate << "%";
        return os;
    }

    // Реализация класса TermDeposit
    TermDeposit::TermDeposit(int accountId, const string &name, double initialBalance, double rate, int months, const char *notes)
        : Deposit(accountId, name, initialBalance, rate), termMonths(months)
    {
        if (notes)
        {
            specialNotes = new char[strlen(notes) + 1];
            strcpy(specialNotes, notes);
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
        specialNotes = new char[strlen(other.specialNotes) + 1];
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
            specialNotes = new char[strlen(other.specialNotes) + 1];
            strcpy(specialNotes, other.specialNotes);
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
        cout << "--- Отчет по срочному вкладу ---\n";
        cout << "Договор: " << contractNumber << "\n";
        cout << *this << "\n";
        cout << "Срок: " << termMonths << " мес. | Примечания: " << specialNotes << "\n";
        cout << "--------------------------------\n";
    }

    void TermDeposit::calculateInterest()
    {
        double earned = (balance * interestRate / 100) * (termMonths / 12.0);
        balance += earned;
        cout << "Проценты начислены. Новый баланс: " << balance << " BYN\n";
    }

    // Простой статический метод шифрования (сдвиг символов)
    string User::encryptPassword(const string &pass)
    {
        string encrypted = pass;
        for (char &c : encrypted)
        {
            c += 3;
        }
        return encrypted;
    }

    // Базовый конструктор пользователя с вызовом шифрования
    User::User(const string &l, const string &p)
        : login(l), encryptedPassword(encryptPassword(p)) {}

    // Реализация дружественной функции авторизации
    bool authenticate(const User &user, const string &inputPass)
    {
        return user.encryptedPassword == User::encryptPassword(inputPass);
    }

    // Конструкторы наследников
    Admin::Admin(const std::string &l, const string &p) : User(l, p) {}
    Operator::Operator(const std::string &l, const string &p) : User(l, p) {}

    // Переопределение виртуальных функций
    void Admin::showMenu() const
    {
        cout << "\n=== МЕНЮ АДМИНИСТРАТОРА ===\n";
        cout << "1. Просмотр всех вкладов\n";
        cout << "2. Добавление/Редактирование/Удаление записей\n";
        cout << "3. Управление пользователями системы\n";
        cout << "4. Выход\n";
        cout << "===========================\n";
    }

    void Operator::showMenu() const
    {
        cout << "\n=== МЕНЮ ОПЕРАТОРА ===\n";
        cout << "1. Оформление нового вклада\n";
        cout << "2. Поиск и фильтрация вкладов\n";
        cout << "3. Формирование отчета\n";
        cout << "4. Выход\n";
        cout << "======================\n";
    }

    string Account::generateContractNumber(int id)
    {
        ostringstream oss;
        oss << "DEP-" << setfill('0') << setw(6) << id;
        return oss.str();
    }
}
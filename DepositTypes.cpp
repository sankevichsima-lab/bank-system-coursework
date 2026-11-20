#include "DepositTypes.h"
#include <iomanip>
#include <cmath>
#include <limits>

using namespace std;

namespace BankSystem
{

    DemandDeposit::DemandDeposit(int accountId,
                                 const string &name,
                                 double initialBalance,
                                 double rate,
                                 const string &date)
        : Deposit(accountId, name, initialBalance, rate),
          openDate(date.empty() ? "01.01.2026" : date),
          isActive(true)
    {
        if (rate < 0.0 || rate > 100.0)
        {
            throw DepositException("Процентная ставка должна быть в диапазоне от 0 до 100%");
        }
    }

    void DemandDeposit::printReport() const
    {
        cout << "--- Отчет по вкладу до востребования ---\n";
        cout << "Договор:       " << contractNumber << "\n";
        cout << *this << "\n";
        cout << "Дата открытия: " << openDate
             << " | Статус: " << (isActive ? "Активен" : "Закрыт") << "\n";
        cout << "----------------------------------------\n";
    }

    void DemandDeposit::calculateInterest()
    {
        if (!isActive)
        {
            throw DepositException("Невозможно начислить проценты: счет " + contractNumber + " закрыт");
        }
        double earned = (balance * interestRate / 100.0) / 12.0;
        balance += earned;
        cout << fixed << setprecision(2);
        cout << "По вкладу до востребования " << contractNumber
             << " начислено " << earned << " BYN. Новый баланс: " << balance << " BYN\n";
    }

    void DemandDeposit::withdrawFunds(double amount)
    {
        if (!isActive)
        {
            throw DepositException("Снятие невозможно: счет " + contractNumber + " закрыт");
        }
        if (amount <= 0.0)
        {
            throw DepositException("Сумма снятия должна быть больше нуля");
        }
        if (amount > balance)
        {
            throw DepositException("Недостаточно средств на счете " + contractNumber);
        }
        balance -= amount;
        cout << fixed << setprecision(2);
        cout << "Со счета до востребования " << contractNumber
             << " снято " << amount << " BYN. Остаток: " << balance << " BYN\n";
    }

    SavingsDeposit::SavingsDeposit(int accountId,
                                   const string &name,
                                   double initialBalance,
                                   double rate,
                                   double minBal,
                                   int months,
                                   bool prolong)
        : Deposit(accountId, name, initialBalance, rate),
          minBalance(minBal),
          capitalizationMonths(months),
          autoProlongation(prolong)
    {
        if (rate < 0.0 || rate > 100.0)
        {
            throw DepositException("Процентная ставка должна быть в диапазоне от 0 до 100%");
        }
        if (minBal < 0.0)
        {
            throw DepositException("Неснижаемый остаток не может быть отрицательным");
        }
        if (initialBalance < minBal)
        {
            throw DepositException("Начальный баланс не может быть меньше неснижаемого остатка");
        }
        if (months <= 0)
        {
            throw DepositException("Период капитализации должен быть больше 0 месяцев");
        }
    }

    void SavingsDeposit::printReport() const
    {
        cout << "--- Отчет по накопительному вкладу ---\n";
        cout << "Договор:             " << contractNumber << "\n";
        cout << *this << "\n";
        cout << fixed << setprecision(2);
        cout << "Неснижаемый остаток: " << minBalance << " BYN\n";
        cout << "Период капитализации:" << " " << capitalizationMonths << " мес.\n";
        cout << "Автопролонгация:     " << (autoProlongation ? "Включена" : "Отключена") << "\n";
        cout << "--------------------------------------\n";
    }

    void SavingsDeposit::calculateInterest()
    {
        double monthlyRate = (interestRate / 100.0) / 12.0;
        double newBalance = balance * pow(1.0 + monthlyRate, capitalizationMonths);
        double earned = newBalance - balance;
        balance = newBalance;

        cout << fixed << setprecision(2);
        cout << "Выполнена капитализация по вкладу " << contractNumber
             << " за " << capitalizationMonths << " мес. Доход: " << earned
             << " BYN. Новый баланс: " << balance << " BYN\n";
    }

    void SavingsDeposit::withdrawFunds(double amount)
    {
        if (amount <= 0.0)
        {
            throw DepositException("Сумма снятия должна быть больше нуля");
        }
        if (balance - amount < minBalance)
        {
            throw DepositException("Операция отклонена: остаток на накопительном вкладе " +
                                   contractNumber + " не может быть меньше неснижаемого остатка");
        }
        balance -= amount;
        cout << fixed << setprecision(2);
        cout << "С накопительного вклада " << contractNumber
             << " снято " << amount << " BYN. Остаток: " << balance << " BYN\n";
    }

    shared_ptr<Deposit> DepositFactory::createTermDeposit(int accountId,
                                                          const string &clientName,
                                                          double initialBalance,
                                                          double rate,
                                                          int months,
                                                          const string &notes)
    {
        if (months <= 0)
        {
            throw DepositException("Срок срочного вклада должен быть больше 0 месяцев");
        }
        if (rate < 0.0 || rate > 100.0)
        {
            throw DepositException("Процентная ставка должна быть от 0 до 100%");
        }
        return make_shared<TermDeposit>(accountId, clientName, initialBalance, rate, months, notes.c_str());
    }

    shared_ptr<Deposit> DepositFactory::createDemandDeposit(int accountId,
                                                            const string &clientName,
                                                            double initialBalance,
                                                            double rate,
                                                            const string &openDate)
    {
        return make_shared<DemandDeposit>(accountId, clientName, initialBalance, rate, openDate);
    }

    shared_ptr<Deposit> DepositFactory::createSavingsDeposit(int accountId,
                                                             const string &clientName,
                                                             double initialBalance,
                                                             double rate,
                                                             double minBalance,
                                                             int months,
                                                             bool autoProlongation)
    {
        return make_shared<SavingsDeposit>(accountId, clientName, initialBalance, rate, minBalance, months, autoProlongation);
    }

    shared_ptr<Deposit> DepositFactory::createDepositInteractive(int accountId,
                                                                 const string &clientName)
    {
        int typeChoice = 0;
        double initialBalance = 0.0;
        double rate = 0.0;

        cout << "\nВыберите тип открываемого вклада:\n";
        cout << "1. Срочный вклад\n";
        cout << "2. Вклад до востребования\n";
        cout << "3. Накопительный вклад (с капитализацией)\n";
        cout << "Ваш выбор (1-3): ";

        if (!(cin >> typeChoice) || typeChoice < 1 || typeChoice > 3)
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            throw DepositException("Выбран некорректный тип вклада");
        }

        cout << "Введите начальную сумму вклада (BYN): ";
        if (!(cin >> initialBalance))
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            throw DepositException("Некорректный ввод суммы вклада");
        }

        cout << "Введите годовую процентную ставку (%): ";
        if (!(cin >> rate))
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            throw DepositException("Некорректный ввод процентной ставки");
        }

        if (typeChoice == static_cast<int>(DepositKind::TERM))
        {
            int months = 0;
            string notes;

            cout << "Введите срок вклада (в месяцах): ";
            if (!(cin >> months))
            {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                throw DepositException("Некорректный ввод срока вклада");
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            cout << "Введите особые условия / примечание: ";
            getline(cin, notes);

            return createTermDeposit(accountId, clientName, initialBalance, rate, months, notes);
        }
        else if (typeChoice == static_cast<int>(DepositKind::DEMAND))
        {
            string openDate;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            cout << "Введите дату открытия (ДД.ММ.ГГГГ): ";
            getline(cin, openDate);

            return createDemandDeposit(accountId, clientName, initialBalance, rate, openDate);
        }
        else
        {
            double minBalance = 0.0;
            int months = 0;
            int prolongChoice = 0;

            cout << "Введите неснижаемый остаток (BYN): ";
            if (!(cin >> minBalance))
            {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                throw DepositException("Некорректный ввод неснижаемого остатка");
            }

            cout << "Введите период капитализации (в месяцах): ";
            if (!(cin >> months))
            {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                throw DepositException("Некорректный ввод периода капитализации");
            }

            cout << "Автопролонгация (1 - да, 0 - нет): ";
            if (!(cin >> prolongChoice))
            {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                throw DepositException("Некорректный ввод параметра автопролонгации");
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            return createSavingsDeposit(accountId, clientName, initialBalance, rate,
                                        minBalance, months, prolongChoice == 1);
        }
    }

    string DepositFactory::kindToString(DepositKind kind)
    {
        switch (kind)
        {
        case DepositKind::TERM:
            return "Срочный вклад";
        case DepositKind::DEMAND:
            return "Вклад до востребования";
        case DepositKind::SAVINGS:
            return "Накопительный вклад";
        default:
            return "Неизвестный тип";
        }
    }
}
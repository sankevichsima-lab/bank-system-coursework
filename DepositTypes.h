#pragma once
#include "BankClasses.h"
#include <iostream>
#include <string>
#include <memory>

using namespace std;

namespace BankSystem
{
    enum class DepositKind
    {
        TERM = 1,
        DEMAND = 2,
        SAVINGS = 3
    };

    class DemandDeposit : public Deposit
    {
    private:
        string openDate;
        bool isActive;

    public:
        DemandDeposit(int accountId,
                      const string &name,
                      double initialBalance,
                      double rate,
                      const string &date = "01.01.2026");

        ~DemandDeposit() override = default;

        const string &getOpenDate() const { return openDate; }
        bool getIsActive() const { return isActive; }
        void setActive(bool status) { isActive = status; }

        void printReport() const override;
        void calculateInterest() override;
        void withdrawFunds(double amount) override;
    };

    class SavingsDeposit : public Deposit
    {
    private:
        double minBalance;
        int capitalizationMonths;
        bool autoProlongation;

    public:
        SavingsDeposit(int accountId,
                       const string &name,
                       double initialBalance,
                       double rate,
                       double minBal,
                       int months,
                       bool prolong = true);

        ~SavingsDeposit() override = default;

        double getMinBalance() const { return minBalance; }
        int getCapitalizationMonths() const { return capitalizationMonths; }
        bool getAutoProlongation() const { return autoProlongation; }
        void setAutoProlongation(bool prolong) { autoProlongation = prolong; }

        void printReport() const override;
        void calculateInterest() override;
        void withdrawFunds(double amount) override;
    };

    class DepositFactory
    {
    public:
        static shared_ptr<Deposit> createTermDeposit(int accountId,
                                                     const string &clientName,
                                                     double initialBalance,
                                                     double rate,
                                                     int months,
                                                     const string &notes);

        static shared_ptr<Deposit> createDemandDeposit(int accountId,
                                                       const string &clientName,
                                                       double initialBalance,
                                                       double rate,
                                                       const string &openDate);

        static shared_ptr<Deposit> createSavingsDeposit(int accountId,
                                                        const string &clientName,
                                                        double initialBalance,
                                                        double rate,
                                                        double minBalance,
                                                        int months,
                                                        bool autoProlongation);

        static shared_ptr<Deposit> createDepositInteractive(int accountId,
                                                            const string &clientName);

        static string kindToString(DepositKind kind);
    };
}
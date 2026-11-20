#pragma once
#include "BankClasses.h"
#include "Client.h"
#include "Transaction.h"
#include "DepositTypes.h"
#include "FileStorage.h"
#include <iostream>
#include <string>
#include <vector>
#include <memory>

using namespace std;

namespace BankSystem
{
    class BankApp
    {
    private:
        DataManager<Client> clientManager;
        DataManager<Deposit> depositManager;
        DataManager<Transaction> transactionManager;
        DataManager<User> userManager;

        shared_ptr<User> currentUser;
        int nextDepositId;

        void loadSystemData();
        void saveSystemData() const;
        void initDefaultDataIfEmpty();
        void syncNextDepositId();

        int readInt(const string &prompt, int minVal, int maxVal) const;
        double readDouble(const string &prompt, double minVal) const;
        string readLine(const string &prompt) const;
        void pauseConsole() const;

        bool loginScreen();
        void runAdminSession();
        void runOperatorSession();

        void manageClientsMenu();
        void viewAllClients() const;
        void addClient();
        void editClient();
        void deleteClient();
        void searchClients() const;
        void sortClients();

        void manageDepositsMenu(bool isAdmin);
        void viewAllDeposits() const;
        void openNewDeposit();
        void editDeposit();
        void closeOrDeleteDeposit();

        void depositOperationsMenu();
        void topUpDeposit();
        void withdrawFromDeposit();
        void accrueInterestSingle();
        void accrueInterestAll();
        void viewTransactions() const;

        void searchAndFilterDepositsMenu();
        void searchDepositByContract() const;
        void searchDepositsByClientName() const;
        void filterDepositsByMinBalance() const;
        void filterDepositsByType() const;
        void sortDeposits();

        void reportsMenu() const;
        void showPortfolioAnalytics() const;
        void exportBankReportToFile() const;
        void exportClientStatementToFile() const;

        void manageUsersMenu();
        void viewAllUsers() const;
        void addUser();
        void deleteUser();

        void logTransaction(const shared_ptr<Deposit> &dep,
                            TransactionType type,
                            double amount,
                            const string &description);

    public:
        BankApp();
        ~BankApp() = default;

        void run();
    };
}
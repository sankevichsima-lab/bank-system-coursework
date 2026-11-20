#pragma once
#include "BankClasses.h"
#include "Client.h"
#include "Transaction.h"
#include "DepositTypes.h"
#include <string>
#include <vector>
#include <memory>
#include <fstream>

using namespace std;

namespace BankSystem
{
    class FileException : public BankException
    {
    public:
        explicit FileException(const string &msg)
            : BankException("Ошибка работы с файлом: " + msg) {}
    };

    class FileStorage
    {
    private:
        string clientsFile;
        string depositsFile;
        string transactionsFile;
        string usersFile;

        FileStorage();

        static void writeString(ofstream &out, const string &str);
        static void readString(ifstream &in, string &str);

    public:
        FileStorage(const FileStorage &) = delete;
        FileStorage &operator=(const FileStorage &) = delete;

        static FileStorage &getInstance();

        void setFilePaths(const string &clientsPath,
                          const string &depositsPath,
                          const string &transactionsPath,
                          const string &usersPath);

        void saveClients(const DataManager<Client> &manager) const;
        void loadClients(DataManager<Client> &manager) const;

        void saveDeposits(const DataManager<Deposit> &manager) const;
        void loadDeposits(DataManager<Deposit> &manager) const;

        void saveTransactions(const DataManager<Transaction> &manager) const;
        void loadTransactions(DataManager<Transaction> &manager) const;

        void saveUsers(const DataManager<User> &manager) const;
        void loadUsers(DataManager<User> &manager) const;

        void saveAll(const DataManager<Client> &clients,
                     const DataManager<Deposit> &deposits,
                     const DataManager<Transaction> &transactions,
                     const DataManager<User> &users) const;

        void loadAll(DataManager<Client> &clients,
                     DataManager<Deposit> &deposits,
                     DataManager<Transaction> &transactions,
                     DataManager<User> &users) const;

        void exportFullReport(const string &filename,
                              const DataManager<Client> &clients,
                              const DataManager<Deposit> &deposits,
                              const DataManager<Transaction> &transactions) const;

        void exportClientStatement(const string &filename,
                                   const Client &client,
                                   const vector<shared_ptr<Deposit>> &clientDeposits,
                                   const vector<shared_ptr<Transaction>> &clientTransactions) const;
    };
}
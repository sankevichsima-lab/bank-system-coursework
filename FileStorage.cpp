#include "FileStorage.h"
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;

namespace BankSystem
{
    namespace
    {
        string formatUtf8(const string &str, size_t width)
        {
            string result;
            size_t visibleLen = 0;
            for (size_t i = 0; i < str.size();)
            {
                unsigned char c = static_cast<unsigned char>(str[i]);
                size_t charLen = 1;
                if ((c & 0x80) == 0)
                    charLen = 1;
                else if ((c & 0xE0) == 0xC0)
                    charLen = 2;
                else if ((c & 0xF0) == 0xE0)
                    charLen = 3;
                else if ((c & 0xF8) == 0xF0)
                    charLen = 4;

                if (visibleLen + 1 > width)
                    break;

                if (i + charLen <= str.size())
                    result.append(str, i, charLen);
                i += charLen;
                visibleLen++;
            }
            if (visibleLen < width)
            {
                result.append(width - visibleLen, ' ');
            }
            return result;
        }

        string cleanField(string s)
        {
            for (char &c : s)
            {
                if (c == ';' || c == '\n' || c == '\r')
                {
                    c = ' ';
                }
            }
            return s;
        }

        vector<string> splitLine(const string &line, char delim = ';')
        {
            vector<string> parts;
            size_t start = 0;
            while (true)
            {
                size_t pos = line.find(delim, start);
                if (pos == string::npos)
                {
                    parts.push_back(line.substr(start));
                    break;
                }
                parts.push_back(line.substr(start, pos - start));
                start = pos + 1;
            }
            return parts;
        }
    }

    FileStorage::FileStorage()
        : clientsFile("clients.txt"),
          depositsFile("deposits.txt"),
          transactionsFile("transactions.txt"),
          usersFile("users.txt") {}

    FileStorage &FileStorage::getInstance()
    {
        static FileStorage instance;
        return instance;
    }

    void FileStorage::setFilePaths(const string &clientsPath,
                                   const string &depositsPath,
                                   const string &transactionsPath,
                                   const string &usersPath)
    {
        clientsFile = clientsPath;
        depositsFile = depositsPath;
        transactionsFile = transactionsPath;
        usersFile = usersPath;
    }

    void FileStorage::writeString(ofstream &out, const string &str)
    {
        out << cleanField(str);
    }

    void FileStorage::readString(ifstream &in, string &str)
    {
        getline(in, str);
    }

    void FileStorage::saveClients(const DataManager<Client> &manager) const
    {
        ofstream out(clientsFile, ios::trunc);
        if (!out.is_open())
        {
            throw FileException("Не удалось открыть файл для записи: " + clientsFile);
        }

        for (const auto &client : manager.getAllRecords())
        {
            if (!client)
                continue;
            out << client->getId() << ";"
                << cleanField(client->getFullName()) << ";"
                << cleanField(client->getPassportNumber()) << ";"
                << cleanField(client->getPhoneNumber()) << ";"
                << cleanField(client->getEmail()) << ";"
                << cleanField(client->getRegistrationDate()) << ";"
                << (client->getIsVip() ? 1 : 0) << "\n";
        }

        if (!out)
        {
            throw FileException("Ошибка записи данных клиентов в файл: " + clientsFile);
        }
        out.close();
    }

    void FileStorage::loadClients(DataManager<Client> &manager) const
    {
        ifstream in(clientsFile);
        if (!in.is_open())
        {
            return;
        }

        manager.clear();
        string line;
        int lineNum = 0;
        while (getline(in, line))
        {
            lineNum++;
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            if (line.empty())
                continue;

            auto parts = splitLine(line);
            if (parts.size() != 7)
            {
                throw FileException("Неверный формат строки " + to_string(lineNum) + " в файле " + clientsFile);
            }

            try
            {
                int id = stoi(parts[0]);
                bool isVip = (stoi(parts[6]) == 1);
                auto client = make_shared<Client>(id, parts[1], parts[2], parts[3], parts[4], parts[5], isVip);
                manager.addRecord(client);
            }
            catch (const exception &e)
            {
                throw FileException("Ошибка чтения строки " + to_string(lineNum) + " в " + clientsFile + ": " + e.what());
            }
        }
        in.close();
    }

    void FileStorage::saveDeposits(const DataManager<Deposit> &manager) const
    {
        ofstream out(depositsFile, ios::trunc);
        if (!out.is_open())
        {
            throw FileException("Не удалось открыть файл для записи: " + depositsFile);
        }

        out << fixed << setprecision(2);
        for (const auto &dep : manager.getAllRecords())
        {
            if (!dep)
                continue;

            if (auto term = dynamic_pointer_cast<TermDeposit>(dep))
            {
                out << static_cast<int>(DepositKind::TERM) << ";"
                    << dep->getId() << ";"
                    << cleanField(dep->getClientName()) << ";"
                    << dep->getBalance() << ";"
                    << dep->getInterestRate() << ";"
                    << term->getTermMonths() << ";"
                    << cleanField(term->getSpecialNotes()) << "\n";
            }
            else if (auto demand = dynamic_pointer_cast<DemandDeposit>(dep))
            {
                out << static_cast<int>(DepositKind::DEMAND) << ";"
                    << dep->getId() << ";"
                    << cleanField(dep->getClientName()) << ";"
                    << dep->getBalance() << ";"
                    << dep->getInterestRate() << ";"
                    << cleanField(demand->getOpenDate()) << ";"
                    << (demand->getIsActive() ? 1 : 0) << "\n";
            }
            else if (auto savings = dynamic_pointer_cast<SavingsDeposit>(dep))
            {
                out << static_cast<int>(DepositKind::SAVINGS) << ";"
                    << dep->getId() << ";"
                    << cleanField(dep->getClientName()) << ";"
                    << dep->getBalance() << ";"
                    << dep->getInterestRate() << ";"
                    << savings->getMinBalance() << ";"
                    << savings->getCapitalizationMonths() << ";"
                    << (savings->getAutoProlongation() ? 1 : 0) << "\n";
            }
            else
            {
                throw FileException("Неизвестный тип вклада при сохранении в файл");
            }
        }

        if (!out)
        {
            throw FileException("Ошибка записи данных вкладов в файл: " + depositsFile);
        }
        out.close();
    }

    void FileStorage::loadDeposits(DataManager<Deposit> &manager) const
    {
        ifstream in(depositsFile);
        if (!in.is_open())
        {
            return;
        }

        manager.clear();
        string line;
        int lineNum = 0;
        while (getline(in, line))
        {
            lineNum++;
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            if (line.empty())
                continue;

            auto parts = splitLine(line);
            if (parts.size() < 7)
            {
                throw FileException("Неверный формат строки " + to_string(lineNum) + " в файле " + depositsFile);
            }

            try
            {
                int kind = stoi(parts[0]);
                int id = stoi(parts[1]);
                string name = parts[2];
                double balance = stod(parts[3]);
                double rate = stod(parts[4]);

                if (kind == static_cast<int>(DepositKind::TERM) && parts.size() == 7)
                {
                    int months = stoi(parts[5]);
                    manager.addRecord(DepositFactory::createTermDeposit(id, name, balance, rate, months, parts[6]));
                }
                else if (kind == static_cast<int>(DepositKind::DEMAND) && parts.size() == 7)
                {
                    bool isActive = (stoi(parts[6]) == 1);
                    auto dep = DepositFactory::createDemandDeposit(id, name, balance, rate, parts[5]);
                    if (auto demand = dynamic_pointer_cast<DemandDeposit>(dep))
                    {
                        demand->setActive(isActive);
                    }
                    manager.addRecord(dep);
                }
                else if (kind == static_cast<int>(DepositKind::SAVINGS) && parts.size() == 8)
                {
                    double minBal = stod(parts[5]);
                    int months = stoi(parts[6]);
                    bool prolong = (stoi(parts[7]) == 1);
                    manager.addRecord(DepositFactory::createSavingsDeposit(id, name, balance, rate, minBal, months, prolong));
                }
                else
                {
                    throw FileException("Неизвестный тип или число полей в строке " + to_string(lineNum));
                }
            }
            catch (const exception &e)
            {
                throw FileException("Ошибка чтения строки " + to_string(lineNum) + " в " + depositsFile + ": " + e.what());
            }
        }
        in.close();
    }

    void FileStorage::saveTransactions(const DataManager<Transaction> &manager) const
    {
        ofstream out(transactionsFile, ios::trunc);
        if (!out.is_open())
        {
            throw FileException("Не удалось открыть файл для записи: " + transactionsFile);
        }

        out << fixed << setprecision(2);
        for (const auto &tr : manager.getAllRecords())
        {
            if (!tr)
                continue;
            out << tr->getId() << ";"
                << tr->getAccountId() << ";"
                << cleanField(tr->getContractNumber()) << ";"
                << static_cast<int>(tr->getType()) << ";"
                << tr->getAmount() << ";"
                << tr->getBalanceAfter() << ";"
                << cleanField(tr->getDate()) << ";"
                << cleanField(tr->getDescription()) << "\n";
        }

        if (!out)
        {
            throw FileException("Ошибка записи транзакций в файл: " + transactionsFile);
        }
        out.close();
    }

    void FileStorage::loadTransactions(DataManager<Transaction> &manager) const
    {
        ifstream in(transactionsFile);
        if (!in.is_open())
        {
            return;
        }

        manager.clear();
        string line;
        int lineNum = 0;
        while (getline(in, line))
        {
            lineNum++;
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            if (line.empty())
                continue;

            auto parts = splitLine(line);
            if (parts.size() != 8)
            {
                throw FileException("Неверный формат строки " + to_string(lineNum) + " в файле " + transactionsFile);
            }

            try
            {
                int id = stoi(parts[0]);
                int accId = stoi(parts[1]);
                int typeVal = stoi(parts[3]);
                if (typeVal < 1 || typeVal > 5)
                {
                    throw FileException("Некорректный тип транзакции");
                }
                double amount = stod(parts[4]);
                double balanceAfter = stod(parts[5]);

                auto tr = make_shared<Transaction>(id, accId, parts[2], static_cast<TransactionType>(typeVal),
                                                   amount, balanceAfter, parts[6], parts[7]);
                manager.addRecord(tr);
            }
            catch (const exception &e)
            {
                throw FileException("Ошибка чтения строки " + to_string(lineNum) + " в " + transactionsFile + ": " + e.what());
            }
        }
        in.close();
    }

    void FileStorage::saveUsers(const DataManager<User> &manager) const
    {
        ofstream out(usersFile, ios::trunc);
        if (!out.is_open())
        {
            throw FileException("Не удалось открыть файл для записи: " + usersFile);
        }

        for (const auto &user : manager.getAllRecords())
        {
            if (!user)
                continue;
            int role = dynamic_pointer_cast<Admin>(user) ? 1 : 2;
            out << role << ";"
                << cleanField(user->getLogin()) << ";"
                << user->getEncryptedPassword() << "\n";
        }

        if (!out)
        {
            throw FileException("Ошибка записи пользователей в файл: " + usersFile);
        }
        out.close();
    }

    void FileStorage::loadUsers(DataManager<User> &manager) const
    {
        ifstream in(usersFile);
        if (!in.is_open())
        {
            return;
        }

        manager.clear();
        string line;
        int lineNum = 0;
        while (getline(in, line))
        {
            lineNum++;
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            if (line.empty())
                continue;

            auto parts = splitLine(line);
            if (parts.size() != 3)
            {
                throw FileException("Неверный формат строки " + to_string(lineNum) + " в файле " + usersFile);
            }

            try
            {
                int role = stoi(parts[0]);
                shared_ptr<User> user;
                if (role == 1)
                {
                    user = make_shared<Admin>(parts[1], "");
                }
                else if (role == 2)
                {
                    user = make_shared<Operator>(parts[1], "");
                }
                else
                {
                    throw FileException("Некорректная роль пользователя");
                }
                user->setEncryptedPassword(parts[2]);
                manager.addRecord(user);
            }
            catch (const exception &e)
            {
                throw FileException("Ошибка чтения строки " + to_string(lineNum) + " в " + usersFile + ": " + e.what());
            }
        }
        in.close();
    }

    void FileStorage::saveAll(const DataManager<Client> &clients,
                              const DataManager<Deposit> &deposits,
                              const DataManager<Transaction> &transactions,
                              const DataManager<User> &users) const
    {
        saveClients(clients);
        saveDeposits(deposits);
        saveTransactions(transactions);
        saveUsers(users);
    }

    void FileStorage::loadAll(DataManager<Client> &clients,
                              DataManager<Deposit> &deposits,
                              DataManager<Transaction> &transactions,
                              DataManager<User> &users) const
    {
        loadClients(clients);
        loadDeposits(deposits);
        loadTransactions(transactions);
        loadUsers(users);
    }

    void FileStorage::exportFullReport(const string &filename,
                                       const DataManager<Client> &clients,
                                       const DataManager<Deposit> &deposits,
                                       const DataManager<Transaction> &transactions) const
    {
        ofstream out(filename, ios::trunc);
        if (!out.is_open())
        {
            throw FileException("Не удалось создать файл отчета: " + filename);
        }

        const auto &clientList = clients.getAllRecords();
        const auto &depositList = deposits.getAllRecords();
        const auto &transList = transactions.getAllRecords();

        double totalPortfolio = 0.0;
        int termCount = 0;
        int demandCount = 0;
        int savingsCount = 0;

        for (const auto &dep : depositList)
        {
            totalPortfolio += dep->getBalance();
            if (dynamic_pointer_cast<TermDeposit>(dep))
                termCount++;
            else if (dynamic_pointer_cast<DemandDeposit>(dep))
                demandCount++;
            else if (dynamic_pointer_cast<SavingsDeposit>(dep))
                savingsCount++;
        }

        out << "====================================================================\n";
        out << "         СВОДНЫЙ ОТЧЕТ ПО ДЕПОЗИТНОМУ ПОРТФЕЛЮ БАНКА\n";
        out << "         Дата формирования: " << Transaction::getCurrentDate() << "\n";
        out << "====================================================================\n\n";

        out << fixed << setprecision(2);
        out << "ОБЩАЯ СТАТИСТИКА:\n";
        out << "  Всего клиентов в базе:          " << clientList.size() << "\n";
        out << "  Всего открытых вкладов:         " << depositList.size() << "\n";
        out << "    - Срочных вкладов:            " << termCount << "\n";
        out << "    - До востребования:           " << demandCount << "\n";
        out << "    - Накопительных:              " << savingsCount << "\n";
        out << "  Суммарный объем вкладов:        " << totalPortfolio << " BYN\n";
        out << "  Всего операций в журнале:       " << transList.size() << "\n\n";

        out << "--------------------------------------------------------------------\n";
        out << "СПИСОК ДЕПОЗИТНЫХ ДОГОВОРОВ:\n";
        out << "--------------------------------------------------------------------\n";
        if (depositList.empty())
        {
            out << "  Вклады отсутствуют.\n";
        }
        else
        {
            for (const auto &dep : depositList)
            {
                string kindName = "Вклад";
                if (dynamic_pointer_cast<TermDeposit>(dep))
                    kindName = "Срочный";
                else if (dynamic_pointer_cast<DemandDeposit>(dep))
                    kindName = "До востреб.";
                else if (dynamic_pointer_cast<SavingsDeposit>(dep))
                    kindName = "Накопительн.";

                out << "  " << dep->getContractNumber()
                    << " | " << formatUtf8(kindName, 13)
                    << " | Клиент: " << formatUtf8(dep->getClientName(), 24)
                    << " | Баланс: " << right << setw(10) << dep->getBalance() << " BYN"
                    << " | Ставка: " << dep->getInterestRate() << "%\n"
                    << left;
            }
        }

        out << "\n--------------------------------------------------------------------\n";
        out << "ЖУРНАЛ ОПЕРАЦИЙ:\n";
        out << "--------------------------------------------------------------------\n";
        if (transList.empty())
        {
            out << "  Операции не зарегистрированы.\n";
        }
        else
        {
            for (const auto &tr : transList)
            {
                out << "  " << tr->toReportLine() << "\n";
            }
        }

        out << "\n====================================================================\n";
        out << "Конец отчета.\n";

        if (!out)
        {
            throw FileException("Ошибка при записи сводного отчета в файл: " + filename);
        }
        out.close();
    }

    void FileStorage::exportClientStatement(const string &filename,
                                            const Client &client,
                                            const vector<shared_ptr<Deposit>> &clientDeposits,
                                            const vector<shared_ptr<Transaction>> &clientTransactions) const
    {
        ofstream out(filename, ios::trunc);
        if (!out.is_open())
        {
            throw FileException("Не удалось создать файл выписки: " + filename);
        }

        double clientTotal = 0.0;
        for (const auto &dep : clientDeposits)
        {
            clientTotal += dep->getBalance();
        }

        out << "====================================================================\n";
        out << "                   ВЫПИСКА ПО ВКЛАДАМ КЛИЕНТА\n";
        out << "                   Дата: " << Transaction::getCurrentDate() << "\n";
        out << "====================================================================\n\n";

        out << "ДАННЫЕ КЛИЕНТА:\n";
        out << "  ID клиента:        " << client.getId() << "\n";
        out << "  ФИО:               " << client.getFullName() << "\n";
        out << "  Паспорт:           " << client.getPassportNumber() << "\n";
        out << "  Телефон:           " << client.getPhoneNumber() << "\n";
        out << "  Email:             " << client.getEmail() << "\n";
        out << "  Статус:            " << (client.getIsVip() ? "VIP-клиент" : "Стандартный") << "\n\n";

        out << fixed << setprecision(2);
        out << "ОТКРЫТЫЕ ВКЛАДЫ (" << clientDeposits.size() << "):\n";
        out << "--------------------------------------------------------------------\n";
        if (clientDeposits.empty())
        {
            out << "  У клиента нет активных вкладов.\n";
        }
        else
        {
            for (const auto &dep : clientDeposits)
            {
                out << "  Договор: " << dep->getContractNumber()
                    << " | Баланс: " << dep->getBalance() << " BYN"
                    << " | Ставка: " << dep->getInterestRate() << "%\n";
            }
            out << "--------------------------------------------------------------------\n";
            out << "  ИТОГО НА СЧЕТАХ КЛИЕНТА: " << clientTotal << " BYN\n";
        }

        out << "\nИСТОРИЯ ОПЕРАЦИЙ ПО СЧЕТАМ КЛИЕНТА:\n";
        out << "--------------------------------------------------------------------\n";
        if (clientTransactions.empty())
        {
            out << "  Операции отсутствуют.\n";
        }
        else
        {
            for (const auto &tr : clientTransactions)
            {
                out << "  " << tr->toReportLine() << "\n";
            }
        }

        out << "\n====================================================================\n";

        if (!out)
        {
            throw FileException("Ошибка при записи выписки клиента в файл: " + filename);
        }
        out.close();
    }
}
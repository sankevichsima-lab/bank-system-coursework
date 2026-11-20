#include "Transaction.h"
#include <iomanip>
#include <sstream>
#include <ctime>

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

        void writeStringBinary(ofstream &out, const string &str)
        {
            size_t len = str.size();
            out.write(reinterpret_cast<const char *>(&len), sizeof(len));
            if (len > 0)
            {
                out.write(str.data(), static_cast<streamsize>(len));
            }
            if (!out)
            {
                throw TransactionException("Не удалось записать строковые данные транзакции");
            }
        }

        void readStringBinary(ifstream &in, string &str)
        {
            size_t len = 0;
            in.read(reinterpret_cast<char *>(&len), sizeof(len));
            if (!in || len > 10000)
            {
                throw TransactionException("Поврежденный формат файла транзакций");
            }
            str.resize(len);
            if (len > 0)
            {
                in.read(&str[0], static_cast<streamsize>(len));
            }
            if (!in)
            {
                throw TransactionException("Не удалось прочитать строковые данные транзакции");
            }
        }
    }

    int Transaction::nextTransactionId = 1;

    Transaction::Transaction()
        : id(0),
          accountId(0),
          contractNumber(""),
          type(TransactionType::INITIAL_DEPOSIT),
          amount(0.0),
          balanceAfter(0.0),
          date(getCurrentDate()),
          description("") {}

    Transaction::Transaction(int transId,
                             int accId,
                             const string &contractNum,
                             TransactionType transType,
                             double transAmount,
                             double newBalance,
                             const string &transDate,
                             const string &desc)
        : id(transId),
          accountId(accId),
          contractNumber(contractNum),
          type(transType),
          amount(transAmount),
          balanceAfter(newBalance),
          date(transDate.empty() ? getCurrentDate() : transDate),
          description(desc)
    {
        if (transId <= 0)
        {
            throw TransactionException("ID транзакции должен быть положительным числом");
        }
        if (accId <= 0)
        {
            throw TransactionException("ID счета в транзакции должен быть положительным числом");
        }
        if (contractNum.empty())
        {
            throw TransactionException("Номер договора не может быть пустым");
        }
        if (transAmount < 0.0)
        {
            throw TransactionException("Сумма транзакции не может быть отрицательной");
        }
        if (newBalance < 0.0)
        {
            throw TransactionException("Баланс после операции не может быть отрицательным");
        }
        syncNextId(transId);
    }

    void Transaction::printInfo() const
    {
        cout << "----------------------------------------\n";
        cout << "Операция [ID: " << id << "]\n";
        cout << "Дата:            " << date << "\n";
        cout << "Договор:         " << contractNumber << " (ID счета: " << accountId << ")\n";
        cout << "Тип операции:    " << typeToString(type) << "\n";
        cout << fixed << setprecision(2);
        cout << "Сумма:           " << amount << " BYN\n";
        cout << "Баланс после:    " << balanceAfter << " BYN\n";
        if (!description.empty())
        {
            cout << "Комментарий:     " << description << "\n";
        }
        cout << "----------------------------------------\n";
    }

    void Transaction::printTableRow() const
    {
        cout << "| " << left << setw(4) << id
             << " | " << formatUtf8(date, 10)
             << " | " << formatUtf8(contractNumber, 12)
             << " | " << formatUtf8(typeToString(type), 22)
             << " | " << right << setw(10) << fixed << setprecision(2) << amount
             << " | " << setw(10) << balanceAfter
             << " |\n"
             << left;
    }

    string Transaction::toReportLine() const
    {
        ostringstream oss;
        oss << fixed << setprecision(2);
        oss << "[" << date << "] ID:" << id
            << " | Договор: " << contractNumber
            << " | " << typeToString(type)
            << " | Сумма: " << amount << " BYN"
            << " | Остаток: " << balanceAfter << " BYN";
        if (!description.empty())
        {
            oss << " | Прим.: " << description;
        }
        return oss.str();
    }

    void Transaction::writeToBinary(ofstream &out) const
    {
        if (!out.is_open())
        {
            throw TransactionException("Файл для записи транзакций не открыт");
        }
        int typeValue = static_cast<int>(type);

        out.write(reinterpret_cast<const char *>(&id), sizeof(id));
        out.write(reinterpret_cast<const char *>(&accountId), sizeof(accountId));
        writeStringBinary(out, contractNumber);
        out.write(reinterpret_cast<const char *>(&typeValue), sizeof(typeValue));
        out.write(reinterpret_cast<const char *>(&amount), sizeof(amount));
        out.write(reinterpret_cast<const char *>(&balanceAfter), sizeof(balanceAfter));
        writeStringBinary(out, date);
        writeStringBinary(out, description);

        if (!out)
        {
            throw TransactionException("Ошибка записи объекта Transaction");
        }
    }

    void Transaction::readFromBinary(ifstream &in)
    {
        if (!in.is_open())
        {
            throw TransactionException("Файл для чтения транзакций не открыт");
        }
        int typeValue = 0;

        in.read(reinterpret_cast<char *>(&id), sizeof(id));
        in.read(reinterpret_cast<char *>(&accountId), sizeof(accountId));
        if (!in)
        {
            throw TransactionException("Ошибка чтения идентификаторов транзакции");
        }
        readStringBinary(in, contractNumber);
        in.read(reinterpret_cast<char *>(&typeValue), sizeof(typeValue));
        in.read(reinterpret_cast<char *>(&amount), sizeof(amount));
        in.read(reinterpret_cast<char *>(&balanceAfter), sizeof(balanceAfter));
        if (!in || typeValue < 1 || typeValue > 5)
        {
            throw TransactionException("Некорректные данные транзакции");
        }
        type = static_cast<TransactionType>(typeValue);

        readStringBinary(in, date);
        readStringBinary(in, description);

        syncNextId(id);
    }

    int Transaction::generateNextId()
    {
        return nextTransactionId++;
    }

    void Transaction::syncNextId(int loadedId)
    {
        if (loadedId >= nextTransactionId)
        {
            nextTransactionId = loadedId + 1;
        }
    }

    string Transaction::typeToString(TransactionType type)
    {
        switch (type)
        {
        case TransactionType::INITIAL_DEPOSIT:
            return "Открытие вклада";
        case TransactionType::TOP_UP:
            return "Пополнение";
        case TransactionType::WITHDRAWAL:
            return "Частичное снятие";
        case TransactionType::INTEREST_ACCRUAL:
            return "Начисление процентов";
        case TransactionType::CLOSURE:
            return "Закрытие вклада";
        default:
            return "Неизвестная операция";
        }
    }

    string Transaction::getCurrentDate()
    {
        time_t now = time(nullptr);
        tm *ltm = localtime(&now);
        ostringstream oss;
        oss << setfill('0') << setw(2) << ltm->tm_mday << "."
            << setw(2) << (1 + ltm->tm_mon) << "."
            << (1900 + ltm->tm_year);
        return oss.str();
    }

    bool Transaction::operator==(const Transaction &other) const
    {
        return id == other.id;
    }

    bool Transaction::operator<(const Transaction &other) const
    {
        return id < other.id;
    }

    ostream &operator<<(ostream &os, const Transaction &tr)
    {
        os << tr.toReportLine();
        return os;
    }
}
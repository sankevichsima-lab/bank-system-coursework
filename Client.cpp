#include "Client.h"
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <limits>

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
                throw ClientException("Не удалось записать строковые данные клиента");
            }
        }

        void readStringBinary(ifstream &in, string &str)
        {
            size_t len = 0;
            in.read(reinterpret_cast<char *>(&len), sizeof(len));
            if (!in || len > 10000)
            {
                throw ClientException("Поврежденный формат файла клиентов");
            }
            str.resize(len);
            if (len > 0)
            {
                in.read(&str[0], static_cast<streamsize>(len));
            }
            if (!in)
            {
                throw ClientException("Не удалось прочитать строковые данные клиента");
            }
        }
    }

    int Client::nextClientId = 1;

    Client::Client()
        : id(0),
          fullName("Не указано"),
          passportNumber(""),
          phoneNumber(""),
          email(""),
          registrationDate("01.01.2026"),
          isVip(false) {}

    Client::Client(int clientId,
                   const string &name,
                   const string &passport,
                   const string &phone,
                   const string &mail,
                   const string &regDate,
                   bool vip)
        : id(clientId), isVip(vip)
    {
        if (clientId <= 0)
        {
            throw ClientException("ID клиента должен быть положительным числом");
        }
        setFullName(name);
        setPassportNumber(passport);
        setPhoneNumber(phone);
        setEmail(mail);
        setRegistrationDate(regDate);
        syncNextId(clientId);
    }

    void Client::setFullName(const string &name)
    {
        if (name.empty() || name.size() < 2)
        {
            throw ClientException("ФИО клиента не может быть пустым или короче 2 символов");
        }
        fullName = name;
    }

    void Client::setPassportNumber(const string &passport)
    {
        if (!isValidPassport(passport))
        {
            throw ClientException("Некорректный формат паспорта (ожидается от 6 до 20 букв/цифр, например MP1234567)");
        }
        passportNumber = passport;
    }

    void Client::setPhoneNumber(const string &phone)
    {
        if (!isValidPhone(phone))
        {
            throw ClientException("Некорректный формат телефона (должен содержать от 7 до 15 цифр)");
        }
        phoneNumber = phone;
    }

    void Client::setEmail(const string &mail)
    {
        if (!isValidEmail(mail))
        {
            throw ClientException("Некорректный формат электронной почты (пример: user@mail.by)");
        }
        email = mail;
    }

    void Client::setRegistrationDate(const string &regDate)
    {
        if (regDate.size() != 10 || regDate[2] != '.' || regDate[5] != '.')
        {
            throw ClientException("Дата регистрации должна быть в формате ДД.ММ.ГГГГ");
        }
        registrationDate = regDate;
    }

    void Client::setVipStatus(bool vip)
    {
        isVip = vip;
    }

    void Client::updateContact(const string &newPhone)
    {
        setPhoneNumber(newPhone);
    }

    void Client::updateContact(const string &newPhone, const string &newEmail)
    {
        setPhoneNumber(newPhone);
        setEmail(newEmail);
    }

    void Client::printInfo() const
    {
        cout << "========================================\n";
        cout << "Карточка клиента [ID: " << id << "]\n";
        cout << "ФИО:              " << fullName << "\n";
        cout << "Паспорт:          " << passportNumber << "\n";
        cout << "Телефон:          " << phoneNumber << "\n";
        cout << "Email:            " << email << "\n";
        cout << "Дата регистрации: " << registrationDate << "\n";
        cout << "Статус:           " << (isVip ? "VIP-клиент" : "Стандартный") << "\n";
        cout << "========================================\n";
    }

    void Client::printTableRow() const
    {
        cout << "| " << left << setw(4) << id
             << " | " << formatUtf8(fullName, 24)
             << " | " << formatUtf8(passportNumber, 12)
             << " | " << formatUtf8(phoneNumber, 16)
             << " | " << formatUtf8(isVip ? "VIP" : "Обычн.", 6)
             << " |\n";
    }

    void Client::writeToBinary(ofstream &out) const
    {
        if (!out.is_open())
        {
            throw ClientException("Файл для записи данных клиента не открыт");
        }
        out.write(reinterpret_cast<const char *>(&id), sizeof(id));
        writeStringBinary(out, fullName);
        writeStringBinary(out, passportNumber);
        writeStringBinary(out, phoneNumber);
        writeStringBinary(out, email);
        writeStringBinary(out, registrationDate);
        out.write(reinterpret_cast<const char *>(&isVip), sizeof(isVip));

        if (!out)
        {
            throw ClientException("Ошибка записи объекта Client");
        }
    }

    void Client::readFromBinary(ifstream &in)
    {
        if (!in.is_open())
        {
            throw ClientException("Файл для чтения данных клиента не открыт");
        }
        in.read(reinterpret_cast<char *>(&id), sizeof(id));
        if (!in)
        {
            throw ClientException("Ошибка чтения ID клиента");
        }
        readStringBinary(in, fullName);
        readStringBinary(in, passportNumber);
        readStringBinary(in, phoneNumber);
        readStringBinary(in, email);
        readStringBinary(in, registrationDate);
        in.read(reinterpret_cast<char *>(&isVip), sizeof(isVip));

        if (!in)
        {
            throw ClientException("Ошибка чтения статуса клиента");
        }
        syncNextId(id);
    }

    int Client::generateNextId()
    {
        return nextClientId++;
    }

    void Client::syncNextId(int loadedId)
    {
        if (loadedId >= nextClientId)
        {
            nextClientId = loadedId + 1;
        }
    }

    bool Client::isValidPassport(const string &passport)
    {
        int validChars = 0;
        for (size_t i = 0; i < passport.size();)
        {
            unsigned char c = static_cast<unsigned char>(passport[i]);
            if (isalnum(c))
            {
                validChars++;
                i++;
            }
            else if (c == ' ' || c == '-')
            {
                i++;
            }
            else if ((c == 0xD0 || c == 0xD1) && i + 1 < passport.size() &&
                     (static_cast<unsigned char>(passport[i + 1]) & 0xC0) == 0x80)
            {
                validChars++;
                i += 2;
            }
            else
            {
                return false;
            }
        }
        return validChars >= 6 && validChars <= 20;
    }

    bool Client::isValidPhone(const string &phone)
    {
        int digitCount = 0;
        for (unsigned char c : phone)
        {
            if (isdigit(c))
            {
                digitCount++;
            }
            else if (c != '+' && c != '-' && c != ' ' && c != '(' && c != ')')
            {
                return false;
            }
        }
        return digitCount >= 7 && digitCount <= 15;
    }

    bool Client::isValidEmail(const string &mail)
    {
        if (mail.empty() || mail.find(' ') != string::npos)
        {
            return false;
        }
        size_t atPos = mail.find('@');
        if (atPos == string::npos || atPos == 0)
        {
            return false;
        }
        size_t dotPos = mail.find('.', atPos + 2);
        if (dotPos == string::npos || dotPos == mail.size() - 1)
        {
            return false;
        }
        return true;
    }

    bool Client::operator==(const Client &other) const
    {
        return id == other.id || passportNumber == other.passportNumber;
    }

    bool Client::operator<(const Client &other) const
    {
        return fullName < other.fullName;
    }

    ostream &operator<<(ostream &os, const Client &client)
    {
        os << "[ID: " << client.id << "] " << client.fullName
           << " | Паспорт: " << client.passportNumber
           << " | Тел.: " << client.phoneNumber
           << " | " << (client.isVip ? "VIP" : "Стандарт");
        return os;
    }

    istream &operator>>(istream &is, Client &client)
    {
        string name, passport, phone, mail, regDate;
        int vipChoice = 0;

        if (is.peek() == '\n')
        {
            is.ignore();
        }

        cout << "Введите ФИО клиента: ";
        getline(is, name);

        cout << "Введите серию и номер паспорта (например, MP1234567): ";
        getline(is, passport);

        cout << "Введите номер телефона (например, +375291234567): ";
        getline(is, phone);

        cout << "Введите email: ";
        getline(is, mail);

        cout << "Введите дату регистрации (ДД.ММ.ГГГГ): ";
        getline(is, regDate);

        cout << "Статус VIP (1 - да, 0 - нет): ";
        if (!(is >> vipChoice) || (vipChoice != 0 && vipChoice != 1))
        {
            is.clear();
            is.ignore(numeric_limits<streamsize>::max(), '\n');
            throw ClientException("Ожидалось число 1 (да) или 0 (нет) для статуса VIP");
        }
        is.ignore(numeric_limits<streamsize>::max(), '\n');

        client.id = Client::generateNextId();
        client.setFullName(name);
        client.setPassportNumber(passport);
        client.setPhoneNumber(phone);
        client.setEmail(mail);
        client.setRegistrationDate(regDate);
        client.setVipStatus(vipChoice == 1);

        return is;
    }
}
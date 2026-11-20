#pragma once
#include "BankClasses.h"
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

namespace BankSystem
{
    class ClientException : public BankException
    {
    public:
        explicit ClientException(const string &msg)
            : BankException("Ошибка данных клиента: " + msg) {}
    };

    class Client
    {
    private:
        int id;
        string fullName;
        string passportNumber;
        string phoneNumber;
        string email;
        string registrationDate;
        bool isVip;

        static int nextClientId;

    public:
        Client();
        Client(int clientId,
               const string &name,
               const string &passport,
               const string &phone,
               const string &mail,
               const string &regDate,
               bool vip = false);

        int getId() const { return id; }
        const string &getFullName() const { return fullName; }
        const string &getPassportNumber() const { return passportNumber; }
        const string &getPhoneNumber() const { return phoneNumber; }
        const string &getEmail() const { return email; }
        const string &getRegistrationDate() const { return registrationDate; }
        bool getIsVip() const { return isVip; }

        void setFullName(const string &name);
        void setPassportNumber(const string &passport);
        void setPhoneNumber(const string &phone);
        void setEmail(const string &mail);
        void setRegistrationDate(const string &regDate);
        void setVipStatus(bool vip);

        void updateContact(const string &newPhone);
        void updateContact(const string &newPhone, const string &newEmail);

        void printInfo() const;
        void printTableRow() const;

        void writeToBinary(ofstream &out) const;
        void readFromBinary(ifstream &in);

        static int generateNextId();
        static void syncNextId(int loadedId);
        static bool isValidPassport(const string &passport);
        static bool isValidPhone(const string &phone);
        static bool isValidEmail(const string &mail);

        bool operator==(const Client &other) const;
        bool operator<(const Client &other) const;
        friend ostream &operator<<(ostream &os, const Client &client);
        friend istream &operator>>(istream &is, Client &client);
    };
}
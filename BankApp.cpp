#include "BankApp.h"
#include <iomanip>
#include <limits>
#include <algorithm>
#include <cctype>

using namespace std;

namespace BankSystem
{
    namespace
    {
        string toLowerStr(const string &s)
        {
            string res = s;
            for (size_t i = 0; i < res.size(); ++i)
            {
                res[i] = static_cast<char>(tolower(static_cast<unsigned char>(res[i])));
            }
            return res;
        }

        bool containsSubstring(const string &text, const string &query)
        {
            if (query.empty())
            {
                return true;
            }
            if (text.find(query) != string::npos)
            {
                return true;
            }
            return toLowerStr(text).find(toLowerStr(query)) != string::npos;
        }
    }

    BankApp::BankApp() : currentUser(nullptr), nextDepositId(1) {}

    void BankApp::loadSystemData()
    {
        FileStorage::getInstance().loadAll(clientManager, depositManager, transactionManager, userManager);
        initDefaultDataIfEmpty();
        syncNextDepositId();
    }

    void BankApp::saveSystemData() const
    {
        FileStorage::getInstance().saveAll(clientManager, depositManager, transactionManager, userManager);
    }

    void BankApp::initDefaultDataIfEmpty()
    {
        if (userManager.size() == 0)
        {
            userManager.addRecord(make_shared<Admin>("admin", "12345"));
            userManager.addRecord(make_shared<Operator>("oper", "0000"));
        }

        if (clientManager.size() == 0)
        {
            auto c1 = make_shared<Client>(Client::generateNextId(),
                                          "Иванов Иван Иванович",
                                          "MP1234567",
                                          "+375291112233",
                                          "ivanov@mail.by",
                                          "10.01.2026",
                                          true);
            auto c2 = make_shared<Client>(Client::generateNextId(),
                                          "Петрова Анна Сергеевна",
                                          "KH7654321",
                                          "+375334445566",
                                          "petrova@gmail.com",
                                          "15.02.2026",
                                          false);
            clientManager.addRecord(c1);
            clientManager.addRecord(c2);
        }

        if (depositManager.size() == 0)
        {
            auto d1 = DepositFactory::createTermDeposit(1, "Иванов Иван Иванович", 5000.0, 12.5, 12, "Безотзывный вклад");
            auto d2 = DepositFactory::createDemandDeposit(2, "Петрова Анна Сергеевна", 1200.0, 2.0, "15.02.2026");
            auto d3 = DepositFactory::createSavingsDeposit(3, "Иванов Иван Иванович", 3000.0, 9.5, 500.0, 6, true);

            depositManager.addRecord(d1);
            depositManager.addRecord(d2);
            depositManager.addRecord(d3);

            logTransaction(d1, TransactionType::INITIAL_DEPOSIT, 5000.0, "Открытие срочного вклада");
            logTransaction(d2, TransactionType::INITIAL_DEPOSIT, 1200.0, "Открытие вклада до востребования");
            logTransaction(d3, TransactionType::INITIAL_DEPOSIT, 3000.0, "Открытие накопительного вклада");
        }
    }

    void BankApp::syncNextDepositId()
    {
        int maxId = 0;
        for (const auto &dep : depositManager.getAllRecords())
        {
            if (dep && dep->getId() > maxId)
            {
                maxId = dep->getId();
            }
        }
        nextDepositId = maxId + 1;
    }

    void BankApp::logTransaction(const shared_ptr<Deposit> &dep,
                                 TransactionType type,
                                 double amount,
                                 const string &description)
    {
        if (!dep)
        {
            return;
        }
        auto tr = make_shared<Transaction>(Transaction::generateNextId(),
                                           dep->getId(),
                                           dep->getContractNumber(),
                                           type,
                                           amount,
                                           dep->getBalance(),
                                           Transaction::getCurrentDate(),
                                           description);
        transactionManager.addRecord(tr);
    }

    int BankApp::readInt(const string &prompt, int minVal, int maxVal) const
    {
        int value = 0;
        while (true)
        {
            cout << prompt;
            if (cin >> value && value >= minVal && value <= maxVal)
            {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return value;
            }
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Некорректный ввод. Введите целое число от " << minVal << " до " << maxVal << ".\n";
        }
    }

    double BankApp::readDouble(const string &prompt, double minVal) const
    {
        double value = 0.0;
        while (true)
        {
            cout << prompt;
            if (cin >> value && value >= minVal)
            {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return value;
            }
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Некорректный ввод. Введите число не меньше " << minVal << ".\n";
        }
    }

    string BankApp::readLine(const string &prompt) const
    {
        string input;
        cout << prompt;
        getline(cin, input);
        return input;
    }

    void BankApp::pauseConsole() const
    {
        cout << "\nНажмите Enter для продолжения...";
        cin.get();
    }

    void BankApp::run()
    {
        loadSystemData();

        cout << "=========================================================\n";
        cout << "  АВТОМАТИЗИРОВАННАЯ СИСТЕМА УПРАВЛЕНИЯ ВКЛАДАМИ БАНКА\n";
        cout << "=========================================================\n";

        while (true)
        {
            if (!loginScreen())
            {
                break;
            }

            if (dynamic_pointer_cast<Admin>(currentUser))
            {
                runAdminSession();
            }
            else if (dynamic_pointer_cast<Operator>(currentUser))
            {
                runOperatorSession();
            }

            try
            {
                saveSystemData();
                cout << "\nДанные успешно сохранены в файлы базы.\n";
            }
            catch (const BankException &e)
            {
                cerr << "\n[ОШИБКА СОХРАНЕНИЯ]: " << e.what() << "\n";
            }

            currentUser = nullptr;
            int nextAction = readInt("\n1. Войти под другой учетной записью\n0. Завершить работу программы\nВаш выбор: ", 0, 1);
            if (nextAction == 0)
            {
                break;
            }
        }

        cout << "\nРабота системы завершена. До свидания!\n";
    }

    bool BankApp::loginScreen()
    {
        cout << "\n--- АВТОРИЗАЦИЯ В СИСТЕМЕ ---\n";
        cout << "(Для выхода из программы введите логин: exit)\n";

        int attempts = 3;
        while (attempts > 0)
        {
            string login = readLine("Логин: ");
            if (login == "exit")
            {
                return false;
            }
            string password = readLine("Пароль: ");

            auto foundUser = userManager.findByCondition([&](const shared_ptr<User> &u)
                                                         { return u->getLogin() == login; });

            if (foundUser && authenticate(*foundUser, password))
            {
                currentUser = foundUser;
                cout << "\nАвторизация успешна! Добро пожаловать, " << currentUser->getLogin() << ".\n";
                return true;
            }

            attempts--;
            cout << "Неверный логин или пароль. Осталось попыток: " << attempts << "\n";
        }

        throw AuthException("Превышено допустимое количество неудачных попыток входа");
    }

    void BankApp::runAdminSession()
    {
        while (true)
        {
            currentUser->showMenu();
            cout << "Расширенная панель Администратора:\n";
            cout << "1. Управление клиентами\n";
            cout << "2. Управление вкладами (просмотр, открытие, изменение, удаление)\n";
            cout << "3. Финансовые операции по вкладам и журнал транзакций\n";
            cout << "4. Поиск, фильтрация и сортировка вкладов\n";
            cout << "5. Аналитика и формирование отчетов (.txt)\n";
            cout << "6. Управление пользователями системы\n";
            cout << "7. Сохранить данные в файлы\n";
            cout << "0. Выход из учетной записи\n";

            int choice = readInt("Выберите раздел (0-7): ", 0, 7);

            try
            {
                switch (choice)
                {
                case 1:
                    manageClientsMenu();
                    break;
                case 2:
                    manageDepositsMenu(true);
                    break;
                case 3:
                    depositOperationsMenu();
                    break;
                case 4:
                    searchAndFilterDepositsMenu();
                    break;
                case 5:
                    reportsMenu();
                    break;
                case 6:
                    manageUsersMenu();
                    break;
                case 7:
                    saveSystemData();
                    cout << "\nВсе данные успешно сохранены в бинарные файлы.\n";
                    pauseConsole();
                    break;
                case 0:
                    return;
                }
            }
            catch (const BankException &e)
            {
                cerr << "\n[ОШИБКА]: " << e.what() << "\n";
                pauseConsole();
            }
        }
    }

    void BankApp::runOperatorSession()
    {
        while (true)
        {
            currentUser->showMenu();
            cout << "Рабочая панель Операциониста:\n";
            cout << "1. Клиенты (просмотр, регистрация, поиск)\n";
            cout << "2. Вклады (просмотр и оформление нового договора)\n";
            cout << "3. Операции по вкладам (пополнение, снятие, начисление %)\n";
            cout << "4. Поиск, фильтрация и сортировка вкладов\n";
            cout << "5. Формирование отчетов и выписок (.txt)\n";
            cout << "0. Выход из учетной записи\n";

            int choice = readInt("Выберите раздел (0-5): ", 0, 5);

            try
            {
                switch (choice)
                {
                case 1:
                    manageClientsMenu();
                    break;
                case 2:
                    manageDepositsMenu(false);
                    break;
                case 3:
                    depositOperationsMenu();
                    break;
                case 4:
                    searchAndFilterDepositsMenu();
                    break;
                case 5:
                    reportsMenu();
                    break;
                case 0:
                    return;
                }
            }
            catch (const BankException &e)
            {
                cerr << "\n[ОШИБКА]: " << e.what() << "\n";
                pauseConsole();
            }
        }
    }

    void BankApp::manageClientsMenu()
    {
        bool isAdmin = (dynamic_pointer_cast<Admin>(currentUser) != nullptr);

        while (true)
        {
            cout << "\n--- УПРАВЛЕНИЕ КЛИЕНТАМИ ---\n";
            cout << "1. Просмотр всех клиентов\n";
            cout << "2. Регистрация нового клиента\n";
            cout << "3. Поиск клиента (по ФИО или паспорту)\n";
            cout << "4. Сортировка клиентов\n";
            if (isAdmin)
            {
                cout << "5. Редактирование данных клиента\n";
                cout << "6. Удаление клиента\n";
            }
            cout << "0. Назад\n";

            int maxChoice = isAdmin ? 6 : 4;
            int choice = readInt("Ваш выбор: ", 0, maxChoice);

            try
            {
                switch (choice)
                {
                case 1:
                    viewAllClients();
                    pauseConsole();
                    break;
                case 2:
                    addClient();
                    pauseConsole();
                    break;
                case 3:
                    searchClients();
                    pauseConsole();
                    break;
                case 4:
                    sortClients();
                    pauseConsole();
                    break;
                case 5:
                    editClient();
                    pauseConsole();
                    break;
                case 6:
                    deleteClient();
                    pauseConsole();
                    break;
                case 0:
                    return;
                }
            }
            catch (const BankException &e)
            {
                cerr << "\n[ОШИБКА]: " << e.what() << "\n";
                pauseConsole();
            }
        }
    }

    void BankApp::viewAllClients() const
    {
        const auto &clients = clientManager.getAllRecords();
        if (clients.empty())
        {
            cout << "\nБаза клиентов пуста.\n";
            return;
        }

        cout << "\n+------+--------------------------+--------------+------------------+--------+\n";
        cout << "| ID   | ФИО клиента              | Паспорт      | Телефон          | Статус |\n";
        cout << "+------+--------------------------+--------------+------------------+--------+\n";
        for (const auto &c : clients)
        {
            c->printTableRow();
        }
        cout << "+------+--------------------------+--------------+------------------+--------+\n";
        cout << "Всего клиентов: " << clients.size() << "\n";
    }

    void BankApp::addClient()
    {
        cout << "\n--- РЕГИСТРАЦИЯ НОВОГО КЛИЕНТА ---\n";
        auto newClient = make_shared<Client>();
        cin >> *newClient;

        auto duplicate = clientManager.findByCondition([&](const shared_ptr<Client> &c)
                                                       { return c->getPassportNumber() == newClient->getPassportNumber(); });
        if (duplicate)
        {
            throw ClientException("Клиент с таким номером паспорта уже зарегистрирован в системе");
        }

        clientManager.addRecord(newClient);
        cout << "\nКлиент успешно зарегистрирован!\n";
        newClient->printInfo();
    }

    void BankApp::editClient()
    {
        viewAllClients();
        if (clientManager.size() == 0)
        {
            return;
        }

        int id = readInt("Введите ID клиента для редактирования: ", 1, 1000000);
        auto client = clientManager.findByCondition([id](const shared_ptr<Client> &c)
                                                    { return c->getId() == id; });
        if (!client)
        {
            throw ClientException("Клиент с указанным ID не найден");
        }

        client->printInfo();
        cout << "Что изменить?\n";
        cout << "1. ФИО\n";
        cout << "2. Телефон\n";
        cout << "3. Телефон и Email\n";
        cout << "4. VIP-статус\n";
        cout << "0. Отмена\n";

        int field = readInt("Ваш выбор: ", 0, 4);
        switch (field)
        {
        case 1:
        {
            string oldName = client->getFullName();
            string newName = readLine("Новое ФИО: ");
            client->setFullName(newName);
            for (const auto &dep : depositManager.getAllRecords())
            {
                if (dep->getClientName() == oldName)
                {
                    dep->setClientName(newName);
                }
            }
            break;
        }
        case 2:
        {
            string phone = readLine("Новый телефон: ");
            client->updateContact(phone);
            break;
        }
        case 3:
        {
            string phone = readLine("Новый телефон: ");
            string email = readLine("Новый email: ");
            client->updateContact(phone, email);
            break;
        }
        case 4:
        {
            int vip = readInt("VIP-статус (1 - да, 0 - нет): ", 0, 1);
            client->setVipStatus(vip == 1);
            break;
        }
        case 0:
            return;
        }

        cout << "\nДанные клиента успешно обновлены.\n";
    }

    void BankApp::deleteClient()
    {
        viewAllClients();
        if (clientManager.size() == 0)
        {
            return;
        }

        int id = readInt("Введите ID клиента для удаления: ", 1, 1000000);
        auto client = clientManager.findByCondition([id](const shared_ptr<Client> &c)
                                                    { return c->getId() == id; });
        if (!client)
        {
            throw ClientException("Клиент с таким ID не найден");
        }

        auto activeDep = depositManager.findByCondition([&](const shared_ptr<Deposit> &d)
                                                        { return d->getClientName() == client->getFullName(); });
        if (activeDep)
        {
            throw ClientException("Нельзя удалить клиента, пока у него есть открытые вклады");
        }

        clientManager.removeRecord(id);
        cout << "\nКлиент успешно удален из базы.\n";
    }

    void BankApp::searchClients() const
    {
        string query = readLine("Введите ФИО или номер паспорта для поиска: ");
        auto results = clientManager.filterRecords([&](const shared_ptr<Client> &c)
                                                   { return containsSubstring(c->getFullName(), query) ||
                                                            containsSubstring(c->getPassportNumber(), query); });

        if (results.empty())
        {
            cout << "\nКлиенты по запросу \"" << query << "\" не найдены.\n";
            return;
        }

        cout << "\nНайдено клиентов: " << results.size() << "\n";
        for (const auto &c : results)
        {
            c->printInfo();
        }
    }

    void BankApp::sortClients()
    {
        cout << "\nСортировать клиентов:\n";
        cout << "1. По ФИО (А -> Я)\n";
        cout << "2. По ID (по возрастанию)\n";
        cout << "3. Сначала VIP-клиенты\n";
        int mode = readInt("Ваш выбор (1-3): ", 1, 3);

        if (mode == 1)
        {
            clientManager.sortRecords([](const shared_ptr<Client> &a, const shared_ptr<Client> &b)
                                      { return a->getFullName() < b->getFullName(); });
        }
        else if (mode == 2)
        {
            clientManager.sortRecords([](const shared_ptr<Client> &a, const shared_ptr<Client> &b)
                                      { return a->getId() < b->getId(); });
        }
        else
        {
            clientManager.sortRecords([](const shared_ptr<Client> &a, const shared_ptr<Client> &b)
                                      { return a->getIsVip() > b->getIsVip(); });
        }

        cout << "\nСписок клиентов отсортирован:\n";
        viewAllClients();
    }

    void BankApp::manageDepositsMenu(bool isAdmin)
    {
        while (true)
        {
            cout << "\n--- УПРАВЛЕНИЕ ВКЛАДАМИ ---\n";
            cout << "1. Просмотр всех вкладов\n";
            cout << "2. Оформить новый вклад\n";
            if (isAdmin)
            {
                cout << "3. Редактировать параметры вклада\n";
                cout << "4. Закрыть / удалить вклад\n";
            }
            cout << "0. Назад\n";

            int maxChoice = isAdmin ? 4 : 2;
            int choice = readInt("Ваш выбор: ", 0, maxChoice);

            try
            {
                switch (choice)
                {
                case 1:
                    viewAllDeposits();
                    pauseConsole();
                    break;
                case 2:
                    openNewDeposit();
                    pauseConsole();
                    break;
                case 3:
                    editDeposit();
                    pauseConsole();
                    break;
                case 4:
                    closeOrDeleteDeposit();
                    pauseConsole();
                    break;
                case 0:
                    return;
                }
            }
            catch (const BankException &e)
            {
                cerr << "\n[ОШИБКА]: " << e.what() << "\n";
                pauseConsole();
            }
        }
    }

    void BankApp::viewAllDeposits() const
    {
        const auto &deposits = depositManager.getAllRecords();
        if (deposits.empty())
        {
            cout << "\nВ базе пока нет открытых вкладов.\n";
            return;
        }

        cout << "\n=== СПИСОК ВСЕХ ВКЛАДОВ (" << deposits.size() << ") ===\n";
        for (const auto &dep : deposits)
        {
            dep->printReport();
        }
    }

    void BankApp::openNewDeposit()
    {
        if (clientManager.size() == 0)
        {
            throw ClientException("Сначала зарегистрируйте хотя бы одного клиента");
        }

        viewAllClients();
        int clientId = readInt("Введите ID клиента для оформления вклада: ", 1, 1000000);
        auto client = clientManager.findByCondition([clientId](const shared_ptr<Client> &c)
                                                    { return c->getId() == clientId; });
        if (!client)
        {
            throw ClientException("Клиент с таким ID не найден");
        }

        int newDepId = nextDepositId++;
        auto dep = DepositFactory::createDepositInteractive(newDepId, client->getFullName());
        depositManager.addRecord(dep);

        logTransaction(dep, TransactionType::INITIAL_DEPOSIT, dep->getBalance(),
                       "Оформление нового вклада клиенту " + client->getFullName());

        cout << "\nВклад успешно оформлен!\n";
        dep->printReport();
    }

    void BankApp::editDeposit()
    {
        viewAllDeposits();
        if (depositManager.size() == 0)
        {
            return;
        }

        int depId = readInt("Введите ID вклада для редактирования: ", 1, 1000000);
        auto dep = depositManager.findByCondition([depId](const shared_ptr<Deposit> &d)
                                                  { return d->getId() == depId; });
        if (!dep)
        {
            throw DepositException("Вклад с указанным ID не найден");
        }

        cout << "\nРедактирование договора " << dep->getContractNumber() << ":\n";
        cout << "1. Изменить процентную ставку\n";
        cout << "2. Изменить статус активности / автопролонгации\n";
        cout << "0. Отмена\n";

        int choice = readInt("Ваш выбор: ", 0, 2);
        if (choice == 1)
        {
            double newRate = readDouble("Введите новую процентную ставку (%): ", 0.0);
            if (newRate > 100.0)
            {
                throw DepositException("Процентная ставка не может превышать 100%");
            }
            dep->setInterestRate(newRate);
            cout << "Процентная ставка успешно обновлена.\n";
        }
        else if (choice == 2)
        {
            if (auto demand = dynamic_pointer_cast<DemandDeposit>(dep))
            {
                int active = readInt("Статус счета (1 - активен, 0 - закрыт): ", 0, 1);
                demand->setActive(active == 1);
                cout << "Статус вклада до востребования обновлен.\n";
            }
            else if (auto savings = dynamic_pointer_cast<SavingsDeposit>(dep))
            {
                int prolong = readInt("Автопролонгация (1 - вкл, 0 - выкл): ", 0, 1);
                savings->setAutoProlongation(prolong == 1);
                cout << "Параметр автопролонгации обновлен.\n";
            }
            else
            {
                cout << "Для срочного вклада данный параметр не применяется.\n";
            }
        }
    }

    void BankApp::closeOrDeleteDeposit()
    {
        viewAllDeposits();
        if (depositManager.size() == 0)
        {
            return;
        }

        int depId = readInt("Введите ID вклада для закрытия и удаления: ", 1, 1000000);
        auto dep = depositManager.findByCondition([depId](const shared_ptr<Deposit> &d)
                                                  { return d->getId() == depId; });
        if (!dep)
        {
            throw DepositException("Вклад с таким ID не найден");
        }

        double finalPayout = dep->getBalance();
        logTransaction(dep, TransactionType::CLOSURE, finalPayout, "Закрытие договора и выдача остатка клиенту");
        depositManager.removeRecord(depId);

        cout << fixed << setprecision(2);
        cout << "\nДоговор " << dep->getContractNumber() << " закрыт. Клиенту выдано: " << finalPayout << " BYN.\n";
    }

    void BankApp::depositOperationsMenu()
    {
        while (true)
        {
            cout << "\n--- ФИНАНСОВЫЕ ОПЕРАЦИИ ПО ВКЛАДАМ ---\n";
            cout << "1. Пополнить вклад\n";
            cout << "2. Частичное снятие средств\n";
            cout << "3. Начислить проценты по конкретному вкладу\n";
            cout << "4. Начислить проценты по всем вкладам\n";
            cout << "5. Просмотреть журнал транзакций\n";
            cout << "0. Назад\n";

            int choice = readInt("Ваш выбор (0-5): ", 0, 5);

            try
            {
                switch (choice)
                {
                case 1:
                    topUpDeposit();
                    pauseConsole();
                    break;
                case 2:
                    withdrawFromDeposit();
                    pauseConsole();
                    break;
                case 3:
                    accrueInterestSingle();
                    pauseConsole();
                    break;
                case 4:
                    accrueInterestAll();
                    pauseConsole();
                    break;
                case 5:
                    viewTransactions();
                    pauseConsole();
                    break;
                case 0:
                    return;
                }
            }
            catch (const BankException &e)
            {
                cerr << "\n[ОШИБКА ОПЕРАЦИИ]: " << e.what() << "\n";
                pauseConsole();
            }
        }
    }

    void BankApp::topUpDeposit()
    {
        viewAllDeposits();
        if (depositManager.size() == 0)
        {
            return;
        }

        int depId = readInt("Введите ID вклада для пополнения: ", 1, 1000000);
        auto dep = depositManager.findByCondition([depId](const shared_ptr<Deposit> &d)
                                                  { return d->getId() == depId; });
        if (!dep)
        {
            throw DepositException("Вклад с таким ID не найден");
        }

        double amount = readDouble("Введите сумму пополнения (BYN): ", 0.01);
        dep->depositFunds(amount);
        logTransaction(dep, TransactionType::TOP_UP, amount, "Пополнение вклада через кассу");
    }

    void BankApp::withdrawFromDeposit()
    {
        viewAllDeposits();
        if (depositManager.size() == 0)
        {
            return;
        }

        int depId = readInt("Введите ID вклада для снятия средств: ", 1, 1000000);
        auto dep = depositManager.findByCondition([depId](const shared_ptr<Deposit> &d)
                                                  { return d->getId() == depId; });
        if (!dep)
        {
            throw DepositException("Вклад с таким ID не найден");
        }

        double amount = readDouble("Введите сумму снятия (BYN): ", 0.01);
        dep->withdrawFunds(amount);
        logTransaction(dep, TransactionType::WITHDRAWAL, amount, "Частичное снятие средств клиентом");
    }

    void BankApp::accrueInterestSingle()
    {
        viewAllDeposits();
        if (depositManager.size() == 0)
        {
            return;
        }

        int depId = readInt("Введите ID вклада для начисления процентов: ", 1, 1000000);
        auto dep = depositManager.findByCondition([depId](const shared_ptr<Deposit> &d)
                                                  { return d->getId() == depId; });
        if (!dep)
        {
            throw DepositException("Вклад с таким ID не найден");
        }

        double before = dep->getBalance();
        dep->calculateInterest();
        double earned = dep->getBalance() - before;
        logTransaction(dep, TransactionType::INTEREST_ACCRUAL, earned, "Начисление процентов по договору");
    }

    void BankApp::accrueInterestAll()
    {
        const auto &deposits = depositManager.getAllRecords();
        if (deposits.empty())
        {
            cout << "\nНет вкладов для начисления процентов.\n";
            return;
        }

        cout << "\n--- МАССОВОЕ НАЧИСЛЕНИЕ ПРОЦЕНТОВ ---\n";
        for (const auto &dep : deposits)
        {
            try
            {
                double before = dep->getBalance();
                dep->calculateInterest();
                double earned = dep->getBalance() - before;
                logTransaction(dep, TransactionType::INTEREST_ACCRUAL, earned, "Плановое начисление процентов");
            }
            catch (const BankException &e)
            {
                cerr << "Пропущен договор " << dep->getContractNumber() << ": " << e.what() << "\n";
            }
        }
    }

    void BankApp::viewTransactions() const
    {
        const auto &transactions = transactionManager.getAllRecords();
        if (transactions.empty())
        {
            cout << "\nЖурнал операций пуст.\n";
            return;
        }

        cout << "\n+------+------------+--------------+------------------------+------------+------------+\n";
        cout << "| ID   | Дата       | Договор      | Тип операции           | Сумма BYN  | Остаток    |\n";
        cout << "+------+------------+--------------+------------------------+------------+------------+\n";
        for (const auto &tr : transactions)
        {
            tr->printTableRow();
        }
        cout << "+------+------------+--------------+------------------------+------------+------------+\n";
        cout << "Всего операций: " << transactions.size() << "\n";
    }

    void BankApp::searchAndFilterDepositsMenu()
    {
        while (true)
        {
            cout << "\n--- ПОИСК, ФИЛЬТРАЦИЯ И СОРТИРОВКА ВКЛАДОВ ---\n";
            cout << "1. Поиск по номеру договора (например, DEP-000001)\n";
            cout << "2. Поиск вкладов по ФИО клиента\n";
            cout << "3. Фильтрация по минимальному балансу\n";
            cout << "4. Фильтрация по типу вклада\n";
            cout << "5. Сортировка списка вкладов\n";
            cout << "0. Назад\n";

            int choice = readInt("Ваш выбор (0-5): ", 0, 5);
            switch (choice)
            {
            case 1:
                searchDepositByContract();
                pauseConsole();
                break;
            case 2:
                searchDepositsByClientName();
                pauseConsole();
                break;
            case 3:
                filterDepositsByMinBalance();
                pauseConsole();
                break;
            case 4:
                filterDepositsByType();
                pauseConsole();
                break;
            case 5:
                sortDeposits();
                pauseConsole();
                break;
            case 0:
                return;
            }
        }
    }

    void BankApp::searchDepositByContract() const
    {
        string contract = readLine("Введите номер договора: ");
        auto found = depositManager.findByCondition([&](const shared_ptr<Deposit> &d)
                                                    { return containsSubstring(d->getContractNumber(), contract); });
        if (!found)
        {
            cout << "\nДоговор \"" << contract << "\" не найден.\n";
            return;
        }
        found->printReport();
    }

    void BankApp::searchDepositsByClientName() const
    {
        string name = readLine("Введите ФИО или часть фамилии клиента: ");
        auto results = depositManager.filterRecords([&](const shared_ptr<Deposit> &d)
                                                    { return containsSubstring(d->getClientName(), name); });
        if (results.empty())
        {
            cout << "\nВклады для клиента \"" << name << "\" не найдены.\n";
            return;
        }
        cout << "\nНайдено вкладов: " << results.size() << "\n";
        for (const auto &dep : results)
        {
            dep->printReport();
        }
    }

    void BankApp::filterDepositsByMinBalance() const
    {
        double minBal = readDouble("Показать вклады с балансом от (BYN): ", 0.0);
        auto results = depositManager.filterRecords([minBal](const shared_ptr<Deposit> &d)
                                                    { return d->getBalance() >= minBal; });
        if (results.empty())
        {
            cout << "\nВклады с балансом от " << minBal << " BYN не найдены.\n";
            return;
        }
        cout << "\nНайдено вкладов: " << results.size() << "\n";
        for (const auto &dep : results)
        {
            dep->printReport();
        }
    }

    void BankApp::filterDepositsByType() const
    {
        cout << "\nВыберите тип вклада:\n";
        cout << "1. Срочные вклады\n";
        cout << "2. Вклады до востребования\n";
        cout << "3. Накопительные вклады\n";
        int kind = readInt("Ваш выбор (1-3): ", 1, 3);

        auto results = depositManager.filterRecords([kind](const shared_ptr<Deposit> &d)
                                                    {
            if (kind == 1) return dynamic_pointer_cast<TermDeposit>(d) != nullptr;
            if (kind == 2) return dynamic_pointer_cast<DemandDeposit>(d) != nullptr;
            return dynamic_pointer_cast<SavingsDeposit>(d) != nullptr; });

        if (results.empty())
        {
            cout << "\nВклады выбранного типа отсутствуют.\n";
            return;
        }
        cout << "\nНайдено вкладов: " << results.size() << "\n";
        for (const auto &dep : results)
        {
            dep->printReport();
        }
    }

    void BankApp::sortDeposits()
    {
        cout << "\nСортировать вклады:\n";
        cout << "1. По балансу (по убыванию)\n";
        cout << "2. По балансу (по возрастанию)\n";
        cout << "3. По процентной ставке (по убыванию)\n";
        cout << "4. По ФИО клиента (А -> Я)\n";
        int mode = readInt("Ваш выбор (1-4): ", 1, 4);

        if (mode == 1)
        {
            depositManager.sortRecords([](const shared_ptr<Deposit> &a, const shared_ptr<Deposit> &b)
                                       { return a->getBalance() > b->getBalance(); });
        }
        else if (mode == 2)
        {
            depositManager.sortRecords([](const shared_ptr<Deposit> &a, const shared_ptr<Deposit> &b)
                                       { return a->getBalance() < b->getBalance(); });
        }
        else if (mode == 3)
        {
            depositManager.sortRecords([](const shared_ptr<Deposit> &a, const shared_ptr<Deposit> &b)
                                       { return a->getInterestRate() > b->getInterestRate(); });
        }
        else
        {
            depositManager.sortRecords([](const shared_ptr<Deposit> &a, const shared_ptr<Deposit> &b)
                                       { return a->getClientName() < b->getClientName(); });
        }

        cout << "\nВклады отсортированы:\n";
        viewAllDeposits();
    }

    void BankApp::reportsMenu() const
    {
        while (true)
        {
            cout << "\n--- ОТЧЕТНОСТЬ И АНАЛИТИКА ---\n";
            cout << "1. Сводная статистика по депозитному портфелю\n";
            cout << "2. Сформировать общий отчет банка в файл (.txt)\n";
            cout << "3. Сформировать выписку по клиенту в файл (.txt)\n";
            cout << "0. Назад\n";

            int choice = readInt("Ваш выбор (0-3): ", 0, 3);
            try
            {
                switch (choice)
                {
                case 1:
                    showPortfolioAnalytics();
                    pauseConsole();
                    break;
                case 2:
                    exportBankReportToFile();
                    pauseConsole();
                    break;
                case 3:
                    exportClientStatementToFile();
                    pauseConsole();
                    break;
                case 0:
                    return;
                }
            }
            catch (const BankException &e)
            {
                cerr << "\n[ОШИБКА ОТЧЕТА]: " << e.what() << "\n";
                pauseConsole();
            }
        }
    }

    void BankApp::showPortfolioAnalytics() const
    {
        const auto &deposits = depositManager.getAllRecords();
        double totalSum = 0.0;
        double avgRate = 0.0;
        int termCnt = 0, demandCnt = 0, savingsCnt = 0;

        for (const auto &d : deposits)
        {
            totalSum += d->getBalance();
            avgRate += d->getInterestRate();
            if (dynamic_pointer_cast<TermDeposit>(d))
                termCnt++;
            else if (dynamic_pointer_cast<DemandDeposit>(d))
                demandCnt++;
            else if (dynamic_pointer_cast<SavingsDeposit>(d))
                savingsCnt++;
        }

        if (!deposits.empty())
        {
            avgRate /= static_cast<double>(deposits.size());
        }

        cout << fixed << setprecision(2);
        cout << "\n=============================================\n";
        cout << "   АНАЛИТИКА ДЕПОЗИТНОГО ПОРТФЕЛЯ БАНКА\n";
        cout << "=============================================\n";
        cout << "Клиентов в системе:            " << clientManager.size() << "\n";
        cout << "Активных вкладов:              " << deposits.size() << "\n";
        cout << "  - Срочных:                   " << termCnt << "\n";
        cout << "  - До востребования:          " << demandCnt << "\n";
        cout << "  - Накопительных:             " << savingsCnt << "\n";
        cout << "Суммарный объем средств:       " << totalSum << " BYN\n";
        cout << "Средняя процентная ставка:     " << avgRate << "%\n";
        cout << "Зарегистрировано транзакций:   " << transactionManager.size() << "\n";
        cout << "Всего счетов за сессию (stat): " << Account::getTotalAccounts() << "\n";
        cout << "=============================================\n";
    }

    void BankApp::exportBankReportToFile() const
    {
        string filename = readLine("Введите имя файла отчета (Enter для bank_report.txt): ");
        if (filename.empty())
        {
            filename = "bank_report.txt";
        }
        FileStorage::getInstance().exportFullReport(filename, clientManager, depositManager, transactionManager);
        cout << "\nСводный отчет успешно сохранен в файл: " << filename << "\n";
    }

    void BankApp::exportClientStatementToFile() const
    {
        viewAllClients();
        if (clientManager.size() == 0)
        {
            return;
        }

        int clientId = readInt("Введите ID клиента для формирования выписки: ", 1, 1000000);
        auto client = clientManager.findByCondition([clientId](const shared_ptr<Client> &c)
                                                    { return c->getId() == clientId; });
        if (!client)
        {
            throw ClientException("Клиент с таким ID не найден");
        }

        auto clientDeposits = depositManager.filterRecords([&](const shared_ptr<Deposit> &d)
                                                           { return d->getClientName() == client->getFullName(); });

        auto clientTransactions = transactionManager.filterRecords([&](const shared_ptr<Transaction> &tr)
                                                                   {
            for (const auto &d : clientDeposits)
            {
                if (d->getId() == tr->getAccountId())
                {
                    return true;
                }
            }
            return false; });

        string defaultName = "statement_client_" + to_string(client->getId()) + ".txt";
        string filename = readLine("Введите имя файла (Enter для " + defaultName + "): ");
        if (filename.empty())
        {
            filename = defaultName;
        }

        FileStorage::getInstance().exportClientStatement(filename, *client, clientDeposits, clientTransactions);
        cout << "\nВыписка клиента успешно сохранена в файл: " << filename << "\n";
    }

    void BankApp::manageUsersMenu()
    {
        while (true)
        {
            cout << "\n--- УПРАВЛЕНИЕ ПОЛЬЗОВАТЕЛЯМИ СИСТЕМЫ ---\n";
            cout << "1. Список пользователей\n";
            cout << "2. Добавить нового пользователя\n";
            cout << "3. Удалить пользователя\n";
            cout << "0. Назад\n";

            int choice = readInt("Ваш выбор (0-3): ", 0, 3);
            try
            {
                switch (choice)
                {
                case 1:
                    viewAllUsers();
                    pauseConsole();
                    break;
                case 2:
                    addUser();
                    pauseConsole();
                    break;
                case 3:
                    deleteUser();
                    pauseConsole();
                    break;
                case 0:
                    return;
                }
            }
            catch (const BankException &e)
            {
                cerr << "\n[ОШИБКА]: " << e.what() << "\n";
                pauseConsole();
            }
        }
    }

    void BankApp::viewAllUsers() const
    {
        const auto &users = userManager.getAllRecords();
        cout << "\n+----------------------+--------------------+\n";
        cout << "| Логин                | Роль               |\n";
        cout << "+----------------------+--------------------+\n";
        for (const auto &u : users)
        {
            string role = dynamic_pointer_cast<Admin>(u) ? "Администратор" : "Операционист";
            string padRole = role + string(role == "Администратор" ? 5 : 6, ' ');
            cout << "| " << left << setw(20) << u->getLogin().substr(0, 20)
                 << " | " << padRole << " |\n";
        }
        cout << "+----------------------+--------------------+\n";
    }

    void BankApp::addUser()
    {
        string login = readLine("Введите новый логин: ");
        if (login.size() < 3)
        {
            throw AuthException("Логин должен содержать минимум 3 символа");
        }

        auto exists = userManager.findByCondition([&](const shared_ptr<User> &u)
                                                  { return u->getLogin() == login; });
        if (exists)
        {
            throw AuthException("Пользователь с таким логином уже существует");
        }

        string password = readLine("Введите пароль (минимум 4 символа): ");
        if (password.size() < 4)
        {
            throw AuthException("Пароль слишком короткий");
        }

        int role = readInt("Роль пользователя (1 - Администратор, 2 - Операционист): ", 1, 2);
        if (role == 1)
        {
            userManager.addRecord(make_shared<Admin>(login, password));
        }
        else
        {
            userManager.addRecord(make_shared<Operator>(login, password));
        }

        cout << "\nПользователь \"" << login << "\" успешно добавлен.\n";
    }

    void BankApp::deleteUser()
    {
        viewAllUsers();
        string login = readLine("Введите логин пользователя для удаления: ");

        if (login == currentUser->getLogin())
        {
            throw AuthException("Нельзя удалить текущую активную учетную запись");
        }

        auto target = userManager.findByCondition([&](const shared_ptr<User> &u)
                                                  { return u->getLogin() == login; });
        if (!target)
        {
            throw AuthException("Пользователь с таким логином не найден");
        }

        auto remaining = userManager.filterRecords([&](const shared_ptr<User> &u)
                                                   { return u->getLogin() != login; });
        userManager.clear();
        for (const auto &u : remaining)
        {
            userManager.addRecord(u);
        }

        cout << "\nПользователь \"" << login << "\" успешно удален.\n";
    }
}
#include <iostream>
#include <string>
#include <memory>
#include "BankClasses.h" // Подключаем ваш заголовочный файл

using namespace BankSystem;
using namespace std;

// Функция для стартового заполнения базы
void initializeData(DataManager<User> &users, DataManager<Deposit> &deposits)
{
    // Создаем пользователей: Администратора и Оператора
    users.addRecord(std::make_shared<Admin>("admin", "12345"));
    users.addRecord(std::make_shared<Operator>("oper", "0000"));

    // Создаем тестовый вклад для проверки
    deposits.addRecord(std::make_shared<TermDeposit>(1, "Иванов И.И.", 1500.0, 5.5, 12, "Пенсионный"));
}

int main()
{
    setlocale(LC_ALL, "Russian");

    DataManager<User> userManager;
    DataManager<Deposit> depositManager;

    try
    {
        initializeData(userManager, depositManager);

        cout << "==========================================\n";
        cout << " СИСТЕМА УПРАВЛЕНИЯ БАНКОВСКИМИ ВКЛАДАМИ\n";
        cout << "==========================================\n";

        string inputLogin, inputPass;
        shared_ptr<User> currentUser = nullptr;

        // 1. Блок авторизации
        while (!currentUser)
        {
            cout << "\nВведите логин: ";
            cin >> inputLogin;
            cout << "Введите пароль: ";
            cin >> inputPass;

            auto foundUser = userManager.findByCondition([&](const shared_ptr<User> &u)
                                                         { return u->getLogin() == inputLogin; });

            if (foundUser && authenticate(*foundUser, inputPass))
            {
                currentUser = foundUser;
                cout << "\nАвторизация успешна! Добро пожаловать, " << currentUser->getLogin() << ".\n";
            }
            else
            {
                cout << "Ошибка: неверный логин или пароль. Повторите попытку.\n";
            }
        }

        // 2. Главный цикл программы
        int choice = 0;
        while (choice != 4)
        {
            currentUser->showMenu();

            cout << "Выберите действие (1-4): ";
            if (!(cin >> choice))
            {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "Неверный ввод. Пожалуйста, введите число.\n";
                continue;
            }

            // 3. Обработчик команд
            switch (choice)
            {
            case 1:
                cout << "\n[ Выбрана Опция 1 ] - Логика в разработке...\n";
                break;
            case 2:
                cout << "\n[ Выбрана Опция 2 ] - Логика в разработке...\n";
                break;
            case 3:
                cout << "\n[ Выбрана Опция 3 ] - Логика в разработке...\n";
                break;
            case 4:
                cout << "\nЗавершение работы сессии. Выход из программы...\n";
                break;
            default:
                cout << "\nНеизвестная команда. Выберите пункт меню от 1 до 4.\n";
            }
        }
    }
    catch (const BankException &e)
    {
        cerr << "\n[СИСТЕМНАЯ ОШИБКА]: " << e.what() << endl;
    }
    catch (const std::exception &e)
    {
        cerr << "\n[КРИТИЧЕСКАЯ ОШИБКА]: " << e.what() << endl;
    }

    return 0;
}
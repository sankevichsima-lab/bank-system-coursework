#include <iostream>
#include <clocale>
#include "BankApp.h"

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;
using namespace BankSystem;

int main()
{
#ifdef _WIN32
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
#endif
    setlocale(LC_ALL, "Russian");

    try
    {
        BankApp app;
        app.run();
    }
    catch (const AuthException &e)
    {
        cerr << "\n[ОШИБКА АВТОРИЗАЦИИ]: " << e.what() << endl;
        return 1;
    }
    catch (const FileException &e)
    {
        cerr << "\n[ОШИБКА ФАЙЛОВОЙ СИСТЕМЫ]: " << e.what() << endl;
        return 2;
    }
    catch (const BankException &e)
    {
        cerr << "\n[СИСТЕМНАЯ ОШИБКА]: " << e.what() << endl;
        return 3;
    }
    catch (const exception &e)
    {
        cerr << "\n[КРИТИЧЕСКАЯ ОШИБКА]: " << e.what() << endl;
        return 4;
    }

    return 0;
}
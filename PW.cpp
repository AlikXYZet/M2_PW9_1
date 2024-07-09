#include <iostream>
#include <string>

#include <functional>
#include <vector>
#include <algorithm>

#include <chrono>   // Задержка времени

#include <thread>   // Управление потоками
#include <mutex>    // Очерёдность потоков

using namespace std;



/* ---   Инициализация глобальных переменных   --- */

/*Блокираторы для соблюдения очереди:*/
// Для сохранения в базу данных
mutex mtx_database;
//---------------------------------------------------------------------



// Структура для хранения и вывода данных
struct Data
{
    string Nickname;
    uint8_t Age;
    uint8_t Rating;
    size_t ID = 0;
};
//---------------------------------------------------------------------



/* ---   Функции   --- */

// Рандомный Nickname
string rand_Nickname()
{
    string out_str;

    for (size_t i = 0; i < rand() % 7 + 3; ++i)
    {
        int randomChar = rand() % (26 + 26 + 10);

        if (randomChar < 26)
            out_str += 'a' + randomChar;
        else if (randomChar < 26 + 26)
            out_str += 'A' + randomChar - 26;
        else
            out_str += '0' + randomChar - 26 - 26;
    }

    return out_str;
}

// Функция вывода данных
void DataPrint(vector<Data>& in_database, bool& bFlag, const function<const bool(Data&, Data&)>& b_predicate)
{
    // Вывод "шапки" в консоль
    cout.width(4);
    cout << "#";

    cout.width(12);
    cout << "Nickname";

    cout.width(4);
    cout << "Age";

    cout.width(7);
    cout << "Rating";

    cout.width(20);
    cout << "ID" << endl;

    while (true)
    {
        if (bFlag)
        {
            printf("\x1B[%d;%df", 7, 0);

            int i = 0;

            mtx_database.lock();

            sort(in_database.begin(), in_database.end(), b_predicate);

            for (const Data& data : in_database)
            {
                cout.width(4);
                cout << ++i;

                cout.width(12);
                cout << data.Nickname;

                cout.width(4);
                cout << to_string(data.Age);

                cout.width(7);
                cout << to_string(data.Rating);

                cout.width(20);
                cout << to_string(data.ID) << endl;
            }

            mtx_database.unlock();

            bFlag = false;
        }
    }
}

// Функция сбора и вывода данных
void DataCollectionAndPrinting(unsigned int in_time, Data in_data, bool& bFlag, vector<Data>& out_database)
{
    // Имитация задержки получения данных
    this_thread::sleep_for(chrono::milliseconds(in_time));

    // Дополнительные данные ID
    in_data.ID = hash<thread::id>{}(this_thread::get_id());

    // Генерация данных и сохранение их в массивс соблюдением порядка
    // для избежания ошибок в "std::vector<Data>"
    mtx_database.lock();
    out_database.push_back(in_data);
    mtx_database.unlock();

    bFlag = true;
}
//---------------------------------------------------------------------



int main()
{
    /* ---   Предварительное   --- */

    // Локализация
    setlocale(LC_ALL, "Russian");
    // <iomanip>: Выравнивание по левому краю
    cout.setf(ios::left);
    // Использовать текущее время в качестве начального значения для генератора псевдослучайных чисел
    srand(time(0));
    //----------------------------------------



    /* ---   Инициализация переменных   --- */

    // База хранения данных
    vector<Data> database;

    // Количество потоков
    const unsigned int numberThreads = 20;

    // Массив потоков
    thread Threads[numberThreads];

    // Случайное время задержки для каждого потока (мс)
    size_t timeThreads = 0;

    // Флаг вывода обновлённых данных
    bool bGoPrint = false;

    // Критерий сортировки с вариантом по умолчанию
    function<const bool(Data&, Data&)> l_sort = [](Data& first, Data& second)
        {
            return first.Nickname < second.Nickname;
        };

    // Поток вывода данных
    thread printThread;

    // Переменная начала отсчёта времени
    chrono::steady_clock::time_point start;
    //----------------------------------------



    /* ---   Выбор сортировки   --- */

    // Переменная выбора
    int switch_on = 0;

    cout
        << 1 << " - Nickname\n"
        << 2 << " - Age\n"
        << 3 << " - Rating\n"
        << 4 << " - ID\n"
        << "Выбирите столбец для сортировки (1 - по умолчанию): ";
    cin >> switch_on;

    // Запуск потока "Потребителя"
    printThread = thread(DataPrint, ref(database), ref(bGoPrint), ref(l_sort));

    // Начало отсчёта времени
    start = start = chrono::steady_clock::now();


    switch (switch_on)
    {
    case 1:
        l_sort = [](Data& first, Data& second)
            {
                return first.Nickname < second.Nickname;
            };
        break;
    case 2:
        l_sort = [](Data& first, Data& second)
            {
                return first.Age < second.Age;
            };
        break;
    case 3:
        l_sort = [](Data& first, Data& second)
            {
                return first.Rating < second.Rating;
            };
        break;
    case 4:
        l_sort = [](Data& first, Data& second)
            {
                return first.ID < second.ID;
            };
        break;
    default:
        break;
    }
    //----------------------------------------



    // Запуск каждого потока "Продюсера"
    for (size_t i = 0; i < numberThreads; ++i)
    {
        // Вариант с временем в диапазоне [0, 10000) мс
        timeThreads = (float)rand() / RAND_MAX * 10000;

        // PS: Используем данный вариант для более показательной
        // имитации наложения вызова "DataPrint()" в потоках
        // [100, 1000] с шагом 100
        //timeThreads = (rand() % 10) * 100 + 100;

        // Генерация данных с рандомными значениями
        Data newData
        {
            rand_Nickname(),
            (uint8_t)(rand() % 20 + 18),
            (uint8_t)(rand() % 7 + 4)
        };

        // !!! Передавать "timeThreads" и "newData" не по ссылке !!!
        Threads[i] = thread(DataCollectionAndPrinting, timeThreads, newData, ref(bGoPrint), ref(database));
    }



    // Ожидание окончания каждого потока
    for (size_t i = 0; i < numberThreads; ++i)
    {
        Threads[i].join();
    }

    // Ожидание завершения вывода
    while (bGoPrint) {}

    // Принудительное завершение потока
    printThread.detach();

    cout
        << "===============================================\n"
        << "Затраченное время: "
        << float(chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start).count()) / 1000 << " s\a"
        << endl;

    system("pause");

    return 0;
}
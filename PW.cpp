#include <iostream>
#include <string>

#include <functional>
#include <vector>

#include <chrono>   // Задержка времени

#include <thread>   // Управление потоками
#include <mutex>    // Очерёдность потоков

using namespace std;



/* ---   Инициализация глобальных переменных   --- */

/*Блокираторы для соблюдения очереди:*/
// Для сохранения в базу данных
mutex mtx_database;

// 
mutex mtx_test;
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
void DataPrint(vector<Data>& in_database, bool& bFlag, const function<const bool(Data&, Data&)>& b_redicate)
{
    // Вывод "шапки" в консоль
    cout.width(12);
    cout << "Nickname"
        << "Age\t"
        << "Rating\t"
        << "ID"
        << endl;

    unsigned int num_line = 1;
    Data curr_data;

    while (true)
    {
        if (bFlag)
        {


            mtx_database.lock();
            curr_data = in_database.back();
            mtx_database.unlock();

            cout.width(12);

            cout
                << curr_data.Nickname
                << to_string(curr_data.Age) << '\t'
                << to_string(curr_data.Rating) << '\t'
                << to_string(curr_data.ID) << "\t\t"
                << " "
                << endl;

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

    // Критерий сортировки
    auto lambda = [](Data& first, Data& second) -> bool
        {
            return first.Nickname < second.Nickname;
        };

    // Поток вывода данных
    thread printThread(DataPrint, ref(database), ref(bGoPrint), ref(lambda));

    // Переменная начала отсчёта времени
    chrono::steady_clock::time_point start = chrono::steady_clock::now();
    //----------------------------------------



    // Запуск каждого потока
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

    // 
    this_thread::sleep_for(chrono::milliseconds(100));

    cout
        << "===============================================\n"
        << "Затраченное время: "
        << float(chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start).count()) / 1000 << " s\a"
        << endl;

    printThread.detach();

    return 0;
}
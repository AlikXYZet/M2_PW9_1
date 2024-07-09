#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>

using namespace std;



/* ---   Инициализация глобальных переменных   --- */
// Блокиратор для соблюдения очереди вывода данных
mutex cyclicMutex;
//---------------------------------------------------

// Функция вывода данных
void LinePrint()
{
	// Вывод в консоль с соблюдением очереди
	cyclicMutex.lock();
	cout << "LinePrint ID: " << this_thread::get_id() << endl;
	cyclicMutex.unlock();
}

// Функция сбора и вывода данных
void DataCollectionAndPrinting(unsigned int time)
{
	// Имитация задержки получения данных
	cout
		<< "time = " << time
		<< " ms for ID: " << this_thread::get_id()
		<< endl;

	this_thread::sleep_for(chrono::milliseconds(time));

	// Вывод данных
	LinePrint();
}



int main()
{
	/* ---   Предварительное   --- */
	// Локализация
	setlocale(LC_ALL, "Russian");
	// Использовать текущее время в качестве начального значения для генератора псевдослучайных чисел
	srand(time(0));
	//----------------------------------------



	/* ---   Инициализация переменных   --- */
	// Количество потоков
	const unsigned int numberThreads = 20;

	// Массив потоков
	thread Threads[numberThreads];

	// Случайное время задержки для каждого потока (мс)
	unsigned int timeThreads = 0;
	//----------------------------------------



	// Запуск каждого потока
	for (unsigned int i = 0; i < numberThreads; ++i)
	{
		timeThreads = (float)rand() / RAND_MAX * 1000;

		// !!! Передавать "timeThreads" не по ссылке !!!
		Threads[i] = thread(DataCollectionAndPrinting, timeThreads);
	}



	// Ожидание окончания каждого потока
	for (size_t i = 0; i < numberThreads; i++)
	{
		Threads[i].join();
	}

	return 0;
}
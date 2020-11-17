// Threads.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
#include <pthread.h>
#include <string>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

/// <summary>
/// Структура данных для потоков
/// </summary>
struct Package {
	int* array;  // Указатель на начало области обработки
	unsigned int threadNum;  // Номер потока
	unsigned int i; //Начало последовательности
	unsigned int j; //Конец последовательности
	unsigned int max = 1; //Размер последовательности
	unsigned int start; //Начало сектора потока
	unsigned int finish; //Конец сектора
	unsigned int number; //Число элементов в массиве
};

/// <summary>
/// Функция для подсчёта максимальной длины последовательности на промежутке потоком
/// </summary>
/// <param name="param">Структура данных для работы потока</param>
/// <returns>nullptr</returns>
void* func(void* param); 

/// <summary>
/// Функция для вывода результата в файл
/// </summary>
/// <param name="threadNumber">Количество потоков</param>
/// <param name="pack">Структура данных</param>
/// <param name="answer">Путь к файлу с ответом</param>
void Write_answer(unsigned int threadNumber, Package* pack, const std::string& answer); 

/// <summary>
/// Передача параметров в структуру данных
/// </summary>
/// <param name="pack">Структура данных</param>
/// <param name="q">Номер потока</param>
/// <param name="num">Число элементов в массиве</param>
/// <param name="threadNumber">Число птоков</param>
/// <param name="array">Маччив для изучения</param>
void Pack_parametrs(Package* pack, unsigned int q, unsigned int num, unsigned int threadNumber, int* array);

/// <summary>
/// Основная программа
/// </summary>
/// <param name="args">Число аргументов командной строки</param>
/// <param name="argv">Массив аргументов командной строки</param>
/// <returns>Код ошибки</returns>
int main(int args, char* argv[])
{
	const string test = argv[1]; //путь до теста
	const string answer = argv[2]; //путь до ответа
	const string threds = argv[3]; //число потоков
	//создание потока для чтения
	ifstream fin(test);
	if (!fin.is_open()) {
		throw runtime_error("IO Exception");
	}
	unsigned int threadNumber = stoi(threds);
	int s;
	unsigned int num;
	fin >> num; //число элементов в массиве
	int* array = new int[num];
	unsigned int w = 0;
	//Чтение элементов массива и запись их в массив
	while (!fin.eof() && w < num)
	{
		fin >> s;
		array[w] = s;
		w++;
	}
	fin.close(); //закрытие потока
	pthread_t* thread = new pthread_t[threadNumber];
	Package* pack = new Package[threadNumber];
	//Создание потоков
	for (unsigned int q = 0; q < threadNumber; q++) {
		Pack_parametrs(pack, q, num, threadNumber, array);
		pthread_create(&thread[q], nullptr, func, (void*)&pack[q]);
	}
	pthread_join(*thread, nullptr); //ожидание завершения всех потоков
	Write_answer(threadNumber, pack, answer); //запись ответа
	//очистка памяти
	delete[] thread; 
	delete[] array;
	delete[] pack;
	return 0;
}

/// <summary>
/// Передача параметров в структуру данных
/// </summary>
/// <param name="pack">Структура данных</param>
/// <param name="q">Номер потока</param>
/// <param name="num">Число элементов в массиве</param>
/// <param name="threadNumber">Число птоков</param>
/// <param name="array">Маччив для изучения</param>
void Pack_parametrs(Package* pack, unsigned int q, unsigned int num, unsigned int threadNumber, int* array)
{
	//Если число потоков меньше числа элементов массива 
	if (threadNumber <= num)
	{
		pack[q].start = (num / threadNumber) * (q); //начать с ячейки массива равной данному номеру
		pack[q].finish = (num / threadNumber) * (q + 1); //закончить соответственно на начале для следующего потока
	}
	//Данная ситуация невозможно при корректном использовании программы, так как создание большого числа потоков 
	//только до определённого момента увеличивает скорость работы прогриаммы
	//Так как на вход подаётся больше 1000 чисел, то создавать 1001 поток не имеет смысла
	else //Если число потоков больше числа элементов массива, то, наоборот, распределять относительно числа потоков
	{
		pack[q].start = (threadNumber / num) * (q) > num ? num : (threadNumber / num) * (q);
		pack[q].finish = (threadNumber / num) * (q + 1) > num ? num : (threadNumber / num) * (q);
	}
	//Запись парамметров в структуру данных
	pack[q].threadNum = q;
	pack[q].array = array;
	pack[q].number = num;
}

/// <summary>
/// Функция для подсчёта максимальной длины последовательности на промежутке потоком
/// </summary>
/// <param name="param">Структура данных для работы потока</param>
/// <returns>nullptr</returns>
void* func(void* param)
{
	//Восстанавливаем структуру
	Package* p = (Package*)param;
	//Начинаем идти с start
	unsigned int i = p->start;
	//Записываем первый для нас элемент массива
	int c = p->array[i];
	//Будем считать, что один элемент является последовательностью
	p->i = i;
	p->j=i;
	unsigned int max = 0;
	unsigned int i2 = 0;
	//Идём до конца нашего промежутка
	//Или пока наша последовательность не перестанет возрастать
	while (i <= (p->finish) || ((i + 1 < p->number) && (p->array[i + 1] > c)))
	{
		//"Обнуляем" значения
		max = 1;
		i2 = i;
		c = p->array[i];
		//Идём пока наша последовательность возрастает
		while ((i + 1 < p->number) && (p->array[i + 1] > c))
		{
			max++;
			i++;
			c = p->array[i];
		}
		//Записываем результат
		if (max > p->max)
		{
			p->i = i2 == p->number ? i2 - 1 : i2;
			p->j = i == p->number ? i - 1 : i;
			p->max = max;
		}
		i++;
	}
	return nullptr;
}


/// <summary>
/// Функция для вывода результата в файл
/// </summary>
/// <param name="threadNumber">Количество потоков</param>
/// <param name="pack">Структура данных</param>
/// <param name="answer">Путь к файлу с ответом</param>
void Write_answer(unsigned int threadNumber, Package* pack, const std::string& answer)
{
	unsigned int i = 0, j = 0, max = 0;
	//Проходимся в цикле по результатам потоков
	for (unsigned int q = 0; q < threadNumber; q++)
	{
		if (pack[q].max > max)
		{
			max = pack[q].max;
			i = pack[q].i;
			j = pack[q].j;
		}
	}
	//Записываем наилучший результат в файл
	fstream out(answer, ios::out);
	out << "i = ";
	out << i;
	out << "\n";
	out << "j = ";
	out << j;
	out << "\n";
	out << "length : ";
	out << max;
	out.close();
}

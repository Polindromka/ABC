#include <iostream>
#include <omp.h>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

/// <summary>
/// Структура данных для потоков
/// </summary>
struct Package {
	int i = 0; //Начало последовательности
	int j = 0; //Конец последовательности
	int t = 0; //Число подряд идущих элементов последовательности
	int last = 0; //Индекс последнего элемента, рассматриваемого потоком
};

/// <summary>
/// Функция, которая работает с потоками. Ищет наибольшую последовательность
/// </summary>
/// <param name="threadsArray">Массив данных по потокам</param>
/// <param name="i">Номер элемента массива</param>
/// <param name="num">Число элементов в массиве</param>
/// <param name="array">Массив элементов</param>
void ThreadsFunction(vector<Package>& threadsArray, int i, int num, int* array);

/// <summary>
/// После отработки потока мы не проверяем, достигли ли максимальной последовательности, поэтому необходимо это сделать 
/// </summary>
/// <param name="threadsArray">Массив потоков</param>
/// <param name="i">Номер потока</param>
void LastElement(vector<Package>& threadsArray, int i);

/// <summary>
/// После отработки потоков, необходимо найти наибольшую длину последовательности, так как она,
/// может быть разбита междуу потоками
/// </summary>
/// <param name="threadsArray">Массив потоков</param>
/// <param name="i">Номер потока</param>
/// <param name="finish">Конец наибольшей последовательности</param>
/// <param name="start">Начало наибольшей последовательности</param>
/// <param name="array">Массив чисел</param>
void MaxLength(vector<Package>& threadsArray, int& i, int& finish, int& start, int* array);

/// <summary>
/// Функция записывает элементы в файл
/// </summary>
/// <param name="answer">Путть к файлу с ответами</param>
/// <param name="start">Индекс начала максимальной последовательности</param>
/// <param name="finish">Индекс окончания максимальной последовательности</param>
void WriteToFile(const string& answer, int start, int finish);

/// <summary>
/// Основная функция
/// </summary>
/// <param name="args">Число аргументов командной строки</param>
/// <param name="argv">Аргументы командной строки</param>
/// <returns></returns>
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
	int threadNumber = stoi(threds);
	int s;
	int num;
	fin >> num; //число элементов в массиве
	int* array = new int[num];
	int w = 0;
	//Чтение элементов массива и запись их в массив
	while (!fin.eof() && w < num)
	{
		fin >> s;
		array[w] = s;
		w++;
	}
	fin.close(); //закрытие потока
	vector<Package> threadsArray(threadNumber);
	//Открытие параллельных потоков, с помощью schedule(static) разделены равномерно между потоками. 
#pragma omp parallel for  schedule(static) num_threads(threadNumber)
	for (int i = 0; i < num; i++)
		ThreadsFunction(threadsArray, i, num, array);
	//Дожидаемся окончания выполнения всех потоков, преде чем приступим к их анализу
#pragma omp barrier
	{
		int  start = 0;
		int finish = 0;
		for (int i = 0; i < threadNumber; i++)
			LastElement(threadsArray, i);
		for (int i = 0; i < threadNumber; i++)
			MaxLength(threadsArray, i, finish, start, array);
		WriteToFile(answer, start, finish);
		delete[] array;
	}
	return 0;
}

/// <summary>
/// Функция записывает элементы в файл
/// </summary>
/// <param name="answer">Путть к файлу с ответами</param>
/// <param name="start">Индекс начала максимальной последовательности</param>
/// <param name="finish">Индекс окончания максимальной последовательности</param>
void WriteToFile(const string& answer, int start, int finish)
{
	fstream out(answer, ios::out);
	out << "i = ";
	out << start;
	out << "\n";
	out << "j = ";
	out << finish;
	out << "\n";
	out << "length: ";
	out << to_string(finish - start + 1);
	out.close();
}

/// <summary>
/// После отработки потоков, необходимо найти наибольшую длину последовательности, так как она,
/// может быть разбита междуу потоками
/// </summary>
/// <param name="threadsArray">Массив потоков</param>
/// <param name="i">Номер потока</param>
/// <param name="finish">Конец наибольшей последовательности</param>
/// <param name="start">Начало наибольшей последовательности</param>
/// <param name="array">Массив чисел</param>
void MaxLength(vector<Package>& threadsArray, int& i, int& finish, int& start, int* array)
{
	//Проверяем, длинее ли сохранённая последовательность, той, что находится в потоке
	//Либо нет ли продолжения нашей последовательности в следующем потоке,
	//Но при условии, что следующий поток не содержит новую последовательность,
	//То есть, пусть дано |1 2 3|-3 -2 -1| и наши потоки данным образом разделили массив
	//Тогда, с одной стороны, окончание нашей последовательности, является началом следующей
	//Но это две разные последовательности и их нельзя объединять
	bool flagNext = i + 1 < threadsArray.size() &&
		((threadsArray[i + 1].i == threadsArray[i].j &&
			array[threadsArray[i + 1].i] > array[threadsArray[i].j - 1])
			|| (threadsArray[i + 1].i == threadsArray[i].j + 1
				&& array[threadsArray[i + 1].i] > array[threadsArray[i].j]));

	if ((threadsArray[i].j - threadsArray[i].i > finish - start) || flagNext)
	{
		//Следующая последовательность, не является началом предыдущей
		if (!flagNext)
		{
			start = threadsArray[i].i;
			finish = threadsArray[i].j;
		}
	//Следующая последовательность является началом предыдущей
		else
		{
			int start_0 = threadsArray[i].i;
			int finish_0 = threadsArray[i].j;
			//Пока следующая последовательность является началом предыдущей,
			//Мы удлиняем нашу возрастающую последовательность
			while ((i + 1 < threadsArray.size() && threadsArray[i].t != 0 &&
				((threadsArray[i + 1].i == threadsArray[i].j &&
					array[threadsArray[i + 1].i] > array[threadsArray[i].j - 1])
					|| (threadsArray[i + 1].i == threadsArray[i].j + 1
						&& array[threadsArray[i + 1].i] > array[threadsArray[i].j]))))
			{
				finish_0 = threadsArray[i + 1].j;
				i++;
			}
			//Проверяем, стала ли длина нашей новой последовательности больше, чем сохранённая
			if (finish_0 - start_0 > finish - start)
			{
				start = start_0;
				finish = finish_0;
			}
		}
	}
}

/// <summary>
/// После отработки потока мы не проверяем, достигли ли максимальной последовательности, поэтому необходимо это сделать 
/// </summary>
/// <param name="threadsArray">Массив потоков</param>
/// <param name="i">Номер потока</param>
void LastElement(vector<Package>& threadsArray, int i)
{
	//Если на последней итерации потока, мы получили наилучший результат, но не сохранили его,
	//то сейчас перезаписываем значения
	if (threadsArray[i].t > (threadsArray[i].j - threadsArray[i].i))
	{
		threadsArray[i].j = threadsArray[i].last;
		threadsArray[i].i = threadsArray[i].last - threadsArray[i].t;
	}
}

/// <summary>
/// Функция, которая работает с потоками. Ищет наибольшую последовательность
/// </summary>
/// <param name="threadsArray">Массив данных по потокам</param>
/// <param name="i">Номер элемента массива</param>
/// <param name="num">Число элементов в массиве</param>
/// <param name="array">Массив элементов</param>
void ThreadsFunction(vector<Package>& threadsArray, int i, int num, int* array)
{
	//Записываем номер элемента, с которого начинает работу наш поток
	if (threadsArray[omp_get_thread_num()].i == 0)
	{
		threadsArray[omp_get_thread_num()].i = i;
		threadsArray[omp_get_thread_num()].last = i;
	}
	//Если следующий жлемент больше предыдущего, то увеличиваем счётчик последовательности
	if (i + 1 < num && array[i + 1] > array[i])
		threadsArray[omp_get_thread_num()].t++;
	//Иначе сохраняем полученный результат
	else
	{
		//Если новое значение лучше, чем сохранённое, то перезаписываем
		if (threadsArray[omp_get_thread_num()].t > (threadsArray[omp_get_thread_num()].j - threadsArray[omp_get_thread_num()].i))
		{
			threadsArray[omp_get_thread_num()].j = i; //Здесь, i+1 уже не удовлетворяе
			threadsArray[omp_get_thread_num()].i = i - threadsArray[omp_get_thread_num()].t;
		}
		//В любом случае обнуляем счётчик, так как наша последовательность закончилась
		threadsArray[omp_get_thread_num()].t = 0;
	}
	//Сохраняем значение последнего просмотренного элемента, удовлетворяющего поиску
	threadsArray[omp_get_thread_num()].last = i+1; //Здесь i+1 ещё может удовлетворять, так как
	//Если бы это был последний (i - последний), то счётчик обнулился и мы им не воспользуемся
	//В любом случае, если бы он нас не устраивал, то счётчик был бы равен нулю
	//А если это не так, то этот жлемент нас устраивает
}


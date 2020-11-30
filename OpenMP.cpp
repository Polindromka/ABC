#include <iostream>
#include <omp.h>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>

using namespace std;

/// <summary>
/// Структура данных для потоков
/// </summary>
struct Package {
	int i = 0; //Начало последовательности
	int j = 0; //Конец последовательности
	int t = 0; //Конец последовательности
	int last = 0; //Конец последовательности
};

bool cmp(Package& a, Package& b) {
	return a.i < b.i;
}

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
	int i;
#pragma omp parallel for  schedule(static) num_threads(threadNumber) private(i) 
	for (i = 0; i < num; i++)
	{
		auto q = omp_get_thread_num();
		if (threadsArray[omp_get_thread_num()].i == 0)
		{
			threadsArray[omp_get_thread_num()].i = i;
			threadsArray[omp_get_thread_num()].last = i;
		}
		if (i + 1 < num && array[i + 1] > array[i])
			threadsArray[q].t++;
		else
		{
			if (threadsArray[q].t > (threadsArray[q].j - threadsArray[q].i))
			{
				threadsArray[q].j = i;
				threadsArray[q].i = i - threadsArray[q].t;
			}
			threadsArray[q].t = 0;
		}
		threadsArray[q].last = i + 1;
	}
#pragma omp barrier
	{
		int  start = 0;
		int finish = 0;
		for (int i = 0; i < threadNumber; i++)
		{
			if (threadsArray[i].t > (threadsArray[i].j - threadsArray[i].i))
			{
				threadsArray[i].j = threadsArray[i].last;
				threadsArray[i].i = threadsArray[i].last - threadsArray[i].t;
			}
		}
		for (int i = 0; i < threadNumber; i++)
		{
			if ((threadsArray[i].j - threadsArray[i].i > finish - start) || (i + 1 < threadNumber &&
				((threadsArray[i + 1].i == threadsArray[i].j &&
					array[threadsArray[i + 1].i] > array[threadsArray[i].j - 1])
					|| (threadsArray[i + 1].i == threadsArray[i].j + 1
						&& array[threadsArray[i + 1].i] > array[threadsArray[i].j]))))
			{
				if (!(i + 1 < threadNumber &&
					((threadsArray[i + 1].i == threadsArray[i].j &&
						array[threadsArray[i + 1].i] > array[threadsArray[i].j - 1])
						|| (threadsArray[i + 1].i == threadsArray[i].j + 1
							&& array[threadsArray[i + 1].i] > array[threadsArray[i].j]))))
				{
					start = threadsArray[i].i;
					finish = threadsArray[i].j;
				}
				else
				{
					int start_0 = threadsArray[i].i;
					int finish_0 = threadsArray[i].j;
					while ((i + 1 < threadNumber && threadsArray[i].t != 0 &&
						((threadsArray[i + 1].i == threadsArray[i].j &&
							array[threadsArray[i + 1].i] > array[threadsArray[i].j - 1])
							|| (threadsArray[i + 1].i == threadsArray[i].j + 1
								&& array[threadsArray[i + 1].i] > array[threadsArray[i].j]))))
					{
						finish_0 = threadsArray[i + 1].j;
						i++;
					}
					if (finish_0 - start_0 > finish - start)
					{
						start = start_0;
						finish = finish_0;
					}
				}
			}
		}
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
	delete[] array;
	return 0;
}


#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include<ctime>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

using namespace std; 

const int bufSize = 2; //количество врачей

int rear = 0; //индекс для записи в буфер
int front = 0; //индекс для чтения из буфера

sem_t  emptySem; //семафор, отображающий насколько  буфер пуст
sem_t  full; //семафор, отображающий насколько полон буфер

pthread_mutex_t mutexDentist; //мутекс для дантиста
pthread_mutex_t mutexOculist; //мутекс для окулиста
pthread_mutex_t mutexTherapist; //мутекс для терапевт
vector<pthread_t> threadC; //потоки к врачам
vector<pthread_t> threadQ; //потоки к специалистам
unsigned int start_time;

/// <summary>
/// Структура - пациент
/// </summary>
struct Patient
{
	int id; //id
	int doctor; //специалист, к которому направили
	pthread_t thread; //поток, в котором он сущесивуеи
};

/// <summary>
/// Дантист
/// </summary>
/// <param name="param">пациент</param>
/// <returns></returns>
void* Dantist(void* param);
/// <summary>
/// Окулист
/// </summary>
/// <param name="param">пациент</param>
/// <returns></returns>
void* Oculist(void* param);
/// <summary>
/// Терапевт
/// </summary>
/// <param name="param">пациент</param>
/// <returns></returns>
void* Therapist(void* param);

/// <summary>
/// Доктор, который направляет пациента
/// </summary>
/// <param name="param">пациент</param>
/// <returns></returns>
void* Doctor(void* param) {

	Patient* patient = (Patient*)param;//извлечь пациента из буфера
	sem_wait(&emptySem); //количество занятых ячеек увеличить на единицу
	srand(GetCurrentThreadId());
	int doctor = rand() % 3; //к какому специалисту будет направлен
	rear = (rear + 1) % bufSize; //номер принимающего врача
	patient->doctor = doctor;//доктор
	unsigned int stop_time = clock();
	sem_post(&full); //количество свободных ячеек уменьшить на единицу
	//обработать полученный элемент 
	printf("%sВремя: %d Пациент %d:\tбыл направлен к %s от %s\n", patient->doctor == 0 ?
		"\t" : patient->doctor == 1 ? "\t\t" : "\t\t\t", stop_time - start_time, patient->id, patient->doctor == 0 ?
		"дантисту" : patient->doctor == 1 ? "окулисту" : "терапевту", rear-1 == 0 ? "Иванова" : "Сергеевича"); //вывод информации о посещении врача
	pthread_create(&threadQ[patient->id - 1], nullptr, (patient->doctor == 0) ?
		Dantist : patient->doctor == 1 ? Oculist : Therapist, (void*)(patient)); //отправка пациента к специалисту
	sem_wait(&full);//количество занятых ячеек уменьшить на единицу
	sem_post(&emptySem);//количество свободных ячеек увеличить на единицу
	return nullptr;
}

/// <summary>
/// Терапевт
/// </summary>
/// <param name="param">пациент</param>
/// <returns></returns>
void* Therapist(void* param)
{

	pthread_mutex_lock(&mutexTherapist);
	Patient* patient = (Patient*)param;
	unsigned int stop_time = clock();
	printf("\t\t\tВремя: %d Пациент %d:\tпосетил %s\n", stop_time - start_time, patient->id, "терапевта");
	pthread_mutex_unlock(&mutexTherapist);
	return nullptr;
}
/// <summary>
/// Окулист
/// </summary>
/// <param name="param">пациент</param>
/// <returns></returns>
void* Oculist(void* param)
{

	pthread_mutex_lock(&mutexOculist);
	Patient* patient = (Patient*)param;
	unsigned int stop_time = clock();
	printf("\t\tВремя: %d Пациент %d:\tпосетил %s\n", stop_time - start_time, patient->id, "окулиста");
	pthread_mutex_unlock(&mutexOculist);
	return nullptr;
}
/// <summary>
/// Дантист
/// </summary>
/// <param name="param">пациент</param>
/// <returns></returns>
void* Dantist(void* param)
{

	pthread_mutex_lock(&mutexDentist);
	Patient* patient = (Patient*)param;
	unsigned int stop_time = clock();
	printf("\tВремя: %d Пациент %d:\tпосетил %s\n", stop_time - start_time, patient->id, "дантиста");
	pthread_mutex_unlock(&mutexDentist);
	return nullptr;
}


int main()
{
	srand((unsigned)time(0));
	int n = rand() % 20 + 11;
	setlocale(LC_ALL, "Russian");
	cout << "Сегодня на приём пришло: " << n << " человек. Они стоят в очереди к двум специалистам (Иванов и Сергеевич).\n";    //инициализация мутексов и семафоров
	pthread_mutex_init(&mutexDentist, nullptr); //инициализация мьютекса дантиста
	pthread_mutex_init(&mutexOculist, nullptr);//инициализация мьютекса окулиста
	pthread_mutex_init(&mutexTherapist, nullptr);//инициализация мьютекса терапист
	sem_init(&emptySem, 0, bufSize); //количество свободных ячеек равно bufSize
	sem_init(&full, 0, 0); //количество занятых ячеек равно 0
	threadC= vector<pthread_t>(n);
	threadQ = vector<pthread_t>(n);
	vector<Patient> patients(n);
	start_time = clock();
	//пациенты приходят и встают в очередь
	for (int i = 0; i < n; i++)
	{

		patients[i].id = i+1;
		patients[i].thread = threadC[i];
		unsigned int stop_time = clock();
		printf("Время: %d Пациент %d занял очередь\n",stop_time-start_time, i+1);
		pthread_create(&threadC[i], nullptr, Doctor, (void*)(&patients[i]));
	}
	//ожидание завершения всех потоков
	for (int i = 0; i < n; i++)
	{
		pthread_join(threadC[i], NULL);
		pthread_join(threadQ[i], NULL);
	}
	return 0;
}

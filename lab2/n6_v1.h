#pragma once

#include <vector>
#include <string>
#include <ctime>
#include <iomanip>

#include "structures.h"

using namespace std;

// Количество создаваемых потоков.
const size_t t_count = 3;
// Количество дней в месяце.
const size_t days_count = 30;

// Текущий день.
size_t day;

// Данные для каждого из потоков (в т.ч. основного).
vector < ThreadData > thread_data(roles_count);

// pull - прямой ход конвейера (пара чисел), fetch - обратный (произведение пары чисел).
vector < bool > pull;
vector < bool > fetch;
vector < TaskAB > task_pull;
vector < TaskC > task_fetch;

// Вывод таблички с информацией о ходе передачи получения по конвейеру
void dumpTask() {
	// Порядок вывода строк таблицы
	static int roles_order[roles_count] = { Roles::P, Roles::L, Roles::S, Roles::R };

	// Форматируем шапку
	cout << setw(20) << "Человек" << setw(6) << "pull" << setw(6) << "fetch" << endl;
	cout << string(32, '=') << endl;

	// Выводим строки
	for (size_t i = 0; i < roles_count; ++i) {
		int role = roles_order[i];

		cout << setw(20) << thread_data[role].getName();
		cout << setw(6) << (task_pull[role].isEmpty() ? "н/д" : "ок");
		cout << setw(6) << (task_fetch[role].isEmpty() ? "н/д" : "ок");
		cout << endl;
	}

	cout << endl;
}

// Функция потока младщего командующего состава
DWORD WINAPI ThreadFuncLS(PVOID pvParam) {
	ThreadData *data = (ThreadData *)pvParam;

	cout << "Запущен поток младшего состава: " << data->getName() << "." << endl;
	
	for (size_t curr_day = day; curr_day <= days_count; ++curr_day) {
		// Активное ожидание наступление нового дня
		while (curr_day != day) {}

		// Активное ожидание поручения (прямой ход)
		while (!pull[data->fw]) {}

		dumpTask();

		// Получаем поручение
		task_pull[data->role] = task_pull[data->fw];
		// Тратим время на его обработку
		data->runDelay();
		// Готовы передать получение дальше
		pull[data->role] = true;

		// Активное ожидание результата поручения (обратный ход)
		while (!fetch[data->bw]) {}

		dumpTask();

		task_fetch[data->role] = task_fetch[data->bw];
		data->runDelay();
		fetch[data->role] = true;
	}

	return 0;
}

// Функция потока рядового, который перемножает 2 числа
DWORD WINAPI ThreadFuncR(PVOID pvParam) {
	ThreadData *data = (ThreadData *)pvParam;

	cout << "Запущен поток рядового: " << data->getName() << "." << endl;

	for (size_t curr_day = day; curr_day <= days_count; ++curr_day) {
		// Активное ожидание наступление нового дня
		while (curr_day != day) {}

		// Активное ожидание поручения
		while (!pull[data->fw]) {}

		dumpTask();

		// Поручение получено
		task_pull[data->role] = task_pull[data->fw];
		pull[data->role] = true;

		// Обработка поручения
		task_fetch[data->role] = task_pull[data->role].calc();
		// Всё готово
		fetch[data->role] = true;
	}

	return 0;
}

void run_n6_v1() {
	vector < HANDLE > hThread(t_count);

	// Полковник <-> Лейтенант <-> Сержант <-> Рядовой
	thread_data[Roles::P] = ThreadData(Roles::P, 0, Roles::L, Roles::L);
	thread_data[Roles::L] = ThreadData(Roles::L, 30, Roles::P, Roles::S);
	thread_data[Roles::S] = ThreadData(Roles::S, 15, Roles::L, Roles::R);
	thread_data[Roles::R] = ThreadData(Roles::R, 0, Roles::S, Roles::S);

	// Месяц в ВЧ
	for (day = 0; day < days_count;) {
		// Никто не знает про поручения
		pull.assign(roles_count, false);
		fetch.assign(roles_count, false);
		task_pull.assign(roles_count, TaskAB());
		task_fetch.assign(roles_count, TaskC());

		// Новый день
		++day;

		cout << "Наступил день № " << day << "!" << endl;

		Sleep(1000);

		// В первый день поднимаем весь состав на ноги
		if (day == 1) {
			for (size_t i = 0; i < t_count; ++i) {
				LPTHREAD_START_ROUTINE func = (thread_data[i].role == Roles::R ? ThreadFuncR : ThreadFuncLS);

				hThread[i] = CreateThread(NULL, 0, func, (PVOID)&thread_data[i], 0, 0);

				if (!hThread[i]) {
					cout << "Main process: thread " << i << " not execute!" << endl;
				}
			}
		}

		Sleep(1000);

		// Полковник создаёт поручение
		task_pull[Roles::P].gen();

		cout << task_pull[Roles::P] << endl;

		// И готов его отдать на обработку
		pull[Roles::P] = true;

		// Полковник ожидает готовности поручения
		while (!fetch[thread_data[Roles::P].bw]) {}

		dumpTask();

		// Получаем поручение
		task_fetch[Roles::P] = task_fetch[thread_data[Roles::P].bw];

		dumpTask();

		// Выводим результат дня
		cout << task_pull[Roles::P] << endl;
		cout << task_fetch[Roles::P] << endl;
	}

	// В конце месяца ждём всех
	DWORD dw = WaitForMultipleObjects(1, hThread.data(), TRUE, INFINITE);

	// Полковник уходит в закат...
}
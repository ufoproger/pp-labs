#pragma once

#include <vector>
#include <string>
#include <ctime>
#include <iomanip>

#include "structures.h"

using namespace std;

const size_t t_count = 3;
const size_t days_count = 3;

size_t day;

vector < ThreadData > thread_data(roles_count); // избыточный размер, чтобы иметь информацию о Кузнецове
vector < bool > pull;
vector < bool > fetch;
vector < TaskAB > task_pull;
vector < TaskC > task_fetch;

void dumpTask() {
	if (
		roles_count != pull.size() ||
		roles_count != task_pull.size() ||
		roles_count != fetch.size() ||
		roles_count != task_fetch.size()
	) {
		return;
	}

	static int roles_order[roles_count] = { Roles::P, Roles::L, Roles::S, Roles::R };

	cout << setw(20) << "Человек" << setw(6) << "pull" << setw(6) << "fetch" << endl;
	cout << string(32, '=') << endl;

	for (size_t i = 0; i < roles_count; ++i) {
		int role = roles_order[i];

		cout << setw(20) << thread_data[role].getName();
		cout << setw(6) << (task_pull[role].isEmpty() ? "н/д" : "ок");
		cout << setw(6) << (task_fetch[role].isEmpty() ? "н/д" : "ок");
		cout << endl;
	}

	cout << endl;
}

DWORD WINAPI ThreadFuncLS(PVOID pvParam) {
	ThreadData *data = (ThreadData *)pvParam;

	cout << "Запущен поток младшего состава: " << data->getName() << "." << endl;
	
	for (size_t curr_day = day; curr_day <= days_count; ++curr_day) {
		while (curr_day != day) {}

		while (!pull[data->fw]) {}

		dumpTask();

		task_pull[data->role] = task_pull[data->fw];
		data->runDelay();
		pull[data->role] = true;

		while (!fetch[data->bw]) {}

		dumpTask();

		task_fetch[data->role] = task_fetch[data->bw];
		data->runDelay();
		fetch[data->role] = true;
	}

	return 0;
}

DWORD WINAPI ThreadFuncR(PVOID pvParam) {
	ThreadData *data = (ThreadData *)pvParam;

	cout << "Запущен поток рядового: " << data->getName() << "." << endl;

	for (size_t curr_day = day; curr_day <= days_count; ++curr_day) {
		while (curr_day != day) {}

		while (!pull[data->fw]) {}

		dumpTask();

		task_pull[data->role] = task_pull[data->fw];
		pull[data->role] = true;

		task_fetch[data->role] = task_pull[data->role].calc();
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

	for (day = 0; day < days_count;) {

		pull.assign(roles_count, false);
		fetch.assign(roles_count, false);
		task_pull.assign(roles_count, TaskAB());
		task_fetch.assign(roles_count, TaskC());

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

		pull[Roles::P] = true;
		task_pull[Roles::P].gen();

		cout << task_pull[Roles::P] << endl;

		while (!fetch[thread_data[Roles::P].bw]) {}

		dumpTask();

		task_fetch[Roles::P] = task_fetch[thread_data[Roles::P].bw];

		dumpTask();

		cout << task_pull[Roles::P] << endl;
		cout << task_fetch[Roles::P] << endl;
	}

	DWORD dw = WaitForMultipleObjects(1, hThread.data(), TRUE, INFINITE);
}
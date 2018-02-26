#pragma once

#include <vector>
#include <string>
#include <ctime>

using namespace std;

const size_t t_count = 3;
const size_t roles_count = 4;

enum Roles {
	L,	// лейтенант
	S,	// сержант
	R,	// рядовой Иванов
	P	// полковник Кузнецов
};

struct ThreadData {
	int role = -1;
	size_t delta = 0;
	size_t from = -1;
	size_t to = -1;

	ThreadData(int role_, size_t delta_, size_t from_, size_t to_) :
		role(role_),
		delta(delta_),
		from(from_),
		to(to_)
	{}

	ThreadData() {}
	
	void runDelay() {
		if (this->delta) {
			Sleep(rand() % this->delta / 5 * 1000);
		}
	}

	string getName() {
		switch (this->role) {
		case Roles::P:
			return "полковник Кузнецов";

		case Roles::L:
			return "лейтенант";

		case Roles::S:
			return "сержант";

		case Roles::R:
			return "рядовой Иванов";
		}

		return "";
	}
};

struct TaskAB {
	int a = 0;
	int b = 0;

	void gen() {
		this->a = rand() % 100 + 1;
		this->b = rand() % 100 + 1;
	}

	inline bool isEmpty() {
		return (!this->a || !this->b);
	}
};

struct TaskC {
	int c = 0;

	inline bool isEmpty() {
		return !this->c;
	}
};

vector < bool > pull;
vector < bool > fetch;
vector < TaskAB > task_pull;
vector < TaskC > task_fetch;

void dumpTask() {
	cout << "dump task" << endl;
}

DWORD WINAPI ThreadFuncLS(PVOID pvParam) {
	ThreadData *data = (ThreadData *)pvParam;

	cout << "Запущен поток младшего состава: " << data->getName() << "." << endl;

	cout << "Ожидание..." << endl;

	while (!pull[data->from]) {}

	cout << "Ожидание завершено..." << endl;

	return 0;
}

DWORD WINAPI ThreadFuncR(PVOID pvParam) {
	ThreadData *data = (ThreadData *)pvParam;

	cout << "Запущен поток рядового: " << data->getName() << "." << endl;

	return 0;
}

void run_n6_v1() {
	srand(time((time_t)NULL));

	vector < HANDLE > hThread(t_count);
	vector < ThreadData > data(t_count);

	pull.assign(roles_count, false);
	fetch.assign(roles_count, false);
	task_pull.assign(roles_count, TaskAB());
	task_fetch.assign(roles_count, TaskC());

	data[Roles::L] = ThreadData(Roles::L, 30, Roles::P, Roles::S);
	data[Roles::S] = ThreadData(Roles::S, 15, Roles::L, Roles::R);
	data[Roles::R] = ThreadData(Roles::R, 0, Roles::S, Roles::S);
	
	for (size_t i = 0; i < t_count; ++i) {
		LPTHREAD_START_ROUTINE func = (data[i].role == Roles::R ? ThreadFuncR : ThreadFuncLS);

		hThread[i] = CreateThread(NULL, 0, func, (PVOID)&data[i], 0, 0);

		if (!hThread[i]) {
			cout << "Main process: thread " << i << " not execute!" << endl;
		}

	}



	DWORD dw = WaitForMultipleObjects(1, hThread.data(), TRUE, INFINITE);
}
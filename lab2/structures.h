#pragma once

#include <vector>
#include <iostream>
#include <iomanip>
#include <string>

using namespace std;

// Количество создаваемых потоков.
const size_t t_count = 3;
// Количество дней в месяце.
const size_t days_count = 30;
const size_t roles_count = 4;

enum Roles {
	L,	// лейтенант
	S,	// сержант
	R,	// рядовой Иванов
	P	// полковник Кузнецов
};

struct TaskC {
	int c;

	TaskC(int c_ = 0) : c(c_) {}

	inline bool isEmpty() const {
		return (!this->c);
	}
};

ostream& operator<<(ostream &out, const TaskC &task) {
	return (out << "Результат выполнения поручения: " << task.c << ".");
}

struct TaskAB {
	const size_t range = 20;

	int a;
	int b;

	TaskAB() : a(0), b(0) {}
	TaskAB(int a_, int b_) : a(a_), b(b_) {}

	TaskAB& operator=(const TaskAB &copy) {
		this->a = copy.a;
		this->b = copy.b;

		return *this;
	}

	TaskC calc() const {
		return TaskC(this->a * this->b);
	}

	void gen() {
		this->a = rand() % 20 + 1;
		this->b = rand() % 20 + 1;
	}

	inline bool isEmpty() const {
		return (!this->a && !this->b);
	}
};

ostream& operator<<(ostream &out, const TaskAB &task) {
	return (out << "Поручение на перемножение чисел " << task.a << " и " << task.b << ".");
}

struct ThreadData {
	int role = -1;
	size_t delta = 0;
	size_t fw = -1; // forward - прямой ход конвейера
	size_t bw = -1; // backward - обратный ход конвейера

	ThreadData(int role_, size_t delta_, size_t fw_, size_t bw_) :
		role(role_),
		delta(delta_),
		fw(fw_),
		bw(bw_)
	{}

	ThreadData() {}

	void runDelay() const {
		if (this->delta) {
			Sleep(((DWORD)this->delta / 15 + 1) * 1000);
		}
	}

	string getName() const {
		switch (this->role) {
		case Roles::P:
			return "Полковник Кузнецов";

		case Roles::L:
			return "Лейтенант";

		case Roles::S:
			return "Сержант";

		case Roles::R:
			return "Рядовой Иванов";
		}

		return "";
	}
};

void DumpTask(const vector < ThreadData > &data, const vector < TaskAB > &pull, const vector < TaskC > &fetch) {
	// Порядок вывода строк таблицы
	static int roles_order[roles_count] = { Roles::P, Roles::L, Roles::S, Roles::R };

	// Форматируем шапку
	cout << setw(20) << "Человек" << setw(6) << "pull" << setw(6) << "fetch" << endl;
	cout << string(32, '=') << endl;

	// Выводим строки
	for (size_t i = 0; i < roles_count; ++i) {
		int role = roles_order[i];

		cout << setw(20) << data[role].getName();
		cout << setw(6) << (pull[role].isEmpty() ? "н/д" : "ок");
		cout << setw(6) << (fetch[role].isEmpty() ? "н/д" : "ок");
		cout << endl;
	}

	cout << endl;
}
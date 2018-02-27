#pragma once

#include <iostream>
#include <string>

using namespace std;

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

	inline bool isEmpty() {
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
			Sleep(rand() % this->delta / 5 * 1000);
		}
	}

	string getName() const {
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


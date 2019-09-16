#include <windows.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

using namespace std;

struct FuncParam {
	int from;
	int to;
	int thread_num;
};

ostream& operator<<(ostream& out, const FuncParam &data) {
	out << "(поток № " << (data.thread_num + 1) << ", ";

	if (data.from + 1 == data.to) {
		out << "отделение № " << (data.from + 1);
	}
	else {
		out << "отделения №№ " << (data.from + 1) << "-" << data.to;
	}

	return (out << ")");
}

struct Z {
	int id;
	int value;

	Z(int id_, int value_) : id(id_), value(value_) {}
	Z() : id(0), value(0) {}
};

ostream& operator<<(ostream& out, const Z &z) {
	return (out << "Заготовитель № " << (z.id + 1) << ", его добыча " << z.value << " шт");
}

vector < vector < int > > m;
vector < vector < Z > > result;

int t_count;
int o_count;
int z_count;

DWORD WINAPI
ThreadFunc(PVOID pvParam) {
	FuncParam x = *((FuncParam *)pvParam);

	cout << "Поток запустился. Информация: " << x << "." << endl;

	for (int i = x.from; i < x.to; ++i) {
		int maxValue = 0;

		for (int j = 0; j < z_count; ++j) {
			maxValue = max(maxValue, m[i][j]);
		}

		for (int j = 0; j < z_count; ++j) {
			if (m[i][j] == maxValue) {
				result[i].push_back(Z(j, maxValue));
			}
		}
	}

	return (DWORD)x.thread_num;
}


int main() {
	srand(time(NULL));
	SetConsoleOutputCP(1251);
	SetConsoleCP(1251);

	cout << "Введите количество потоков: ";
	cin >> t_count;

	if (t_count < 1) {
		cout << "Число потоков не может быть меньше 1." << endl;
		system("pause");

		return -1;
	}

	cout << "Введите количество отделений: ";
	cin >> o_count;

	cout << "Введите количество заготовителей: ";
	cin >> z_count;

	if (o_count < t_count) {
		cout << "Введено количество потоков больше, чем количество организаций." << endl;
		cout << "Во избежании холостого выполнения потоков их количество будет уменьшено." << endl;

		t_count = o_count;

		cout << "Новое количество потоков: " << t_count << " шт." << endl;
	}

	for (int i = 0; i < o_count; ++i) {
		m.push_back(vector < int >(z_count, 0));

		for (int j = 0; j < z_count; ++j) {
			m[i][j] = (rand() % 20) + 1;
		}
	}

	cout << endl << "Входные данные: " << endl;

	for (int i = 0; i < o_count; ++i) {
		for (int j = 0; j < z_count; ++j) {
			cout << m[i][j] << "\t";
		}

		cout << endl;
	}

	vector < DWORD > dwThreadId(t_count);
	vector < HANDLE > hThread(t_count);

	vector < FuncParam > x(t_count);
	result.resize(o_count);

	vector < int > steps(t_count, o_count / t_count);

	for (int i = 0; i < o_count % t_count; ++i) {
		steps[i]++;
	}

	for (int i = 0, o_i = 0; i < t_count; o_i += steps[i], ++i) {
		x[i].thread_num = i;
		x[i].from = o_i;
		x[i].to = min(o_i + steps[i], o_count);

		hThread[i] = CreateThread(NULL, 0, ThreadFunc, (PVOID)&x[i], 0, &dwThreadId[i]);

		if (!hThread[i]) {
			cout << "Main process: thread " << i << " not execute!" << endl;
		}
	}

	DWORD dw = WaitForMultipleObjects(t_count, hThread.data(), TRUE, INFINITE);

	for (int i = 0; i < t_count; i++) {
		DWORD exitResult;

		GetExitCodeThread(hThread[i], &exitResult);

		cout << "Поток № " << (exitResult + 1) << " завершился." << endl;
	}

	cout << endl;
	cout << "Результаты отделений: " << endl;

	for (int i = 0; i < o_count; ++i) {
		cout << "Отделение № " << (i + 1) << ": " << endl;

		for (auto z : result[i]) {
			cout << "\t" << z << endl;
		}

		cout << endl << endl;
	}

	cout << endl;

	int maxValue = 0;

	for (auto row : result) {
		for (auto cell : row) {
			maxValue = max(maxValue, cell.value);
		}
	}

	cout << "Максимальный результат: " << maxValue << " шт." << endl;
	cout << "Лучшие заготовители: " << endl;

	for (int i = 0; i < o_count; ++i) {
		for (int j = 0; j < result[i].size(); ++j) {
			if (result[i][j].value == maxValue) {
				cout << "\tОтделение № " << (i + 1) << ": " << result[i][j] << "." << endl;
			}
		}
	}
	
	system("pause");

	return 0;
}

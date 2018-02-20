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
	vector < DWORD > dwResult(t_count);
	vector < HANDLE > hThread(t_count);

	// Предполагаемое количество отделений, которое будет обрабатывать 1 поток.
	int step = o_count / t_count;

	// Если не удаётся распределить отделения равномерно по потокам, накидываем ещё по одному отделению первым потокам.
	if (o_count % t_count) {
		step += 1;
	}

	// Если отделений меньше, чем потоков, то распихиваем по одному отделению на поток (кому хватит).
	if (step < 1) {
		step = 1;
	}

	vector < FuncParam > x(t_count);
	result.resize(o_count);

	for (int o_i = 0, t_i = 0; o_i < o_count; o_i += step, ++t_i) {
		x[t_i].thread_num = t_i;
		x[t_i].from = o_i;
		x[t_i].to = min(o_i + step, o_count);

		hThread[t_i] = CreateThread(NULL, 0, ThreadFunc, (PVOID)&x[t_i], 0, &dwThreadId[t_i]);

		if (!hThread[t_i]) {
			cout << "Main process: thread " << t_i << " not execute!" << endl;
		}
	}

	DWORD dw = WaitForMultipleObjects(t_count, hThread.data(), TRUE, INFINITE);

	for (int i = 0; i < t_count; i++) {
		DWORD *exitResult = &dwResult.data()[i];

		GetExitCodeThread(hThread[i], exitResult);

		cout << "Поток № " << ((int)*exitResult + 1) << " завершился." << endl;
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
	
	cin.ignore();
	cin.get();

	return 0;
}

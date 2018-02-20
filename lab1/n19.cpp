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
	out << "(����� � " << (data.thread_num + 1) << ", ";

	if (data.from + 1 == data.to) {
		out << "��������� � " << (data.from + 1);
	}
	else {
		out << "��������� �� " << (data.from + 1) << "-" << data.to;
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
	return (out << "������������ � " << (z.id + 1) << ", ��� ������ " << z.value << " ��");
}

vector < vector < int > > m;
vector < vector < Z > > result;

int t_count;
int o_count;
int z_count;

DWORD WINAPI
ThreadFunc(PVOID pvParam) {
	FuncParam x = *((FuncParam *)pvParam);

	cout << "����� ����������. ����������: " << x << "." << endl;

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

	cout << "������� ���������� �������: ";
	cin >> t_count;

	cout << "������� ���������� ���������: ";
	cin >> o_count;

	cout << "������� ���������� �������������: ";
	cin >> z_count;

	if (o_count < t_count) {
		cout << "������� ���������� ������� ������, ��� ���������� �����������." << endl;
		cout << "�� ��������� ��������� ���������� ������� �� ���������� ����� ���������." << endl;

		t_count = o_count;

		cout << "����� ���������� �������: " << t_count << " ��." << endl;
	}

	for (int i = 0; i < o_count; ++i) {
		m.push_back(vector < int >(z_count, 0));

		for (int j = 0; j < z_count; ++j) {
			m[i][j] = (rand() % 20) + 1;
		}
	}

	cout << endl << "������� ������: " << endl;

	for (int i = 0; i < o_count; ++i) {
		for (int j = 0; j < z_count; ++j) {
			cout << m[i][j] << "\t";
		}

		cout << endl;
	}

	vector < DWORD > dwThreadId(t_count);
	vector < DWORD > dwResult(t_count);
	vector < HANDLE > hThread(t_count);

	// �������������� ���������� ���������, ������� ����� ������������ 1 �����.
	int step = o_count / t_count;

	// ���� �� ������ ������������ ��������� ���������� �� �������, ���������� ��� �� ������ ��������� ������ �������.
	if (o_count % t_count) {
		step += 1;
	}

	// ���� ��������� ������, ��� �������, �� ����������� �� ������ ��������� �� ����� (���� ������).
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

		cout << "����� � " << ((int)*exitResult + 1) << " ����������." << endl;
	}

	cout << endl;
	cout << "���������� ���������: " << endl;

	for (int i = 0; i < o_count; ++i) {
		cout << "��������� � " << (i + 1) << ": " << endl;

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

	cout << "������������ ���������: " << maxValue << " ��." << endl;
	cout << "������ ������������: " << endl;

	for (int i = 0; i < o_count; ++i) { 
		for (int j = 0; j < result[i].size(); ++j) {
			if (result[i][j].value == maxValue) {
				cout << "\t��������� � " << (i + 1) << ": " << result[i][j] << "." << endl;
			}
		}
	}
	
	cin.ignore();
	cin.get();

	return 0;
}

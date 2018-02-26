#include <iostream>
#include <Windows.h>

#include "n6_v1.h"
#include "n6_v2.h"

using namespace std;

int main() {
	//srand(time(NULL));
	SetConsoleOutputCP(1251);
	SetConsoleCP(1251);

	int program_v = 1;

	//cout << "������� ����� ��������� (1 ��� 2): ";
	//cin >> program_v;

	switch (program_v) {
	case 1:
		cout << "����� �������� ������ ���������." << endl;
		run_n6_v1();
		break;

	case 2:
		cout << "����� �������� ������ ���������." << endl;
		run_n6_v2();
		break;

	default:
		cout << "������ ����������� ����� ���������. �����." << endl;
	}

	system("pause");

	return 0;
}
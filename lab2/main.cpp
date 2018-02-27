#include <iostream>
#include <Windows.h>

#include "structures.h"
#include "n6_v1.h"
#include "n6_v2.h"

using namespace std;

int main() {
	srand((unsigned int)time(NULL));
	SetConsoleOutputCP(1251);
	SetConsoleCP(1251);

	int program_v = 1;

	cout << "Воинская часть № " << (rand() % 90000 + 10000) << ". Добро пожаловать!" << endl;

	cout << "Введите номер программы (1 или 2): ";
	cin >> program_v;

	switch (program_v) {
	case 1:
		cout << "Будет запущена первая программа." << endl;
		v1::run_n6();
		break;

	case 2:
		cout << "Будет запущена вторая программа." << endl;
		v2::run_n6();
		break;

	default:
		cout << "Указан неизвестный номер программы. Выход." << endl;
	}

	system("pause");

	return 0;
}
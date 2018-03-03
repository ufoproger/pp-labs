#include <Windows.h>>
#include <iostream>
#include <vector>
#include <ctime>

using namespace std;

#define P(sem) WaitForSingleObject(sem, INFINITE)
#define Vn(sem, n) ReleaseSemaphore(sem, n, NULL)
#define V(sem) Vn(sem, 1)

HANDLE hPot;
HANDLE hChef;
HANDLE hCout;

DWORD WINAPI ThreadFuncCannibal(PVOID pvParam) {
	size_t index = PtrToUlong(pvParam);

	P(hCout);
	cout << "Выполняется поток каннибала № " << index << "." << endl;
	V(hCout);

	for (;;) {
		Sleep(rand() % 8 * 1000);

		P(hPot);
		P(hCout);
		cout << "Каннибал № " << index << " взял из котла кусок мяса..." << endl;
		V(hCout);
		V(hChef);
	}

	return 0;
}

DWORD WINAPI ThreadFuncChef(PVOID pvParam) {
	size_t m = PtrToUlong(pvParam);

	P(hCout);
	cout << "Выполняется поток повара." << endl;
	V(hCout);

	for (;;) {
		for (size_t i = 0; i < m; ++i) {
			P(hChef);
		}

		P(hCout);
		cout << "Добавленное поваром количество куском мяса в котёл: " << m << " шт." << endl;
		V(hCout);
		Vn(hPot, m);
	}

	return 0;
}

int main(int argc, char *argv[]) {
	srand((unsigned int)time(NULL));
	SetConsoleOutputCP(1251);
	SetConsoleCP(1251);

	size_t n, m;

	cout << "Количество дикарей и кусков мяса в котле: ";
	cin >> n >> m;

	if (n < 1 || m < 1) {
		cout << "Значения не могут быть меньше единицы!" << endl;
		return -1;
	}

	hPot = CreateSemaphore(NULL, m, m, NULL);
	hChef = CreateSemaphore(NULL, 0, m, NULL);
	hCout = CreateSemaphore(NULL, 1, 1, NULL);

	vector < HANDLE > hThreads(n + 1);

	for (size_t i = 0; i <= n; i++)
	{
		LPTHREAD_START_ROUTINE func = (i < n) ? ThreadFuncCannibal : ThreadFuncChef;
		size_t param = (i < n) ? (i + 1) : m;

		hThreads[i] = CreateThread(NULL, 0, func, UlongToPtr(param), 0, NULL);
	}

	WaitForMultipleObjects(n + 1, hThreads.data(), TRUE, INFINITE);
	system("pause");

	return 0;
}

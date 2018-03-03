#include <Windows.h>>
#include <iostream>
#include <vector>
#include <ctime>

using namespace std;

HANDLE hCannibals = NULL;
HANDLE hChef = NULL;
vector < size_t > threadsData;
size_t n, m;

DWORD WINAPI ThreadFuncCannibal(PVOID pvParam = NULL) {
	cout << "Выполняется поток каннибала." << endl;

	for (;;) {
		Sleep(rand() % 8 * 1000);

		WaitForSingleObject(hCannibals, INFINITE);

		cout << "Съел кусок мяса" << endl;

		ReleaseSemaphore(hChef, 1, NULL);
	}

	return 0;
}

DWORD WINAPI ThreadFuncChef(PVOID pvParam = NULL) {
	cout << "Выполняется поток повара." << endl;

	for (;;) {
		for (size_t i = 0; i < m; ++i) {
			WaitForSingleObject(hChef, INFINITE);
		}

		cout << "Повар добавляет куски мяса в котёл." << endl;
		ReleaseSemaphore(hCannibals, m, NULL);
	}

	return 0;
}

int main(int argc, char *argv[]) {
	srand((unsigned int)time(NULL));
	SetConsoleOutputCP(1251);
	SetConsoleCP(1251);

	cout << "Количество дикарей и кусков мяса в котле: ";
	cin >> n >> m;

	vector < HANDLE > hThreads(n + 1, NULL);

	hCannibals = CreateSemaphore(
		NULL,           // default security attributes
		m,  // initial count
		m,  // maximum count
		NULL
	);

	hChef = CreateSemaphore(
		NULL,           // default security attributes
		0,  // initial count
		m,  // maximum count
		NULL
	);

	for (size_t i = 0; i <= n; i++)
	{
		LPTHREAD_START_ROUTINE func = (i < n ? ThreadFuncCannibal : ThreadFuncChef);

		hThreads[i] = CreateThread(
			NULL,       // default security attributes
			0,          // default stack size
			func,
			NULL,       // no thread function arguments
			0,          // default creation flags
			NULL); // receive thread identifier

		if (hThreads[i] == NULL)
		{
			cout << "CreateThread error: " << GetLastError() << endl;
			return 1;
		}
	}

	WaitForMultipleObjects(n + 1, hThreads.data(), TRUE, INFINITE);

	system("pause");

	return 0;
}

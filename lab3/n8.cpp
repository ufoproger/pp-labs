#include <Windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <ctime>

using namespace std;

#define P(sem) WaitForSingleObject(sem, INFINITE)
#define Vn(sem, n) ReleaseSemaphore(sem, n, NULL)
#define V(sem) Vn(sem, 1)

#define SEM_POT_NAME "pot"
#define SEM_CHEF_NAME "chef"
#define SEM_COUT_NAME "cout"

HANDLE hPot, hChef, hCout;

DWORD WINAPI ThreadFuncWildman(PVOID pvParam) {
	long index = PtrToUlong(pvParam);

	P(hCout);
	cout << "Выполняется поток дикаря № " << index << "." << endl;
	V(hCout);

	for (;;) {
		Sleep(rand() % 10 * 500);

		P(hCout);
		cout << "Дикарь № " << index << " захотел есть..." << endl;
		V(hCout);

		P(hPot);
		P(hCout);
		cout << "Дикарь № " << index << " взял кусок мяса из котла." << endl;
		V(hCout);
		V(hChef);
	}

	return 0;
}

DWORD WINAPI ThreadFuncChef(PVOID pvParam) {
	long m = PtrToUlong(pvParam);

	P(hCout);
	cout << "Выполняется поток повара." << endl;
	V(hCout);

	for (;;) {
		for (int i = 0; i < m; ++i) {
			P(hChef);
		}

		P(hCout);
		cout << "Повар просыпается..." << endl;
		V(hCout);

		Sleep(1000);

		P(hCout);
		Vn(hPot, m);
		cout << "Повар добавил мясо в котёл: " << m << " шт." << endl;
		V(hCout);
	}

	return 0;
}

int main(int argc, char *argv[]) {
	srand((unsigned int)time(NULL));
	SetConsoleOutputCP(1251);
	SetConsoleCP(1251);

	cout << "Аргументы командной строки: " << endl;

	for (int i = 0; i < argc; ++i) {
		cout << "\t" << (i + 1) << ") " << argv[i] << endl;
	}

	cout << endl;

	if (argc == 3) {
		string role = string(argv[1]);
		long value = stoi(string(argv[2]));

		hPot = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, SEM_POT_NAME);
		hChef = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, SEM_CHEF_NAME);
		hCout = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, SEM_COUT_NAME);

		if (role == "/chef") {
			ThreadFuncChef(ULongToPtr(value));
		}
		else if (role == "/wildman") {
			ThreadFuncWildman(ULongToPtr(value));
		}
	}
	else {
		long n, m;
		cout << "Количество дикарей и кусков мяса в котле: ";
		cin >> n >> m;

		if (n < 1 || m < 1) {
			cout << "Значения не могут быть меньше единицы!" << endl;
			return -1;
		}

		char method;

		cout << "Выберите реализацию задания (t = через потоки, p = через процессы): ";
		cin >> method;

		hPot = CreateSemaphore(NULL, m, m, SEM_POT_NAME);
		hChef = CreateSemaphore(NULL, 0, m, SEM_CHEF_NAME);
		hCout = CreateSemaphore(NULL, 1, 1, SEM_COUT_NAME);

		vector < HANDLE > hChildren(n + 1);

		switch (method) {
		case 't':
		case 'T': {
			cout << "Выбрана реализация через потоки..." << endl;

			for (int i = 0; i <= n; i++)
			{
				LPTHREAD_START_ROUTINE func = (i < n) ? ThreadFuncWildman : ThreadFuncChef;
				long param = (i < n) ? (i + 1) : m;

				hChildren[i] = CreateThread(NULL, 0, func, UlongToPtr(param), 0, NULL);
			}

			break;
		}

		case 'p':
		case 'P': {
			cout << "Выбрана реализация через процессы..." << endl;

			vector < STARTUPINFO >si(n + 1);
			vector < PROCESS_INFORMATION > pi(n + 1);

			for (int i = 0; i <= n; ++i) {
				ZeroMemory(&si[i], sizeof(si[i]));
				si[i].cb = sizeof(si[i]);
				ZeroMemory(&pi[i], sizeof(pi[i]));

				string cmd(argv[0]);

				if (i < n) {
					cmd += " /wildman " + to_string(i + 1);
				}
				else {
					cmd += " /chef " + to_string(m);
				}

				CreateProcess(NULL,		// No module name (use command line)
					const_cast<char *>(cmd.c_str()),	// Command line
					NULL,			// Process handle not inheritable
					NULL,			// Thread handle not inheritable
					TRUE,			// Set handle inheritance to FALSE
					0,				// No creation flags
					NULL,			// Use parent's environment block
					NULL,			// Use parent's starting directory 
					&si[i],			// Pointer to STARTUPINFO structure
					&pi[i]			// Pointer to PROCESS_INFORMATION structure
				);

				hChildren[i] = pi[i].hProcess;
			}

			break;
		}

		default:
			cout << "Неизвестная реализация!" << endl;
			hChildren.clear();
		}
		
		WaitForMultipleObjects((DWORD)hChildren.size(), hChildren.data(), TRUE, INFINITE);
	}

	CloseHandle(hPot);
	CloseHandle(hChef);
	CloseHandle(hCout);

	system("pause");

	return 0;
}

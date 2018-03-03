#include <Windows.h>
#include <iostream>
#include <vector>
#include <string>
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

	cout << "Аргументы командной строки: " << endl;

	for (size_t i = 0; i < argc; ++i) {
		cout << "\t" << (i + 1) << ") " << argv[i] << endl;
	}

	cout << endl;

	if (argc == 3) {
		string role = string(argv[1]);
		size_t value = stoi(string(argv[2]));

		if (role == "/chef") {
			ThreadFuncChef(ULongToPtr(value));
		}
		else if (role == "/cannibal") {
			ThreadFuncCannibal(ULongToPtr(value));
		}
	}
	else {
		size_t n, m;
		cout << "Количество дикарей и кусков мяса в котле: ";
		cin >> n >> m;

		if (n < 1 || m < 1) {
			cout << "Значения не могут быть меньше единицы!" << endl;
			return -1;
		}

		char method;

		cout << "Выберите реализацию задания (t = через потоки, p = через процессы): ";
		cin >> method;

		hPot = CreateSemaphore(NULL, m, m, NULL);
		hChef = CreateSemaphore(NULL, 0, m, NULL);
		hCout = CreateSemaphore(NULL, 1, 1, NULL);

		switch (method) {
		case 't':
		case 'T': {
			cout << "Выбрана реализация через потоки..." << endl;

			vector < HANDLE > hThreads(n + 1);

			for (size_t i = 0; i <= n; i++)
			{
				LPTHREAD_START_ROUTINE func = (i < n) ? ThreadFuncCannibal : ThreadFuncChef;
				size_t param = (i < n) ? (i + 1) : m;

				hThreads[i] = CreateThread(NULL, 0, func, UlongToPtr(param), 0, NULL);
			}

			WaitForMultipleObjects(n + 1, hThreads.data(), TRUE, INFINITE);
			break;
		}

		case 'p':
		case 'P': {
			cout << "Выбрана реализация через процессы..." << endl;

			vector < STARTUPINFO >siProcesses(n + 1);
			vector < PROCESS_INFORMATION > piProcesses(n + 1);

			for (size_t i = 0; i <= n; ++i) {
				ZeroMemory(&siProcesses[i], sizeof(siProcesses[i]));
				siProcesses[i].cb = sizeof(siProcesses[i]);
				ZeroMemory(&piProcesses[i], sizeof(piProcesses[i]));

				string cmd(argv[0]);

				if (i < n) {
					cmd += " /cannibal " + to_string(i);
				}
				else {
					cmd += " /chef " + to_string(m);
				}

				CreateProcess(NULL,   // No module name (use command line)
					const_cast<char *>(cmd.c_str()),        // Command line
					NULL,           // Process handle not inheritable
					NULL,           // Thread handle not inheritable
					FALSE,          // Set handle inheritance to FALSE
					0,              // No creation flags
					NULL,           // Use parent's environment block
					NULL,           // Use parent's starting directory 
					&siProcesses[i],            // Pointer to STARTUPINFO structure
					&piProcesses[i]           // Pointer to PROCESS_INFORMATION structure
				);
			}

			///WaitForSingleObject(pi.hProcess, INFINITE);

			break;
		}

		default:
			cout << "Неизвестная реализация!" << endl;
		}
	}

	Sleep(3000);
	system("pause");

	return 0;
}

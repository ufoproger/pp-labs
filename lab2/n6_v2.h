#pragma once

using namespace std;

namespace v2 {
	CRITICAL_SECTION cs_cout;
	SYNCHRONIZATION_BARRIER sb_day;

	// Текущий день.
	size_t day;

	// Данные для каждого из потоков (в т.ч. основного).
	vector < ThreadData > thread_data(roles_count);

	// pull - прямой ход конвейера (пара чисел), fetch - обратный (произведение пары чисел).
	vector < HANDLE > h_pull(roles_count, NULL);
	vector < HANDLE > h_fetch(roles_count, NULL);
	vector < TaskAB > task_pull;
	vector < TaskC > task_fetch;

	bool WaitMutex(HANDLE hHandle) {
		return (WaitForSingleObject(hHandle, INFINITE) == WAIT_OBJECT_0);
	}

	// Вывод таблички с информацией о ходе передачи получения по конвейеру
	void dumpTask() {
		EnterCriticalSection(&cs_cout);
		DumpTask(thread_data, task_pull, task_fetch);
		LeaveCriticalSection(&cs_cout);
	}

	// Функция потока младщего командующего состава
	DWORD WINAPI ThreadFuncLS(PVOID pvParam) {
		ThreadData *data = (ThreadData *)pvParam;

		EnterCriticalSection(&cs_cout);
		cout << "Запущен поток младшего состава: " << data->getName() << "." << endl;
		LeaveCriticalSection(&cs_cout);

		h_pull[data->role] = CreateMutex(NULL, FALSE, NULL);
		h_fetch[data->role] = CreateMutex(NULL, FALSE, NULL);

		for (size_t curr_day = day; curr_day <= days_count; ++curr_day) {
			WaitMutex(h_pull[data->role]);
			WaitMutex(h_fetch[data->role]);

			EnterCriticalSection(&cs_cout);
			cout << data->getName() << " готов к началу нового дня." << endl;
			LeaveCriticalSection(&cs_cout);

			EnterSynchronizationBarrier(&sb_day, NULL);

			// Активное ожидание поручения (прямой ход)
			WaitMutex(h_pull[data->fw]);
			dumpTask();

			// Получаем поручение
			task_pull[data->role] = task_pull[data->fw];
			ReleaseMutex(h_pull[data->fw]);

			// Тратим время на его обработку
			data->runDelay();

			// Готовы передать получение дальше
			ReleaseMutex(h_pull[data->role]);

			WaitForSingleObject(h_fetch[data->bw], INFINITE);
			dumpTask();

			task_fetch[data->role] = task_fetch[data->bw];
			ReleaseMutex(h_fetch[data->bw]);
			data->runDelay();
			ReleaseMutex(h_fetch[data->role]);
		}

		CloseHandle(h_pull[data->role]);
		CloseHandle(h_fetch[data->role]);

		return 0;
	}

	// Функция потока рядового, который перемножает 2 числа
	DWORD WINAPI ThreadFuncR(PVOID pvParam) {
		ThreadData *data = (ThreadData *)pvParam;

		EnterCriticalSection(&cs_cout);
		cout << "Запущен поток рядового: " << data->getName() << "." << endl;
		LeaveCriticalSection(&cs_cout);

		h_pull[data->role] = CreateMutex(NULL, FALSE, NULL);
		h_fetch[data->role] = CreateMutex(NULL, FALSE, NULL);

		for (size_t curr_day = day; curr_day <= days_count; ++curr_day) {
			WaitMutex(h_pull[data->role]);
			WaitMutex(h_fetch[data->role]);

			EnterCriticalSection(&cs_cout);
			cout << data->getName() << " готов к началу нового дня." << endl;
			LeaveCriticalSection(&cs_cout);

			EnterSynchronizationBarrier(&sb_day, NULL);
			
			// Активное ожидание поручения
			WaitMutex(h_pull[data->fw]);
			//while (!pull[data->fw]) {}

			dumpTask();

			// Поручение получено
			task_pull[data->role] = task_pull[data->fw];
			ReleaseMutex(h_pull[data->fw]);
			ReleaseMutex(h_pull[data->role]);

			// Обработка поручения
			task_fetch[data->role] = task_pull[data->role].calc();

			// Всё готово
			ReleaseMutex(h_fetch[data->role]);
		}

		return 0;
	}

	void run_n6() {
		vector < HANDLE > hThread(t_count);

		// Полковник <-> Лейтенант <-> Сержант <-> Рядовой
		auto data = thread_data[Roles::P] = ThreadData(Roles::P, 0, Roles::L, Roles::L);
		thread_data[Roles::L] = ThreadData(Roles::L, 30, Roles::P, Roles::S);
		thread_data[Roles::S] = ThreadData(Roles::S, 15, Roles::L, Roles::R);
		thread_data[Roles::R] = ThreadData(Roles::R, 0, Roles::S, Roles::S);

		InitializeCriticalSection(&cs_cout);
		InitializeSynchronizationBarrier(&sb_day, roles_count, INFINITE);

		h_pull[data.role] = CreateMutex(NULL, FALSE, NULL);
		h_fetch[data.role] = CreateMutex(NULL, FALSE, NULL);

		// Месяц в ВЧ
		for (day = 1; day <= days_count; ++day) {
			// Никто не знает про поручения
			task_pull.assign(roles_count, TaskAB());
			task_fetch.assign(roles_count, TaskC());

			WaitMutex(h_pull[data.role]);
			WaitMutex(h_fetch[data.role]);

			// В первый день поднимаем весь состав на ноги
			if (day == 1) {
				for (size_t i = 0; i < t_count; ++i) {
					LPTHREAD_START_ROUTINE func = (thread_data[i].role == Roles::R ? ThreadFuncR : ThreadFuncLS);

					hThread[i] = CreateThread(NULL, 0, func, (PVOID)&thread_data[i], 0, 0);

					if (!hThread[i]) {
						EnterCriticalSection(&cs_cout);
						cout << "Ошибка создания потока \"" << thread_data[i].getName() << "\"!" << endl;
						LeaveCriticalSection(&cs_cout);
					}
				}
			}

			Sleep(1000);

			EnterCriticalSection(&cs_cout);
			cout << "Полковник готов к началу нового дня." << endl;
			LeaveCriticalSection(&cs_cout);
			EnterSynchronizationBarrier(&sb_day, NULL);
			
			EnterCriticalSection(&cs_cout);
			cout << "Наступил день № " << day << "!" << endl;
			LeaveCriticalSection(&cs_cout);

			Sleep(1000);

			// Полковник создаёт поручение
			task_pull[data.role].gen();

			cout << task_pull[data.role] << endl;

			// И готов его отдать на обработку
			ReleaseMutex(h_pull[data.role]);

			// Полковник ожидает готовности поручения
			WaitMutex(h_fetch[data.bw]);
			dumpTask();

			// Получаем поручение
			task_fetch[data.role] = task_fetch[data.bw];
			ReleaseMutex(h_fetch[data.bw]);
			dumpTask();

			// Выводим результат дня
			EnterCriticalSection(&cs_cout);
			cout << task_pull[Roles::P] << endl;
			cout << task_fetch[Roles::P] << endl;
			LeaveCriticalSection(&cs_cout);
		}

		// В конце месяца ждём всех
		DWORD dw = WaitForMultipleObjects(1, hThread.data(), TRUE, INFINITE);

		// Полковник уходит в закат...
		DeleteCriticalSection(&cs_cout);
		DeleteSynchronizationBarrier(&sb_day);
		CloseHandle(h_pull[data.role]);
		CloseHandle(h_fetch[data.role]);
	}
}
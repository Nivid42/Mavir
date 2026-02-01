#include "Menu.h"

void Menu::ShowMenu()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	while (true)
	{
		std::cout << "\n========================================\n";
		std::cout << "[+] Mavir Antivirus Menu\n";
		std::cout << "========================================\n";
		std::wcout << L"[!] Background scan is running automatically.\n";
		std::cout << "[/] Options:\n";
		std::cout << "    [1] Static File Analysis (*WARNING!!!!, **Not implemented yet)\n";
		std::cout << "    [0] Exit\n";
		std::cout << "========================================\n";
		std::cout << "*Executes the File, therefor use it under safe circumstances (VM without Network leakage)" << "\n";
		std::cout << "**In development, most of the Parsing is already done but im working on the side on the heurstic" << "\n";
		std::cout << "========================================\n";

		std::cout << "Your choice: ";

		std::string userChoice;
		std::cin >> userChoice;

		if (userChoice == "0")
		{
			std::cout << "[+] Program is shutting down...\n";
			break;
		}
		else if (userChoice == "1")
		{
			std::cout << "[!] This feature is not implemented yet.\n";
		}
		else
		{
			std::wcout << L"[-] Invalid input. Please choose 0 or 1.\n";
		}
	}
}

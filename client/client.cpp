#include <iostream>
#include <windows.h>
#include <string>
#include <thread>
#include <future>
#include <limits>
#include <chrono>
#include <vector>

class PasswordCracker {
private:
	const std::string alphabet0 = "abcdefghijklmnopqrstuvwxyz'";
	const std::string alphabet1 = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ'0123456789";
	const std::string alphabet2 = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ'0123456789абвгдежзийклмнопрстуфхцчшщъыьэю€јЅ¬√ƒ≈∆«»… ЋћЌќѕ–—“”‘’÷„ЎўЏџ№Ёёя";

	std::vector<std::string> alphabets = { alphabet0, alphabet1, alphabet2 };

	bool BruteForce(HANDLE hPipe, const std::string& login, int numberOfAlphabet, int passLength) {
		const std::string alphabet = alphabets[numberOfAlphabet];

		for (int length = 1; length <= passLength; ++length) {
			std::string password(length, ' ');

			while (true) {
				int index = 0;
				while (index < length && password[index] == alphabet.back()) {
					password[index++] = alphabet.front();
				}

				if (index == length) {
					break;
				}

				password[index] = alphabet[alphabet.find(password[index]) + 1];

				std::string message = login + ":" + password;
				OVERLAPPED writeOverlapped;
				ZeroMemory(&writeOverlapped, sizeof(OVERLAPPED));
				writeOverlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

				if (WriteFile(hPipe, message.c_str(), message.length() + 1, nullptr, &writeOverlapped) || GetLastError() == ERROR_IO_PENDING) {
					WaitForSingleObject(writeOverlapped.hEvent, INFINITE);
				}

				CloseHandle(writeOverlapped.hEvent);

				char buffer[1024];
				DWORD bytesRead;
				if (ReadFile(hPipe, buffer, sizeof(buffer), &bytesRead, nullptr) && bytesRead > 0) {
					if (std::stoi(buffer) == 1) {
						std::cout << "Password found for " << message << " Alphabet number: " << numberOfAlphabet << std::endl;
						return true;
					}
				}
			}
		}
		return false;
	}

public:
	void PipeCreate(LPWSTR pipeName, const std::string& login, int indexOfAlphabet, int passLength) {
		auto startTime = std::chrono::high_resolution_clock::now();

		HANDLE hPipe = CreateFile(
			pipeName,
			GENERIC_READ | GENERIC_WRITE,
			0,
			nullptr,
			OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED,
			nullptr
		);

		if (hPipe == INVALID_HANDLE_VALUE) {
			std::cerr << "Failed to connect to the server. Error code: " << GetLastError() << std::endl;
			return;
		}

		std::cout << "Attempting to establish a connection for " << login << " with the server.\n";

		if (!BruteForce(hPipe, login, indexOfAlphabet, passLength)) {
			std::cout << "Password not found for login " << login << std::endl;
		}

		CloseHandle(hPipe);

		auto endTime = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
		std::cout << "Time taken for " << login << "  duration: " << duration << " ms" << std::endl << std::endl;
	}

	void PipeTest(LPWSTR pipeName, const std::string& login) {

		std::string message;
		std::cout << "Enter test login:password" << std::endl;
		std::cin >> message;

		auto startTime = std::chrono::high_resolution_clock::now();

		HANDLE hPipe = CreateFile(
			pipeName,
			GENERIC_READ | GENERIC_WRITE,
			0,
			nullptr,
			OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED,
			nullptr
		);

		if (hPipe == INVALID_HANDLE_VALUE) {
			std::cerr << "Failed to connect to the server. Error code: " << GetLastError() << std::endl;
			return;
		}

		OVERLAPPED writeOverlapped;
		ZeroMemory(&writeOverlapped, sizeof(OVERLAPPED));
		writeOverlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

		if (WriteFile(hPipe, message.c_str(), message.length() + 1, nullptr, &writeOverlapped) || GetLastError() == ERROR_IO_PENDING) {
			WaitForSingleObject(writeOverlapped.hEvent, INFINITE);
		}

		CloseHandle(writeOverlapped.hEvent);

		char buffer[1024];
		DWORD bytesRead;
		if (ReadFile(hPipe, buffer, sizeof(buffer), &bytesRead, nullptr) && bytesRead > 0) {
			if (std::stoi(buffer) == 1) {
				std::cout << "Password found: " << message << std::endl;
			}
		}

		DisconnectNamedPipe(hPipe);
		CloseHandle(hPipe);

		auto endTime = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
		std::cout << "Time taken for " << login << "  duration: " << duration << " ms" << std::endl << std::endl;
	}
};

int main() {

	LPWSTR namePipe1 = const_cast<LPWSTR>(L"\\\\.\\pipe\\MyNamedPipe1");

	PasswordCracker passwordCracker;

	int choice;

	while (true) {
		std::cout << "Select the operating mode:\n";
		std::cout << "1. Communication check\n";
		std::cout << "2. Hacking\n";

		std::cin >> choice;

		if (choice == 1) {
			auto future1 = std::async(std::launch::async, [namePipe1, &passwordCracker] {
				passwordCracker.PipeTest(namePipe1, "login1");
				});
		}
		else if (choice == 2) {
			std::string logins[] = { "login2", "login3", "login4", "login5" };
			int passLength = 10;
			int numberOfAlphabet = 0;

			std::cout << "Enter the maximum allowable password length: ";
			std::cin >> passLength;

			std::cout << "Alphabet 0 - small Latin letters and apostrophe\n";
			std::cout << "Alphabet 1 - small and capital Latin letters and apostrophe + numbers\n";
			std::cout << "Alphabet 2 - small and capital Latin letters and apostrophe + numbers + small and capital Cyrillic letters\n\n";
			std::cout << "Select the alphabet number (0, 1, or 2):";
			std::cin >> numberOfAlphabet;

			int loginsForHacking[4];

			for (int i = 0; i < 4; i++) {
				std::cout << "Hacking into login" << (i + 2) << "? (1 - hack; 0 - don't hack)" << std::endl;
				std::cin >> loginsForHacking[i];
			}

			std::vector<std::future<int>> futures;
			// создание асинхронно именнованых канала дл€ одновременного взлома нескольких логинов
			for (int i = 0; i < 4; ++i) {
				if (loginsForHacking[i] == 0)
					continue;

				std::future<int> fut = std::async(std::launch::async, [i, &passwordCracker, logins, numberOfAlphabet, passLength] {
					std::wstring pipeName = L"\\\\.\\pipe\\MyNamedPipe" + std::to_wstring(i + 2);
					passwordCracker.PipeCreate(const_cast<LPWSTR>(pipeName.c_str()), logins[i], numberOfAlphabet, passLength);
					return i;
					});
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				futures.push_back(std::move(fut));
			}

			for (auto& fut : futures) {
				fut.get();
			}

			int repeatChoice;
			std::cout << "Should I repeat the operation again? 1 - repeat; 0 - no\n";
			std::cin >> repeatChoice;

			if (repeatChoice == 1) {
				continue;
			}
			else {
				break;
			}
		}
		else {
			std::cout << "Wrong choice. Please select 1 or 2.\n";
			std::cin.clear();
			while (std::cin.get() != '\n');
		}
	}

	return 0;
}
#include <iostream>
#include <windows.h>
#include <string>
#include <thread>
#include <future>
#include <fstream>
#include <vector>
#include <chrono>

struct UserInfo {
    std::string login;
    std::string password;
};

class NamedPipeServer {
public:
    NamedPipeServer(const std::wstring& pipeName, const std::string& login, const std::string& password)
        : pipeName(pipeName), login(login), password(password) {}

    void PipeCreate() {
        bool shouldContinue = true;

        while (shouldContinue) { // экзотический костыль для работы тестового логина

            hPipe = CreateNamedPipe(
                pipeName.c_str(),
                PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                PIPE_UNLIMITED_INSTANCES,
                1024, 1024,
                NMPWAIT_USE_DEFAULT_WAIT,
                nullptr
            );

            if (hPipe == INVALID_HANDLE_VALUE) {
                std::cerr << "Failed to create a named channel. Error code: " << GetLastError() << std::endl;
                return;
            }

            OVERLAPPED overlapped;
            ZeroMemory(&overlapped, sizeof(OVERLAPPED));
            overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

            if (ConnectNamedPipe(hPipe, &overlapped) || GetLastError() == ERROR_IO_PENDING) {
                WaitForSingleObject(overlapped.hEvent, INFINITE);
                std::cout << "Client with login: " + login + " was connected." << std::endl;

                while (WaitForSingleObject(overlapped.hEvent, 0) == WAIT_OBJECT_0) {
                    char buffer[1024];
                    DWORD bytesRead;

                    if (ReadFile(hPipe, buffer, sizeof(buffer), &bytesRead, &overlapped) || GetLastError() == ERROR_IO_PENDING) {
                        DWORD waitResult = WaitForSingleObject(overlapped.hEvent, INFINITE);
                        if (waitResult == WAIT_OBJECT_0) {
                            std::string receivedData(buffer);
                            size_t pos = receivedData.find(':');
                            std::string receivedLogin = receivedData.substr(0, pos);
                            std::string receivedPassword = receivedData.substr(pos + 1);

                            if (receivedLogin == login && receivedPassword == password) {
                                const char* response = "1";
                                WriteFile(hPipe, response, strlen(response) + 1, nullptr, nullptr);
                                std::cout << "Client with login: " << login << " was disconnected." << std::endl;

                                if (login == "login1") {
                                    shouldContinue = true;
                                    break;
                                }
                                else {
                                    shouldContinue = false;
                                    break;
                                }
                            }
                            else {
                                const char* response = "0";
                                WriteFile(hPipe, response, strlen(response) + 1, nullptr, nullptr);
                            }
                        }
                    }
                }
            }
            else {
                std::cerr << "Failed to connect a named channel. Error code: " << GetLastError() << std::endl;
            }

            CloseHandle(overlapped.hEvent);
            CloseHandle(hPipe);
        }
    }

private:
    std::wstring pipeName;
    std::string login;
    std::string password;
    HANDLE hPipe;
};

int main() {

    OPENFILENAME ofn;
    char szFile[260];

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = (LPWSTR)szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"Text Files\0*.txt\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;

    std::vector<UserInfo> users;

    if (GetOpenFileName(&ofn) == TRUE) {
        std::ifstream file(ofn.lpstrFile);
        if (file.is_open()) {
            std::string dummy;
            std::getline(file, dummy);

            while (!file.eof()) {
                UserInfo user;
                file >> user.login >> user.password;
                users.push_back(user);
            }

            file.close();
        }
        else {
            std::cerr << "Unable to open file" << std::endl;
            return 1;
        }
    }
    else {
        std::cerr << "File selection canceled by user" << std::endl;
        return 1;
    }

    while (1) {
        std::future<int>* futures = new std::future<int>[users.size()];

        for (int i = 0; i < users.size(); ++i) {
            futures[i] = std::async(std::launch::async, [i, users] {
                std::wstring pipeName = L"\\\\.\\pipe\\MyNamedPipe" + std::to_wstring(i + 1);
                NamedPipeServer pipeServer(pipeName, users[i].login, users[i].password);
                pipeServer.PipeCreate();
                return i;
                });
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        std::cout << "The server is waiting for connections\n";

        for (int i = 1; i < users.size(); ++i) {
            futures[i].get();
        }


        int result = MessageBox(NULL, L"Continue working?", L"Shall I continue?", MB_YESNO | MB_ICONQUESTION);

        if (result != IDYES) {
            break;
        }
    }

    return 0;
}

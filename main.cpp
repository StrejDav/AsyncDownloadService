#include <iostream>
#include <string>
#include <string_view>
#include "DownloadService.hpp"

#define N_WORKER_THREADS 4U

int main() {
    constexpr std::string_view DOWNLOAD_COMMAND = "download";
    constexpr std::string_view QUIT_COMMAND = "quit";

    std::string input;
    std::string url;

    DownloadService service;
    service.startService(N_WORKER_THREADS);

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);

        if (QUIT_COMMAND == input) {
            break;
        }
        else if (input.find(DOWNLOAD_COMMAND) != std::string::npos) {
            url = input.substr(DOWNLOAD_COMMAND.length() + 1);
            service.pushToDownloadQueue(url);
        }
        else {
            std::cout << "Invalid command" << std::endl;
        }
    }

    service.stopService();

    return 0;
}

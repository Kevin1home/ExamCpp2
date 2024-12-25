#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_BUFFER_SIZE 1024
#define TIMEOUT 30

void execute_program(const std::vector<std::string>& command, int client_socket) {
    pid_t pid = fork();
    if (pid == -1) {
        std::string error_msg = "Error: failed to fork process.\n";
        send(client_socket, error_msg.c_str(), error_msg.size(), 0);
        exit(1);
    }

    if (pid == 0) {
        // child process
        std::vector<const char*> args;
        for (const auto& arg : command) {
            args.push_back(arg.c_str());
        }
        args.push_back(nullptr); // pushing back nullptr for execvp

        dup2(client_socket, STDOUT_FILENO);
        dup2(client_socket, STDERR_FILENO);

        if (execvp(command[0].c_str(), const_cast<char* const*>(args.data())) == -1) {
            std::string error_msg = "Error: " + std::string(strerror(errno)) + "\n";
            write(client_socket, error_msg.c_str(), error_msg.size());
            exit(1);
        }
    } else {
        // Parent process
        auto start_time = std::chrono::steady_clock::now();
        int status;
        while (true) {
            int wpid = waitpid(pid, &status, WNOHANG);
            if (wpid == pid) {
                if (WIFEXITED(status)) {
                    std::string exit_code_msg = "Exit Code: " + std::to_string(WEXITSTATUS(status)) + "\n";
                    send(client_socket, exit_code_msg.c_str(), exit_code_msg.size(), 0);
                } else if (WIFSIGNALED(status)) {
                    std::string exit_code_msg = "Exit Code: Signal " + std::to_string(WTERMSIG(status)) + "\n";
                    send(client_socket, exit_code_msg.c_str(), exit_code_msg.size(), 0);
                }
                break;
            }

            auto current_time = std::chrono::steady_clock::now();
            auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time);
            if (elapsed_time.count() >= TIMEOUT) {
                kill(pid, SIGKILL);
                std::string timeout_msg = "Timeout expired\n";
                send(client_socket, timeout_msg.c_str(), timeout_msg.size(), 0);
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        close(client_socket);
    }
}

void handle_client(int client_socket) {
    char buffer[MAX_BUFFER_SIZE];
    int received_len = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (received_len <= 0) {
        std::cerr << "Error receiving data from client\n";
        close(client_socket);
        return;
    }

    buffer[received_len] = '\0';

    std::string input(buffer);
    std::istringstream iss(input);
    std::vector<std::string> command;
    std::string token;

    while (iss >> token) {
        command.push_back(token);
    }

    if (command.empty()) {
        std::string error_msg = "Error: no command received\n";
        send(client_socket, error_msg.c_str(), error_msg.size(), 0);
        close(client_socket);
        return;
    }

    execute_program(command, client_socket);
}

void start_server(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd == -1) {
        std::cerr << "Could not create a socket!\n";
        exit(1);
    }

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr)); 
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Could not bind to address!\n";
        close(server_fd);
        exit(1);
    }

    if (listen(server_fd, 5) == -1) {
        std::cerr << "Cannot listen on socket!\n";
        close(server_fd);
        exit(1);
    }

    std::cout << "Server started on port " << port << "\n";

    while (true) {
        int client_socket = accept(server_fd, nullptr, nullptr);
        if (client_socket == -1) {
            std::cerr << "Cannot accept connections!\n";
            continue;
        }

        std::thread client_thread(handle_client, client_socket);
        client_thread.detach();
    }

    close(server_fd);
}

int main(int argc, char* argv[]) {
    if (2 != argc) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int port = std::stoi(argv[1]);
    start_server(port);

    return 0;
}

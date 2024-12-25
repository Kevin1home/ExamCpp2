#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <map>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 1024

std::map<std::string, std::ofstream> client_logs;
std::mutex log_mutex;

void handle_client(int client_socket, const std::string& client_ip) {
    char buffer[MAX_BUFFER_SIZE];
    int bytes_received;

    {
        std::lock_guard<std::mutex> lock(log_mutex);
        if (client_logs.find(client_ip) == client_logs.end()) {
            client_logs[client_ip] = std::ofstream(client_ip + ".log", std::ios::app);
        }
    }

    while ((bytes_received = recv(client_socket, buffer, MAX_BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';

        std::lock_guard<std::mutex> lock(log_mutex);
        client_logs[client_ip] << buffer;
        client_logs[client_ip].flush();
    }

    close(client_socket);
}

void start_server(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        return;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Binding failed");
    close(server_fd);
        return;}

    if (listen(server_fd, 5) == -1) {
        perror("Listen failed");
    close(server_fd);
    return;
    }

    std::cout << "Server started on port " << port << "\n";

    while (true) {
        sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_socket = accept(server_fd, (sockaddr*)&client_addr, &addr_len);

        if (client_socket == -1) {
            std::cerr << "Error accepting connection\n";
            continue;
        }

        std::string client_ip = inet_ntoa(client_addr.sin_addr);
        std::thread(handle_client, client_socket, client_ip).detach();
    }

    close(server_fd);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return 1;
    }

    int port = std::stoi(argv[1]);

    if (daemon(1, 1) == -1) {
        std::cerr << "Error starting as daemon\n";
        return 1;
    }

    start_server(port);
    return 0;
}

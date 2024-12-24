#include <iostream>
#include <arpa/inet.h>
#include <thread>
#include <unistd.h>
#include <cstring>

void receiveMessages(int sock_fd, char* self_message) {
    char buffer[1024];
    struct sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(sender_addr);

    while (true) {
        int nread = recvfrom(sock_fd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&sender_addr, &addr_len);
        if (nread > 0) {
            buffer[nread] = '\0';

            if (strcmp(self_message, buffer) != 0) {
                char sender_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(sender_addr.sin_addr), sender_ip, INET_ADDRSTRLEN);
                int sender_port = ntohs(sender_addr.sin_port);

                std::cout << "Message from [" << sender_ip << ":" << sender_port << "]: " << buffer << std::endl;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl; // every client should have the same port
        return 1;
    }

    int port = std::stoi(argv[1]);

    int sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_fd < 0) {
        std::cerr << "Could not create a socket!\n" << std::endl;
        return 1;
    }

    int broadcast_enable = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0) {
        std::cerr << "Could not enable broadcast option!\n" << std::endl;
        close(sock_fd);
        return 1;
    }

    struct sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(port);
    client_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock_fd, (const struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
        std::cerr << "Could not bind socket!\n" << std::endl;
        close(sock_fd);
        return 1;
    }

    struct sockaddr_in broadcast_addr;
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(port);
    broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    char message[1024] = {0};

    // extra thread for receiving messages
    std::thread receiver(receiveMessages, sock_fd, message);
    receiver.detach();

    // main thread for sending messages
    while (true) {
        std::cin.getline(message, sizeof(message));
        if (std::strcmp(message, "exit") == 0) {
            break;
        }

        int nwrite = sendto(sock_fd, message, std::strlen(message), 0, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr));
        if (nwrite < 0) {
            std::cerr << "Could not send message!\n" << std::endl;
        }
    }

    close(sock_fd);
    return 0;
}

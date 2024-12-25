#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#define MAX_BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server_ip> <port>" << std::endl;
        return 1;
    }

    std::string server_ip = argv[1];
    int port = std::stoi(argv[2]);

    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1) {
        std::cerr << "Could not create a socket!\n" << std::endl;
        return 1;
    }

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr)); 
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);
    server_addr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Could not connect to server!\n" << std::endl;
        close(sockfd);
        return 1;
    }

    std::string command;
    std::cout << "Enter command: ";
    std::getline(std::cin, command);

    send(sockfd, command.c_str(), command.length(), 0);

    std::string response;
    char buffer[MAX_BUFFER_SIZE];
    int bytes_received;
    while ((bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        response.append(buffer);
    }

    std::cout << "Server response: \n" << response << std::endl;

    close(sockfd);
    return 0;
}

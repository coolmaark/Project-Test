#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fstream>  // For file handling

#define PORT 12345  // Port to listen on

int main() {
    int sockfd;
    char buffer[1024];
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Failed to create socket\n";
        return -1;
    }

    // Configure server address
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        std::cerr << "Bind failed\n";
        close(sockfd);
        return -1;
    }

    std::cout << "UDP server listening on port " << PORT << "\n";

    // Main loop to receive data
    while (true) {
        len = sizeof(cliaddr);
        ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&cliaddr, &len);
        if (n < 0) {
            std::cerr << "Failed to receive data\n";
            continue;
        }
        
        buffer[n] = '\0';  // Null-terminate the received data
        std::cout << "Received: " << buffer << "\n";

        // Write the received JSON data to data.json
        std::ofstream outfile("data.json");
        if (outfile.is_open()) {
            outfile << buffer;
            outfile.close();
            std::cout << "Data saved to data.json\n";
        } else {
            std::cerr << "Error opening file to write\n";
        }
    }

    close(sockfd);
    return 0;
}
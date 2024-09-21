#include <iostream>
#include <cstring>
#include <csignal>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fstream>
#include <cstdlib>
#include <sys/time.h>
#include <jsoncpp/json/json.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/select.h>

#define PORT 12345

int sockfd;
bool running = true;

void signalHandler(int signum)
{
    std::cout << "\nInterrupt signal (" << signum << ") received.\n";
    running = false;
}

int main()
{
    char buffer[2048]; // Increased buffer size for larger JSON
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;

    signal(SIGINT, signalHandler);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        std::cerr << "Failed to create socket\n";
        return -1;
    }

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        std::cerr << "setsockopt(SO_REUSEADDR) failed\n";
        close(sockfd);
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        std::cerr << "Bind failed\n";
        close(sockfd);
        return -1;
    }

    std::cout << "UDP server listening on port " << PORT << "\n";

    while (running)
    {
        fd_set readfds;
        struct timeval timeout;
        timeout.tv_sec = 5; // Wait for 5 seconds
        timeout.tv_usec = 0;

        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        int activity = select(sockfd + 1, &readfds, nullptr, nullptr, &timeout);
        if (activity < 0)
        {
            std::cerr << "Select error: " << strerror(errno) << "\n";
            continue;
        }
        else if (activity == 0)
        {
            std::cout << "Waiting for data...\n";
            continue;
        }

        if (FD_ISSET(sockfd, &readfds))
        {
            len = sizeof(cliaddr);
            ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&cliaddr, &len);
            if (n < 0)
            {
                std::cerr << "recvfrom error: " << strerror(errno) << "\n";
                continue;
            }

            buffer[n] = '\0';                            // Ensure null termination
            std::cout << "Received: " << buffer << "\n"; // Debugging output

            // Parse JSON
            Json::Value newData;
            Json::CharReaderBuilder reader;
            std::string errs;
            std::istringstream ss(buffer);
            if (!Json::parseFromStream(reader, ss, &newData, &errs))
            {
                std::cerr << "Error parsing JSON data: " << errs << "\n"; // Print the error
                continue;
            }

            // Read existing data from the file
            Json::Value existingData;
            std::ifstream infile("data.json", std::ifstream::binary);
            if (infile.is_open())
            {
                infile >> existingData;
                infile.close();
            }

            // Append new data to existing data
            for (const auto &key : newData.getMemberNames())
            {
                for (const auto &value : newData[key])
                {
                    existingData[key].append(value);
                }
            }

            // Write updated data back to file
            std::ofstream outfile("data.json", std::ofstream::binary);
            if (outfile.is_open())
            {
                outfile << existingData;
                outfile.close();
                std::cout << "Data appended to data.json\n";
            }
            else
            {
                std::cerr << "Error opening file to write\n";
            }
        }
    }

    close(sockfd);
    std::cout << "\nServer stopped.\n";
    return 0;
}

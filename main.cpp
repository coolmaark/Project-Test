#include <iostream>
#include <fstream>
#include <csignal>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <nlohmann/json.hpp>
#include <vector>
#include <sys/wait.h>  // Include for wait
#include <fcntl.h>  // For fcntl to make socket non-blocking

using json = nlohmann::json;

using namespace std;  // Add this line to use the std namespace

vector<json> received_data;
bool running = true;

// Signal handler for Ctrl+C (SIGINT)
void signal_handler(int signal) {
    if (signal == SIGINT) {
        cout << "\nCtrl+C pressed. Saving data to data.json...\n";

        // Open the file for writing
        ofstream file("data.json");
        if (file.is_open()) {
            file << "[\n";
            for (size_t i = 0; i < received_data.size(); ++i) {
                file << received_data[i].dump(4);
                if (i != received_data.size() - 1) {
                    file << ",\n";
                }
            }
            file << "\n]";
            file.close();
            cout << "Data saved to data.json successfully.\n";
        } else {
            cerr << "Failed to open data.json for writing.\n";
        }

        // Run the generate_files command
        if (fork() == 0) {  // Create a new process
            execlp("./generate_files", "generate_files", "data.json", "output.pdf", nullptr);
            // If execlp fails
            cerr << "Failed to execute generate_files.\n";
            exit(1);
        } else {
            wait(nullptr);  // Wait for the child process to finish
        }

        running = false;  // Stop the main loop
    }
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[1024];
    socklen_t addr_len = sizeof(client_addr);

    // Register signal handler for Ctrl+C
    signal(SIGINT, signal_handler);

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cerr << "Failed to create socket.\n";
        return -1;
    }

    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(12345);

    // Bind the socket to port 12345
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        cerr << "Bind failed.\n";
        close(sockfd);
        return -1;
    }

    // Make the socket non-blocking
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        cerr << "Failed to get socket flags.\n";
        close(sockfd);
        return -1;
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        cerr << "Failed to set socket to non-blocking.\n";
        close(sockfd);
        return -1;
    }

    cout << "UDP server is listening on port 12345 (non-blocking mode)...\n";

    // Main loop to receive data
    while (running) {
        memset(buffer, 0, sizeof(buffer));
        int n = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&client_addr, &addr_len);
        
        if (n > 0) {
            buffer[n] = '\0';  // Null-terminate the buffer
            string received(buffer);

            try {
                // Parse received JSON and add to the list
                json data = json::parse(received);
                received_data.push_back(data);

                cout << "Received data: " << data.dump(4) << endl;
            } catch (json::parse_error& e) {
                cerr << "Error parsing JSON: " << e.what() << endl;
            }
        } else if (n == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
            // Handle error, except for "would block" or "try again" errors (which are expected in non-blocking sockets)
            cerr << "Error receiving data: " << strerror(errno) << endl;
        }

        // Small sleep to prevent busy looping when no data is available
        usleep(100000);  // Sleep for 100ms
    }

    // Close the socket
    close(sockfd);

    cout << "Server has been shut down.\n";
    return 0;
}

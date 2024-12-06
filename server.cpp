#include "server.hpp"
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <cstring>
#include <sstream>

// Error handling definition
#define Error(message) (std::cout << "ERROR! " << message << std::endl)
#define Error404 ("HTTP/1.1 404 Not Found\r\nContent-Length: 13\r\n\r\n404 Not Found")

void ServerHTTP::start(int port) {
    running = true; // Initialize running flag
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0); // Start socket
    if (serverSocket < 0) {
        Error("Failed to create socket.");
        return;
    }

    sockaddr_in serverAddress = {};             // Initialize serverAddress
    serverAddress.sin_port = htons(port);       // Set port to port entered
    serverAddress.sin_family = AF_INET;         // Set family to IPv4
    serverAddress.sin_addr.s_addr = INADDR_ANY; // Accept any IP

    // Bind server socket.
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        Error("Bind failed.");
        close(serverSocket); // Close serverSocket if bind fails
        return;
    }

    listen(serverSocket, SOMAXCONN); // Use SOMAXCONN for backlog

    // Start the batch worker threads.
    for (int i = 0; i < 4; i++) {
        BatchWorkers.emplace_back(&ServerHTTP::BatchWorker, this);
    }

    while (running) {
        int clientSocket = accept(serverSocket, nullptr, nullptr); // Accept clientSocket
        if (clientSocket < 0) {
            Error("Unable to connect to client.");
            continue;
        }
        {
            std::unique_lock<std::shared_mutex> lock(ClientMutex);
            clientQueue.push(clientSocket); // Add the clientSocket to the queue.
        }
        ConVar.notify_one(); // Notify the worker thread
    }

    // Close serverSocket at the end (if it ever reaches here)
    close(serverSocket);
}


void ServerHTTP::handle_request(const std::string& request, int clientSocket) {
    if (request.empty()) { // If request is empty, throw error
        Error("Request is empty.");
        make_response(Error404, clientSocket);
        return;
    }

    // Break down the requests into lines
    std::istringstream request_stream(request);
    std::string request_line;
    std::getline(request_stream, request_line);

    // Get method and path of the request line
    std::string method, path, version;
    std::istringstream request_line_stream(request_line);
    request_line_stream >> method >> path >> version;

    std::string response = route(method, path); // Assuming route returns a response string
    make_response(response, clientSocket);
}

void ServerHTTP::BatchWorker() {
    while (true) {
        int clientSocket;
        {
            std::unique_lock<std::shared_mutex> lock(ClientMutex); // Correct
            ConVar.wait(lock, [this] { return !clientQueue.empty(); }); // Wait for clientSocket.
            clientSocket = clientQueue.front();
            clientQueue.pop();
        }
        char buffer[3000] = {0}; // Declare a buffer to store the request
        ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1); // Read the buffer

        if (bytesRead < 0) { // If read was unsuccessful
            Error("Failed to read buffer.");
            close(clientSocket); // Close client socket on error
            continue;
        }

        std::string request(buffer, bytesRead); // Convert buffer to std::string.
        handle_request(request, clientSocket); // Pass clientSocket to handle_request
    }
}

void ServerHTTP::make_response(const std::string& response, int clientSocket) {
    if (response.empty()) { // If response is empty, throw error
        Error("Message is empty.");
        send(clientSocket, Error404, strlen(Error404), 0);
        return;
    }

    // Send response to client
    send(clientSocket, response.c_str(), response.length(), 0);
}

void ServerHTTP::client_connection(int clientSocket) {
    // This method is no longer needed since processing is done in BatchWorker
    close(clientSocket); // Close the clientSocket when response has been made
}

#ifndef  SERVER_HPP
#define SERVER_HPP

#include "routes.h" //Include the route.c file
#include <string> //Include string for storing char arrays.
#include <unordered_map> //Include unordered_map for storing routes.
#include <vector> //Include vector framework (storing clientsockets)
#include <queue> //Include the queue framework (multithreading)
#include <mutex> // Include the mutex framework (multithreading)
#include <shared_mutex> // Include add-on for mutex framework
#include <condition_variable> //Include the condition variable framework.
#include <thread> // Include the thread framework (multithreading).


class ServerHTTP {
    private:
        volatile bool running;
        std::unordered_map<std::string, std::string> parsers; //Used for initially.
        std::shared_mutex ClientMutex; //Shared mutex.
        std::queue<int> clientQueue; //Store all waiting client sockets.
        std::condition_variable ConVar; //Condition variable.
        std::vector<std::thread> BatchWorkers; //Stores all batch workers.
        

    public:
        void start(int port);
        void BatchWorker();
        void handle_request(const std::string& message, int clientSocket);
        void make_response(const std::string& response, int clientSocket);
        void client_connection(int clientSocket);
};

#endif

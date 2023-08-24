/*
This is a fastfood restoraunt server code, that accepts connections from clients. (I have written Android app for client, check it out)
Giorgi Gvichiani (C)2023 Georgia
*/

#include <iostream>
#include <winsock2.h> // for networking and socket programming, don't forget to link tis to your project
#include <thread> // so that server can handle multiple clients
#include <queue> // for orders
#include <sstream> // Include this header for stringstream
#include <ws2tcpip.h> // For socket functions like inet_pton, so that we can specify ip. if you use "INADDR_ANY" then this header isn't neaded 
#include <mutex> // Include this header for std::mutex
#include <map>

std::map<int, std::string> id_name_map;



char clientID = '0'; // each client will have an id
struct order_structure {
    char client_id;
    int ord[4] = {0,0,0,0};
};
std::queue<order_structure> Order_Queue;

// Declare a mutex to protect the Order_Queue
std::mutex queueMutex;


void HandleClient(SOCKET clientSocket,char clientid) {

    int bytesSent = send(clientSocket, &clientid, sizeof(clientid), 0);
    if (bytesSent != sizeof(clientid)) {
        std::cerr << "Error sending client ID." << std::endl;
        closesocket(clientSocket);
        return;
    }
    

    
    int receivedOrder[4]; // 
    for (int i = 0; i < 4; i++) {
        char ch;
        int bytesReceived = recv(clientSocket, &ch, 1, 0); // Receive as bytes
        if (bytesReceived > 0) {
            receivedOrder[i] = ch - '0';
        }
    }
    
    order_structure new_order;
    new_order.client_id = clientid;
    new_order.ord[0] = receivedOrder[0];
    new_order.ord[1] = receivedOrder[1];
    new_order.ord[2] = receivedOrder[2];
    new_order.ord[3] = receivedOrder[3];

    // Lock the mutex to protect the shared resource
    std::lock_guard<std::mutex> lock(queueMutex);

    std::cout << "Client with id " << clientid << " Ordered :\n";
    for (int i = 0; i < 4; i++) {
        std::cout << id_name_map[i]<<" quantity : "<< new_order.ord[i] <<'\n';
    }
    // Add new_order to the Order_Queue
    Order_Queue.push(new_order);
    std::cout << "order was added to the queue , size of queue : " << Order_Queue.size()<<'\n';

    // Mutex is automatically released when 'lock' goes out of scope
  
    closesocket(clientSocket);
}

int main() {
    //crate a map for order:
    id_name_map[0] = "Pizza";
    id_name_map[1] = "Buger";
    id_name_map[2] = "HotDog";
    id_name_map[3] = "CocaCola";

    // 1)initialize WSDATA
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        return 1;
    }
    std::cout << "WSA Initialized!\n";


    // 2)create a server socket , here we specify sockets transportation protocol (TCP or UDP)
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket." << std::endl;
        WSACleanup();
        return 1;
    }
    std::cout << "Socket was created!\n";

    //bind a server socket, here we specify port and ip addres we want server to bind in next step
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12345);
    serverAddress.sin_addr.s_addr = INADDR_ANY;// this one will automaticly choose the internet adapter and ip

    /* if we want specific ip :
    if (inet_pton(AF_INET, "192.168.137.1", &serverAddress.sin_addr) <= 0) {
        perror("Invalid address");
        return 1; // or handle the error in a way that's appropriate for your program
    } */

    // 3)bind a socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind socket." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    std::cout << "Socket was binded!\n";

    // 4) starting to listen for connections
    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        std::cerr << "Failed to listen on the socket." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    std::cout << "Listening for client connections...\n";

    while (true) {
        // 5) accepting each connection, and assigning each cleant one worker thread
        sockaddr_in clientAddress;
        int clientAddressSize = sizeof(clientAddress);
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressSize);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Failed to accept client connection." << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        std::cout << "Connection was created, preparing thread for the client...\n";

        std::thread clientThread(HandleClient, clientSocket,clientID);
        if (clientID == '9')clientID = '0';
        else clientID++;
        clientThread.detach();
    }

    //7) disconnect (theoreticly we will never reach tis part, because of infinite while loop);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}

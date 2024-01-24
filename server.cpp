#include <iostream>
#include <unordered_map>
#include <string>
#include <bits/stdc++.h>
#include <unistd.h>
#include <arpa/inet.h>
using namespace std;

struct Part {
    string partName;
    float partPrice;
    int partQuantity;
    string partDescription;
};
 
unordered_map<int, Part>inventory;
 
void initializeInventory() {
    inventory = {
        {1, {"IC Motor", 40000, 100, "400 lb-ft Petrol Engine"}},
        {2, {"Brake Pads", 20000, 50, "Export Quality Brake Pads"}},
        {3, {"Box Gear", 35000.50, 30, "Manual 7 Wheel Gear"}},
        {4, {"Recliner Seats", 40, 25, "Breathable Fibre Recliner Seats"}},
        {5, {"Air Filter", 15, 80, "High Efficiency Air Filter"}},
        {6, {"Engine", 90000, 90, "500 lb-ft Engine"}}
    };
}
 
void searchPart(int partNumber, int clientSocket) {
    if (inventory.find(partNumber) != inventory.end()) {
        string result = "Part details for part number " + to_string(partNumber) + ":\n";
        result += "Name: " + inventory[partNumber].partName + "\n";
        result += "Price: " + to_string(inventory[partNumber].partPrice) + "\n";
        result += "Quantity: " + to_string(inventory[partNumber].partQuantity) + "\n";
        result += "Description: " + inventory[partNumber].partDescription + "\n";
        send(clientSocket, result.c_str(), result.size(), 0);
    } else {
        string notFoundMsg = "Part with part number " + to_string(partNumber) + " not found.\n";
        send(clientSocket, notFoundMsg.c_str(), notFoundMsg.size(), 0);
    }
}
 
string retrievePartName(int partNumber) {
    if (inventory.find(partNumber) != inventory.end()) {
        return inventory[partNumber].partName;
    } 
    else {
        return "Part Not Found";
    }
}
 
int getPartQuantity(int partNumber) {
    if (inventory.find(partNumber) != inventory.end()) {
        return inventory[partNumber].partQuantity;
    } 
    else {
        return 0;
    }
}
 
void orderPart(int partNumber) {
    if (inventory.find(partNumber) != inventory.end() && inventory[partNumber].partQuantity > 0) {
        inventory[partNumber].partQuantity--;
        cout << "Ordered part with part number " << partNumber << endl;
    } else {
        cout << "Part with part number " << partNumber << " not available." << endl;
    }
}
 
int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <ip> <port>\n";
        return 1;
    }
 
    const char* ip = argv[1];
    int port = stoi(argv[2]);
 
    initializeInventory();
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Error creating socket");
        return 1;
    }
 
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
 
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Error binding socket");
        close(serverSocket);
        return 1;
    }
 
    if (listen(serverSocket, 2) == -1) {
        perror("Error listening for connections");
        close(serverSocket);
        return 1;
    }
 
    cout << "Server is listening for connections on port " << port << "...\n";
 
    while (true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        cout << "Client connected.\n";
 
        // Receive command from the client
        char buffer[256];
        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            cerr << "Error receiving command from client.\n";
            close(clientSocket);
            continue;
        }
 
        // Parse the command and execute corresponding server-side procedure
        string command(buffer, bytesRead);
        int partNumber;
 
        if (sscanf(buffer, "SEARCH %d", &partNumber) == 1) {
            searchPart(partNumber, clientSocket);
        } else if (sscanf(buffer, "RETRIEVE %d", &partNumber) == 1) {
            string partName = retrievePartName(partNumber);
            send(clientSocket, partName.c_str(), partName.size(), 0);
        } else if (sscanf(buffer, "AVAILABILITY %d", &partNumber) == 1) {
            int quantity = getPartQuantity(partNumber);
            send(clientSocket, &quantity, sizeof(quantity), 0);
        } else if (sscanf(buffer, "ORDER %d", &partNumber) == 1) {
            orderPart(partNumber);
        } else if (command.substr(0, 4) == "EXIT") {
            close(clientSocket);
            break;
        } else {
            cerr << "Invalid command received.\n";
        }
 
        close(clientSocket);  // Close the connection with the client
    }

    close(serverSocket);
 
    return 0;
}

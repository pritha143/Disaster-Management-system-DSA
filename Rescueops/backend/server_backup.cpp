
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "dsa.h"

using namespace std;

// =====================================
// GLOBAL DATA STRUCTURES
// =====================================

VictimList victimList;
RescueQueue normalQueue;
EmergencyQueue emergencyQueue;
DisasterGraph disasterGraph;

vector<Incident> incidents;

int nextId = 1;
// VOLUNTEER MANAGEMENT
struct Volunteer {
    int id;
    string name;
    string location;
    bool available;
    int assignedIncident;
};

vector<Volunteer> volunteers;
int nextVolunteerId = 1;

// =====================================
// LOAD SAVED INCIDENTS
// =====================================

void loadIncidents() {
    ifstream file("backend/data/incidents.txt");

    if (!file.is_open()) {
        cout << "No previous incident file found." << endl;
        return;
    }

    string line;

    while (getline(file, line)) {
        stringstream ss(line);
        string idText, name, type, priorityText;

        if (getline(ss, idText, '|') &&
            getline(ss, name, '|') &&
            getline(ss, type, '|') &&
            getline(ss, priorityText)) {

            try {
                Incident x;
                x.id = stoi(idText);
                x.name = name;
                x.type = type;
                x.priority = stoi(priorityText);

                incidents.push_back(x);
                victimList.insert(x);
                normalQueue.add(x);
                emergencyQueue.add(x);

                if (x.id >= nextId)
                    nextId = x.id + 1;
            }
            catch (...) {
                cout << "Skipping invalid incident record." << endl;
            }
        }
    }

    file.close();

    cout << "Saved incidents loaded: "
         << incidents.size() << endl;
}

// =====================================
// SAVE VOLUNTEERS
void saveVolunteers() {
    ofstream file("backend/data/volunteers.txt");

    for (const auto &v : volunteers) {
        file << v.id << "|"
             << v.name << "|"
             << v.location << "|"
             << (v.available ? 1 : 0) << "|"
             << v.assignedIncident << endl;
    }

    file.close();
}

// LOAD VOLUNTEERS
void loadVolunteers() {
    ifstream file("backend/data/volunteers.txt");

    if (!file.is_open()) {
        cout << "No volunteer file found." << endl;
        return;
    }

    string line;

    while (getline(file, line)) {
        stringstream ss(line);
        string idText, name, location, availableText, incidentText;

        if (getline(ss, idText, '|') &&
            getline(ss, name, '|') &&
            getline(ss, location, '|') &&
            getline(ss, availableText, '|') &&
            getline(ss, incidentText)) {
            try {
                Volunteer v;
                v.id = stoi(idText);
                v.name = name;
                v.location = location;
                v.available = (stoi(availableText) == 1);
                v.assignedIncident = stoi(incidentText);

                volunteers.push_back(v);
        

                if (v.id >= nextVolunteerId)
                    nextVolunteerId = v.id + 1;
            }
            catch (...) {
                cout << "Skipping invalid volunteer record." << endl;
            }
        }
    }

    file.close();
    cout << "Saved volunteers loaded: " << volunteers.size() << endl;
}
// READ SIMPLE JSON VALUES
// =====================================

string getString(string body, string key) {
    string search = "\"" + key + "\"";

    size_t pos = body.find(search);

    if (pos == string::npos)
        return "";

    pos = body.find(':', pos);

    if (pos == string::npos)
        return "";

    pos = body.find('"', pos);

    if (pos == string::npos)
        return "";

    size_t end = body.find('"', pos + 1);

    if (end == string::npos)
        return "";

    return body.substr(pos + 1, end - pos - 1);
}

int getNumber(string body, string key, int defaultValue) {
    string search = "\"" + key + "\"";

    size_t pos = body.find(search);

    if (pos == string::npos)
        return defaultValue;

    pos = body.find(':', pos);

    if (pos == string::npos)
        return defaultValue;

    try {
        return stoi(body.substr(pos + 1));
    }
    catch (...) {
        return defaultValue;
    }
}

// =====================================
// JSON ESCAPING
// =====================================

string escapeJSON(string s) {
    string result;

    for (char c : s) {
        if (c == '"')
            result += "\\\"";
        else if (c == '\\')
            result += "\\\\";
        else if (c == '\n')
            result += "\\n";
        else if (c == '\r')
            result += "\\r";
        else
            result += c;
    }

    return result;
}

// =====================================
// FILE HANDLING
// =====================================

void saveIncident(Incident x) {

    ofstream file("backend/data/incidents.txt", ios::app);

    if (file.is_open()) {

        file << x.id << "|"
             << x.name << "|"
             << x.type << "|"
             << x.priority << endl;

        file.close();
    }
}

// =====================================
// DISPLAY ALL INCIDENTS
// =====================================

string getAllIncidents() {

    string result = "{\"incidents\":[";

    for (int i = 0; i < (int)incidents.size(); i++) {

        Incident x = incidents[i];

        result += "{";

        result += "\"id\":" + to_string(x.id) + ",";

        result += "\"name\":\""
                  + escapeJSON(x.name) + "\",";

        result += "\"type\":\""
                  + escapeJSON(x.type) + "\",";

        result += "\"priority\":"
                  + to_string(x.priority);

        result += "}";

        if (i != (int)incidents.size() - 1)
            result += ",";
    }

    result += "]}";

    return result;
}

// =====================================
// API REQUEST HANDLER
// =====================================

string handleRequest(string method,
                     string path,
                     string body) {

    // HEALTH CHECK
    if (method == "GET" && path == "/api/health") {

        return
        "{\"status\":\"running\","
        "\"backend\":\"C++\","
        "\"project\":\"Rescue Ops\"}";
    }

    // GET INCIDENTS
    if (method == "GET" && path == "/api/incidents") {

        return getAllIncidents();
    }

    // SUBMIT SOS
    if (method == "POST" && path == "/api/sos") {

        string name = getString(body, "name");

        string type = getString(body, "type");

        int priority = getNumber(body, "priority", 1);

        if (name.empty() || type.empty()) {

            return
            "{\"success\":false,"
            "\"message\":\"Name and type are required\"}";
        }

        if (priority < 1)
            priority = 1;

        if (priority > 5)
            priority = 5;

        Incident x;

        x.id = nextId++;

        x.name = name;

        x.type = type;

        x.priority = priority;

        // LINKED LIST
        victimList.insert(x);

        // NORMAL QUEUE
        normalQueue.add(x);

        // PRIORITY QUEUE
        emergencyQueue.add(x);

        // VECTOR
        incidents.push_back(x);

        // FILE STORAGE
        saveIncident(x);

        cout << "\nNEW SOS RECEIVED!" << endl;

        cout << "ID: " << x.id << endl;

        cout << "Victim: " << x.name << endl;

        cout << "Emergency: " << x.type << endl;

        cout << "Priority: " << x.priority << endl;

        return
        "{\"success\":true,"
        "\"message\":\"SOS submitted successfully\","
        "\"id\":" + to_string(x.id) + "}";
    }

    // BFS
    if (method == "GET" && path == "/api/bfs") {

        cout << "\nBFS ROUTE:" << endl;

        disasterGraph.BFS(0, 5);

        return
        "{\"success\":true,"
        "\"algorithm\":\"BFS\","
        "\"message\":\"Check server terminal for route\"}";
    }

    // DFS
    if (method == "GET" && path == "/api/dfs") {

        cout << "\nDFS EXPLORATION:" << endl;

        disasterGraph.DFS(0);

        return
        "{\"success\":true,"
        "\"algorithm\":\"DFS\","
        "\"message\":\"Check server terminal for traversal\"}";
    }

    // PROCESS HIGHEST PRIORITY
    if (method == "POST" && path == "/api/process") {

        cout << "\nPROCESSING EMERGENCY:" << endl;

        string processed = emergencyQueue.process();

        if (processed.empty()) {
            return "{\"success\":false,\"message\":\"No emergencies in queue\"}";
        }

        return
        "{\"success\":true,"
        "\"message\":\"Highest priority request processed\","
        "\"incident\":\"" + escapeJSON(processed) + "\"}";
    }

  // PROCESS NORMAL FIFO QUEUE
    if (method == "POST" && path == "/api/process-normal") {

        string processed = normalQueue.process();

        if (processed.empty()) {
            return "{\"success\":false,\"message\":\"Normal queue is empty\"}";
        }

        return
        "{\"success\":true,"
        "\"message\":\"Normal rescue request processed\","
        "\"incident\":\"" + escapeJSON(processed) + "\"}";
    }
// REGISTER VOLUNTEER
    if (method == "POST" && path == "/api/volunteers") {
        string name = getString(body, "name");
        string location = getString(body, "location");

        if (name.empty() || location.empty()) {
            return "{\"success\":false,\"message\":\"Name and location are required\"}";
        }

        Volunteer v;
        v.id = nextVolunteerId++;
        v.name = name;
        v.location = location;
        v.available = true;
        v.assignedIncident = -1;

        volunteers.push_back(v);
        saveVolunteers();

        return
        "{\"success\":true,"
        "\"message\":\"Volunteer registered\","
        "\"id\":" + to_string(v.id) + "}";
    }

    // GET VOLUNTEERS
    if (method == "GET" && path == "/api/volunteers") {
        string result = "{\"volunteers\":[";
        for (size_t i = 0; i < volunteers.size(); i++) {
            if (i > 0) result += ",";

            Volunteer v = volunteers[i];
            result += "{\"id\":" + to_string(v.id) +
                ",\"name\":\"" + escapeJSON(v.name) +
                "\",\"location\":\"" + escapeJSON(v.location) +
                "\",\"available\":" + string(v.available ? "true" : "false") +
                ",\"assignedIncident\":" + to_string(v.assignedIncident) + "}";
        }
        result += "]}";
        return result;
    }

    // ASSIGN VOLUNTEER TO INCIDENT
    if (method == "POST" && path == "/api/assign") {
        int incidentId = getNumber(body, "incidentId", -1);

        int position = searchIncident(incidents, incidentId);

        if (position == -1) {
            return "{\"success\":false,\"message\":\"Incident not found\"}";
        }

        for (auto &v : volunteers) {
            if (v.available) {
                v.available = false;
                v.assignedIncident = incidentId;
                saveVolunteers();

                return
                "{\"success\":true,"
                "\"message\":\"Volunteer assigned successfully\","
                "\"volunteerId\":" + to_string(v.id) +
                ",\"volunteer\":\"" + escapeJSON(v.name) +
                "\",\"incidentId\":" + to_string(incidentId) + "}";
            }
        }

        return "{\"success\":false,\"message\":\"No available volunteers\"}";
    }
// SEARCH INCIDENT
if (method == "GET" && path.find("/api/search") == 0) {

    int id = -1;

    size_t pos = path.find("id=");

    if (pos != string::npos) {
        id = stoi(path.substr(pos + 3));
    }

    int position = searchIncident(incidents, id);

    if (position == -1) {
        return "{\"found\":false}";
    }

    Incident item = incidents[position];

    return "{\"found\":true,\"id\":" + to_string(item.id) +
           ",\"name\":\"" + escapeJSON(item.name) +
           "\",\"type\":\"" + escapeJSON(item.type) +
           "\",\"priority\":" + to_string(item.priority) + "}";
}
    

    // SORT INCIDENTS
    if (method == "GET" && path == "/api/sort") {

        sortEmergencies(incidents);

        return getAllIncidents();
    }

    return "{\"error\":\"Endpoint not found\"}";
}

// =====================================
// SEND HTTP RESPONSE
// =====================================

void sendResponse(SOCKET client, string data) {

    string response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json; charset=utf-8\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Content-Length: " + to_string(data.size()) +
        "\r\nConnection: close\r\n\r\n" + data;

    send(client, response.c_str(),
         (int)response.size(), 0);
}

// =====================================
// MAIN SERVER
// =====================================

int main() {

    loadIncidents();
    loadVolunteers();

    WSADATA wsa;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {

        cout << "Winsock initialization failed!" << endl;

        return 1;
    }

    SOCKET serverSocket = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (serverSocket == INVALID_SOCKET) {

        cout << "Socket creation failed!" << endl;

        WSACleanup();

        return 1;
    }

    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;

    serverAddress.sin_port = htons(8088);

    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket,
             (sockaddr*)&serverAddress,
             sizeof(serverAddress)) == SOCKET_ERROR) {

        cout << "Port 8088 is unavailable!" << endl;

        closesocket(serverSocket);

        WSACleanup();

        return 1;
    }

    if (listen(serverSocket, 10) == SOCKET_ERROR) {

        cout << "Server listen failed!" << endl;

        closesocket(serverSocket);

        WSACleanup();

        return 1;
    }

    cout << "\n================================" << endl;

    cout << "       RESCUE OPS SERVER" << endl;

    cout << "================================" << endl;

    cout << "C++ Backend: RUNNING" << endl;

    cout << "Port: 8088" << endl;

    cout << "================================\n" << endl;

    while (true) {

        SOCKET client = accept(
            serverSocket,
            NULL,
            NULL
        );

        if (client == INVALID_SOCKET)
            continue;

        string request;

        char buffer[4096];

        int received;

        while ((received = recv(
            client,
            buffer,
            sizeof(buffer),
            0
        )) > 0) {

            request.append(buffer, received);

            size_t headerEnd =
                request.find("\r\n\r\n");

            if (headerEnd != string::npos) {

                int contentLength = 0;

                size_t lengthPos =
                    request.find("Content-Length:");

                if (lengthPos != string::npos) {

                    contentLength = stoi(
                        request.substr(lengthPos + 15)
                    );
                }

                size_t bodyStart = headerEnd + 4;

                if (request.size() >=
                    bodyStart + contentLength) {

                    break;
                }
            }
        }

        size_t firstSpace = request.find(' ');

        size_t secondSpace =
            request.find(' ', firstSpace + 1);

        if (firstSpace == string::npos ||
            secondSpace == string::npos) {

            sendResponse(client, "{\"error\":\"Invalid request\"}");

            closesocket(client);

            continue;
        }

        string method =
            request.substr(0, firstSpace);

        string path =
            request.substr(
                firstSpace + 1,
                secondSpace - firstSpace - 1
            );


        size_t headerEnd =
            request.find("\r\n\r\n");

        string body = "";

        if (headerEnd != string::npos) {

            body = request.substr(headerEnd + 4);
        }

        string result;

        if (method == "OPTIONS") {

            result = "{}";
        }
        else {

            result = handleRequest(
                method,
                path,
                body
            );
        }

        sendResponse(client, result);

        closesocket(client);
    }

    closesocket(serverSocket);

    WSACleanup();

    return 0;
}


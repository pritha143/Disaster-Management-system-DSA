#include <winsock2.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <string>
#include <iomanip>
#include <limits>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <cstdlib>

using namespace std;

struct Volunteer
{
    int id;
    string name;
    string phone;
    string password;
    double latitude;
    double longitude;
};

struct SOS
{
    int sosId;
    string type;
    int priority;
    double latitude;
    double longitude;
};

struct CompareSOS
{
    bool operator()(const SOS &a, const SOS &b) const
    {
        if (a.priority != b.priority)
        {
            return a.priority > b.priority;
        }
        return a.sosId > b.sosId;
    }
};

string escapeJson(const string &input)
{
    string out;
    for (char c : input)
    {
        if (c == '\\') out += "\\\\";
        else if (c == '"') out += "\\\"";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else out += c;
    }
    return out;
}

class VolunteerModule
{
private:
    vector<Volunteer> volunteers;
    priority_queue<SOS, vector<SOS>, CompareSOS> sosQueue;
    int nextSosId = 101;

    double distanceKm(double lat1, double lon1, double lat2, double lon2) const
    {
        constexpr double kEarthRadiusKm = 6371.0;
        constexpr double kPi = 3.14159265358979323846;
        constexpr double kDegToRad = kPi / 180.0;

        double dLat = (lat2 - lat1) * kDegToRad;
        double dLon = (lon2 - lon1) * kDegToRad;
        double rLat1 = lat1 * kDegToRad;
        double rLat2 = lat2 * kDegToRad;

        double sinHalfLat = sin(dLat / 2.0);
        double sinHalfLon = sin(dLon / 2.0);
        double a = sinHalfLat * sinHalfLat + cos(rLat1) * cos(rLat2) * sinHalfLon * sinHalfLon;
        double c = 2.0 * atan2(sqrt(a), sqrt(max(0.0, 1.0 - a)));
        return kEarthRadiusKm * c;
    }

    void saveActiveSOS()
    {
        ofstream file("active_sos.txt");
        priority_queue<SOS, vector<SOS>, CompareSOS> temp = sosQueue;

        while (!temp.empty())
        {
            SOS s = temp.top();
            temp.pop();
            file << s.sosId << "|" << s.type << "|" << s.priority << "|" << fixed << setprecision(6) << s.latitude << "|" << fixed << setprecision(6) << s.longitude << "\n";
        }
        file.close();
    }

    void saveAllVolunteers() const
    {
        ofstream file("volunteers.txt");
        for (const auto &v : volunteers)
        {
            file << v.id << "|" << v.name << "|" << v.phone << "|" << v.password << "|" << fixed << setprecision(6) << v.latitude << "|" << fixed << setprecision(6) << v.longitude << "\n";
        }
        file.close();
    }

    void appendResolvedSOS(const SOS &s, const string &resolvedBy) const
    {
        ofstream file("resolved_sos.txt", ios::app);
        file << s.sosId << "|" << s.type << "|" << s.priority << "|" << fixed << setprecision(6) << s.latitude << "|" << fixed << setprecision(6) << s.longitude << "|" << resolvedBy << "\n";
        file.close();
    }

    void loadVolunteers()
    {
        ifstream file("volunteers.txt");
        string line;

        while (getline(file, line))
        {
            if (line.empty()) continue;
            stringstream ss(line);
            string id, name, phone, password, latStr, lonStr;

            if (getline(ss, id, '|') && getline(ss, name, '|') && getline(ss, phone, '|') && getline(ss, password, '|') && getline(ss, latStr, '|') && getline(ss, lonStr))
            {
                try
                {
                    Volunteer v;
                    v.id = stoi(id);
                    v.name = name;
                    v.phone = phone;
                    v.password = password;
                    v.latitude = stod(latStr);
                    v.longitude = stod(lonStr);
                    volunteers.push_back(v);
                }
                catch (...) {}
            }
        }
        file.close();
    }

    void loadActiveSOS()
    {
        ifstream file("active_sos.txt");
        string line;

        while (getline(file, line))
        {
            if (line.empty()) continue;
            stringstream ss(line);
            string id, type, priorityStr, lat, lon;

            if (getline(ss, id, '|') && getline(ss, type, '|') && getline(ss, priorityStr, '|') && getline(ss, lat, '|') && getline(ss, lon))
            {
                try
                {
                    SOS s;
                    s.sosId = stoi(id);
                    s.type = type;
                    s.priority = stoi(priorityStr);
                    s.latitude = stod(lat);
                    s.longitude = stod(lon);
                    sosQueue.push(s);
                    if (s.sosId >= nextSosId)
                    {
                        nextSosId = s.sosId + 1;
                    }
                }
                catch (...) {}
            }
        }
        file.close();
    }

    void printNearestVolunteer(double sosLat, double sosLon) const
    {
        if (volunteers.empty())
        {
            cout << "   -> No valid volunteers loaded to compute nearest responder.\n";
            return;
        }

        const Volunteer *nearest = nullptr;
        double minDist = numeric_limits<double>::infinity();

        for (const auto &v : volunteers)
        {
            double dist = distanceKm(v.latitude, v.longitude, sosLat, sosLon);
            if (dist < minDist)
            {
                minDist = dist;
                nearest = &v;
            }
        }

        if (nearest != nullptr)
        {
            cout << "   -> Nearest Volunteer: " << nearest->name << " (ID: " << nearest->id << ", Phone: " << nearest->phone << ", " << fixed << setprecision(2) << minDist << " km away)\n";
        }
    }

public:
    VolunteerModule()
    {
        loadVolunteers();
        loadActiveSOS();

        if (sosQueue.empty())
        {
            addIncomingSOS("Fire / Hazard", 1, 30.289627, 77.997382);
            addIncomingSOS("Medical", 2, 30.267037, 78.081926);
        }
    }

    bool registerVolunteer(const string &name, const string &phone, const string &password, double lat, double lon, Volunteer &outCreated, string &errMsg)
    {
        if (name.empty() || phone.empty() || password.empty())
        {
            errMsg = "Name, phone, and password are required.";
            return false;
        }

        int maxId = 100;
        for (const auto &v : volunteers)
        {
            if (v.id > maxId) maxId = v.id;
            if (v.name == name)
            {
                errMsg = "Volunteer name already registered. Try logging in.";
                return false;
            }
        }

        Volunteer v;
        v.id = maxId + 1;
        v.name = name;
        v.phone = phone;
        v.password = password;
        v.latitude = lat;
        v.longitude = lon;

        volunteers.push_back(v);
        saveAllVolunteers();
        outCreated = v;

        cout << "\n[VOLUNTEER REGISTERED] ID #" << v.id << " | Name: " << v.name << " | Phone: " << v.phone << " | Coords: " << fixed << setprecision(6) << v.latitude << ", " << v.longitude << "\n";
        return true;
    }

    bool loginVolunteer(const string &name, const string &password, bool hasCoords, double lat, double lon, Volunteer &outUser)
    {
        for (auto &v : volunteers)
        {
            if (v.name == name && v.password == password)
            {
                if (hasCoords)
                {
                    v.latitude = lat;
                    v.longitude = lon;
                }
                outUser = v;
                return true;
            }
        }
        return false;
    }

    string getVolunteersJson() const
    {
        stringstream json;
        json << "[";
        for (size_t i = 0; i < volunteers.size(); ++i)
        {
            if (i > 0) json << ",";
            const auto &v = volunteers[i];
            json << "{\"id\":" << v.id << ",\"name\":\"" << escapeJson(v.name) << "\"" << ",\"phone\":\"" << escapeJson(v.phone) << "\"" << ",\"latitude\":" << fixed << setprecision(6) << v.latitude << ",\"longitude\":" << fixed << setprecision(6) << v.longitude << "}";
        }
        json << "]";
        return json.str();
    }

    int addIncomingSOS(const string &type, int priority, double lat, double lon)
    {
        int assignedId = nextSosId++;
        sosQueue.push({assignedId, type, priority, lat, lon});
        saveActiveSOS();

        cout << "\n[SOS RECEIVED] ID #" << assignedId << " | Type: " << type << " | Priority: " << priority << " | Coords: " << fixed << setprecision(6) << lat << ", " << lon << "\n";
        cout << "   -> Maps: https://www.google.com/maps?q=" << fixed << setprecision(6) << lat << "," << lon << "\n";
        printNearestVolunteer(lat, lon);

        return assignedId;
    }

    bool cancelSOS(int targetId)
    {
        priority_queue<SOS, vector<SOS>, CompareSOS> remaining;
        bool found = false;

        while (!sosQueue.empty())
        {
            SOS s = sosQueue.top();
            sosQueue.pop();
            if (s.sosId == targetId && !found)
            {
                found = true;
            }
            else
            {
                remaining.push(s);
            }
        }

        sosQueue = remaining;
        if (found)
        {
            saveActiveSOS();
            cout << "\n[SOS CANCELLED / FALSE ALARM] ID #" << targetId << " removed from active queue.\n";
        }
        return found;
    }

    bool resolveSOS(int targetId, const string &resolvedBy)
    {
        priority_queue<SOS, vector<SOS>, CompareSOS> remaining;
        bool found = false;
        SOS resolvedItem{};

        while (!sosQueue.empty())
        {
            SOS s = sosQueue.top();
            sosQueue.pop();
            if (s.sosId == targetId && !found)
            {
                found = true;
                resolvedItem = s;
            }
            else
            {
                remaining.push(s);
            }
        }

        sosQueue = remaining;
        if (found)
        {
            saveActiveSOS();
            appendResolvedSOS(resolvedItem, resolvedBy.empty() ? "Volunteer" : resolvedBy);
            cout << "\n[SOS RESOLVED] ID #" << targetId << " (" << resolvedItem.type << ") resolved by " << (resolvedBy.empty() ? "Volunteer" : resolvedBy) << ".\n";
        }
        return found;
    }

    string getActiveSOSJson(int volunteerId = -1) const
    {
        const Volunteer *vol = nullptr;
        if (volunteerId != -1)
        {
            for (const auto &v : volunteers)
            {
                if (v.id == volunteerId)
                {
                    vol = &v;
                    break;
                }
            }
        }

        priority_queue<SOS, vector<SOS>, CompareSOS> temp = sosQueue;
        stringstream json;
        json << "[";
        bool first = true;

        while (!temp.empty())
        {
            SOS s = temp.top();
            temp.pop();
            if (!first) json << ",";
            first = false;

            double dist = (vol != nullptr) ? distanceKm(vol->latitude, vol->longitude, s.latitude, s.longitude) : 0.0;

            json << "{\"sosId\":" << s.sosId << ",\"type\":\"" << escapeJson(s.type) << "\"" << ",\"priority\":" << s.priority << ",\"latitude\":" << fixed << setprecision(6) << s.latitude << ",\"longitude\":" << fixed << setprecision(6) << s.longitude << ",\"distanceKm\":" << fixed << setprecision(2) << dist << "}";
        }
        json << "]";
        return json.str();
    }

    string getNextSosQueueForVolunteer(int volunteerId) const
    {
        const Volunteer *vol = nullptr;
        for (const auto &v : volunteers)
        {
            if (v.id == volunteerId)
            {
                vol = &v;
                break;
            }
        }

        priority_queue<SOS, vector<SOS>, CompareSOS> temp = sosQueue;
        vector<SOS> allActive;
        while (!temp.empty())
        {
            allActive.push_back(temp.top());
            temp.pop();
        }

        priority_queue<SOS, vector<SOS>, CompareSOS> nextQueue;

        if (vol != nullptr && !allActive.empty())
        {
            for (const auto &s : allActive)
            {
                double myDist = distanceKm(vol->latitude, vol->longitude, s.latitude, s.longitude);
                bool shortestAmongAll = true;
                for (const auto &other : volunteers)
                {
                    if (other.id == vol->id) continue;
                    double otherDist = distanceKm(other.latitude, other.longitude, s.latitude, s.longitude);
                    if (otherDist < myDist)
                    {
                        shortestAmongAll = false;
                        break;
                    }
                }
                if (shortestAmongAll)
                {
                    nextQueue.push(s);
                }
            }

            if (nextQueue.empty())
            {
                vector<pair<double, SOS>> rankedByDist;
                for (const auto &s : allActive)
                {
                    rankedByDist.push_back({distanceKm(vol->latitude, vol->longitude, s.latitude, s.longitude), s});
                }
                sort(rankedByDist.begin(), rankedByDist.end(), [](const auto &a, const auto &b) {
                    return a.first < b.first;
                });
                size_t count = min<size_t>(2, rankedByDist.size());
                for (size_t i = 0; i < count; ++i)
                {
                    nextQueue.push(rankedByDist[i].second);
                }
            }
        }
        else
        {
            nextQueue = sosQueue;
        }

        stringstream json;
        json << "[";
        bool first = true;
        while (!nextQueue.empty())
        {
            SOS s = nextQueue.top();
            nextQueue.pop();
            if (!first) json << ",";
            first = false;

            double dist = (vol != nullptr) ? distanceKm(vol->latitude, vol->longitude, s.latitude, s.longitude) : 0.0;

            json << "{\"sosId\":" << s.sosId << ",\"type\":\"" << escapeJson(s.type) << "\"" << ",\"priority\":" << s.priority << ",\"latitude\":" << fixed << setprecision(6) << s.latitude << ",\"longitude\":" << fixed << setprecision(6) << s.longitude << ",\"distanceKm\":" << fixed << setprecision(2) << dist << "}";
        }
        json << "]";
        return json.str();
    }
};

string extractJsonString(const string &json, const string &key)
{
    string pattern = "\"" + key + "\"";
    size_t pos = json.find(pattern);
    if (pos == string::npos) return "";
    pos = json.find(':', pos + pattern.length());
    if (pos == string::npos) return "";
    size_t firstQuote = json.find('"', pos + 1);
    if (firstQuote == string::npos) return "";
    size_t secondQuote = json.find('"', firstQuote + 1);
    if (secondQuote == string::npos) return "";
    return json.substr(firstQuote + 1, secondQuote - firstQuote - 1);
}

double extractJsonNumber(const string &json, const string &key, double fallback = 0.0)
{
    string pattern = "\"" + key + "\"";
    size_t pos = json.find(pattern);
    if (pos == string::npos) return fallback;
    pos = json.find(':', pos + pattern.length());
    if (pos == string::npos) return fallback;
    pos++;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '"')) pos++;
    return atof(json.c_str() + pos);
}

void sendHttpResponse(SOCKET clientSocket, const string &status, const string &body)
{
    stringstream response;
    response << "HTTP/1.1 " << status << "\r\n" << "Content-Type: application/json\r\n" << "Access-Control-Allow-Origin: *\r\n" << "Access-Control-Allow-Methods: GET, POST, DELETE, OPTIONS\r\n" << "Access-Control-Allow-Headers: Content-Type\r\n" << "Content-Length: " << body.size() << "\r\n" << "Connection: close\r\n\r\n" << body;

    string fullResp = response.str();
    send(clientSocket, fullResp.c_str(), (int)fullResp.size(), 0);
}

string readFullHttpRequest(SOCKET clientSocket)
{
    string request;
    char buffer[4096];
    int bytesRead = 0;

    size_t headerEnd = string::npos;
    while ((headerEnd = request.find("\r\n\r\n")) == string::npos)
    {
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) return request;
        request.append(buffer, bytesRead);
    }

    size_t contentLength = 0;
    string headersLower = request.substr(0, headerEnd);
    for (char &ch : headersLower)
    {
        if (ch >= 'A' && ch <= 'Z')
        {
            ch = static_cast<char>(ch - 'A' + 'a');
        }
    }

    size_t clPos = headersLower.find("content-length:");
    if (clPos != string::npos)
    {
        size_t valStart = clPos + 15;
        while (valStart < headersLower.size() && (headersLower[valStart] == ' ' || headersLower[valStart] == '\t'))
        {
            valStart++;
        }
        size_t valEnd = headersLower.find("\r\n", valStart);
        string lenStr = (valEnd == string::npos) ? headersLower.substr(valStart) : headersLower.substr(valStart, valEnd - valStart);
        try
        {
            contentLength = static_cast<size_t>(stoul(lenStr));
        }
        catch (...)
        {
            contentLength = 0;
        }
    }

    size_t bodyStart = headerEnd + 4;
    while (request.size() - bodyStart < contentLength)
    {
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) break;
        request.append(buffer, bytesRead);
    }

    return request;
}

int main()
{
    VolunteerModule module;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cerr << "WSAStartup failed.\n";
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        cerr << "Socket creation failed.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(8080);

    if (bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        cerr << "Bind failed on port 8080 (is another server already running?).\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 10) == SOCKET_ERROR)
    {
        cerr << "Listen failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "==============================================\n";
    cout << " RESCUE OPS C++ Backend running on 127.0.0.1:8080\n";
    cout << " Waiting for SOS & Volunteer requests...\n";
    cout << "==============================================\n";

    while (true)
    {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) continue;

        string request = readFullHttpRequest(clientSocket);

        if (!request.empty())
        {
            if (request.find("OPTIONS ") == 0)
            {
                sendHttpResponse(clientSocket, "204 No Content", "");
            }
            else if (request.find("POST /api/register") == 0)
            {
                size_t bodyPos = request.find("\r\n\r\n");
                string body = (bodyPos != string::npos) ? request.substr(bodyPos + 4) : "";

                string name = extractJsonString(body, "name");
                string phone = extractJsonString(body, "phone");
                string password = extractJsonString(body, "password");
                double lat = extractJsonNumber(body, "latitude", 30.269260);
                double lon = extractJsonNumber(body, "longitude", 77.993599);

                Volunteer created{};
                string errMsg;
                if (module.registerVolunteer(name, phone, password, lat, lon, created, errMsg))
                {
                    stringstream reply;
                    reply << "{\"success\":true,\"user\":{\"id\":" << created.id << ",\"name\":\"" << escapeJson(created.name) << "\""  << ",\"phone\":\"" << escapeJson(created.phone) << "\""  << ",\"latitude\":" << fixed << setprecision(6) << created.latitude  << ",\"longitude\":" << fixed << setprecision(6) << created.longitude << "}}";
                    sendHttpResponse(clientSocket, "200 OK", reply.str());
                }
                else
                {
                    sendHttpResponse(clientSocket, "400 Bad Request", "{\"success\":false,\"error\":\"" + escapeJson(errMsg) + "\"}");
                }
            }
            else if (request.find("POST /api/login") == 0)
            {
                size_t bodyPos = request.find("\r\n\r\n");
                string body = (bodyPos != string::npos) ? request.substr(bodyPos + 4) : "";

                string name = extractJsonString(body, "name");
                string password = extractJsonString(body, "password");
                bool hasLat = body.find("\"latitude\"") != string::npos;
                bool hasLon = body.find("\"longitude\"") != string::npos;
                double lat = extractJsonNumber(body, "latitude", 0.0);
                double lon = extractJsonNumber(body, "longitude", 0.0);

                Volunteer user{};
                if (module.loginVolunteer(name, password, hasLat && hasLon, lat, lon, user))
                {
                    stringstream reply;
                    reply << "{\"success\":true,\"user\":{\"id\":" << user.id << ",\"name\":\"" << escapeJson(user.name) << "\"" << ",\"phone\":\"" << escapeJson(user.phone) << "\"" << ",\"latitude\":" << fixed << setprecision(6) << user.latitude << ",\"longitude\":" << fixed << setprecision(6) << user.longitude << "}}";
                    sendHttpResponse(clientSocket, "200 OK", reply.str());
                }
                else
                {
                    sendHttpResponse(clientSocket, "401 Unauthorized", "{\"success\":false,\"error\":\"Invalid name or password\"}");
                }
            }
            else if (request.find("GET /api/volunteers") == 0)
            {
                sendHttpResponse(clientSocket, "200 OK", module.getVolunteersJson());
            }
            else if (request.find("GET /api/sos/for-volunteer") == 0)
            {
                int volId = -1;
                size_t qPos = request.find("volunteerId=");
                if (qPos != string::npos)
                {
                    volId = atoi(request.c_str() + qPos + 12);
                }
                sendHttpResponse(clientSocket, "200 OK", module.getNextSosQueueForVolunteer(volId));
            }
            else if (request.find("POST /api/sos/resolve") == 0)
            {
                size_t bodyPos = request.find("\r\n\r\n");
                string body = (bodyPos != string::npos) ? request.substr(bodyPos + 4) : "";

                int sosId = (int)extractJsonNumber(body, "sosId", -1);
                string resolvedBy = extractJsonString(body, "resolvedBy");

                if (module.resolveSOS(sosId, resolvedBy))
                {
                    sendHttpResponse(clientSocket, "200 OK", "{\"success\":true}");
                }
                else
                {
                    sendHttpResponse(clientSocket, "404 Not Found", "{\"success\":false,\"error\":\"SOS not found\"}");
                }
            }
            else if (request.find("POST /api/sos") == 0)
            {
                size_t bodyPos = request.find("\r\n\r\n");
                string body = (bodyPos != string::npos) ? request.substr(bodyPos + 4) : "";

                string type = extractJsonString(body, "type");
                if (type.empty()) type = "Disaster / General";
                int priority = (int)extractJsonNumber(body, "priority", 4);
                double lat = extractJsonNumber(body, "latitude", 0.0);
                double lon = extractJsonNumber(body, "longitude", 0.0);

                int newId = module.addIncomingSOS(type, priority, lat, lon);

                stringstream reply;
                reply << "{\"success\":true,\"sosId\":" << newId << "}";
                sendHttpResponse(clientSocket, "200 OK", reply.str());
            }
            else if (request.find("GET /api/sos") == 0)
            {
                int volId = -1;
                size_t qPos = request.find("volunteerId=");
                if (qPos != string::npos)
                {
                    volId = atoi(request.c_str() + qPos + 12);
                }
                sendHttpResponse(clientSocket, "200 OK", module.getActiveSOSJson(volId));
            }
            else if (request.find("DELETE /api/sos/") == 0)
            {
                size_t idStart = string("DELETE /api/sos/").size();
                size_t idEnd = request.find(' ', idStart);
                int targetId = atoi(request.substr(idStart, idEnd - idStart).c_str());

                if (module.cancelSOS(targetId))
                {
                    sendHttpResponse(clientSocket, "200 OK", "{\"success\":true}");
                }
                else
                {
                    sendHttpResponse(clientSocket, "404 Not Found", "{\"success\":false,\"error\":\"SOS not found\"}");
                }
            }
            else
            {
                sendHttpResponse(clientSocket, "404 Not Found", "{\"error\":\"Route not found\"}");
            }
        }

        closesocket(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}

#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <string>
#include <iomanip>
#include <limits>
#include <sstream>
using namespace std;

struct Volunteer
{
    int id;
    string name;
    string phone;
};

struct SOS
{
    int sosId;
    double latitude;
    double longitude;
};

class VolunteerModule
{
private:
    vector<Volunteer> volunteers;
    queue<SOS> sosQueue;

    void saveActiveSOS()
    {
        ofstream file("active_sos.txt");

        queue<SOS> temp = sosQueue;

        while (!temp.empty())
        {
            SOS s = temp.front();
            temp.pop();

            file << s.sosId << "|"
                 << s.latitude << "|"
                 << s.longitude << "\n";
        }

        file.close();
    }

    void loadVolunteers()
    {
        ifstream file("volunteers.txt");
        string line;

        while (getline(file, line))
        {
            stringstream ss(line);
            string id, name, phone;

            if (getline(ss, id, '|') &&
                getline(ss, name, '|') &&
                getline(ss, phone))
            {
                try
                {
                    Volunteer v;
                    v.id = stoi(id);
                    v.name = name;
                    v.phone = phone;
                    volunteers.push_back(v);
                }
                catch (...)
                {
                    cerr << "Skipping invalid volunteer record.\n";
                }
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
            stringstream ss(line);
            string id, lat, lon;

            if (getline(ss, id, '|') &&
                getline(ss, lat, '|') &&
                getline(ss, lon))
            {
                try
                {
                    SOS s;
                    s.sosId = stoi(id);
                    s.latitude = stod(lat);
                    s.longitude = stod(lon);
                    sosQueue.push(s);
                }
                catch (...)
                {
                    cerr << "Skipping invalid SOS record.\n";
                }
            }
        }

        file.close();
    }

    void printGoogleMapsLink(double lat, double lon)
    {
        cout << "Maps Link: https://www.google.com/maps?q="
             << fixed << setprecision(6)
             << lat << "," << lon << "\n";
    }

public:
    VolunteerModule()
    {
        loadVolunteers();
        loadActiveSOS();

        ifstream check("active_sos.txt");

        if (!check.good())
        {
            simulateIncomingFrontendSOS(
                101, 30.289627, 77.997382);

            simulateIncomingFrontendSOS(
                102, 30.267037, 78.081926);

            saveActiveSOS();
        }

        check.close();
    }

    void addVolunteer()
    {
        Volunteer v;

        cout << "\nEnter Volunteer ID: ";
        if (!(cin >> v.id))
        {
            cout << "Invalid ID!\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cout << "Enter Volunteer Name: ";
        getline(cin, v.name);

        cout << "Enter Phone: ";
        getline(cin, v.phone);

        
        ofstream file("volunteers.txt", ios::app);

        if (!file)
        {
            cout << "Unable to open volunteer file!\n";
            return;
        }

        file << v.id << "|" << v.name
             << "|" << v.phone << "\n";
        file.close();

        volunteers.push_back(v);

        cout << "Volunteer registered and saved successfully!\n";
    }

    void displayVolunteers()
    {
        if (volunteers.empty())
        {
            cout << "\nNo volunteers registered!\n";
            return;
        }

        cout << "\n===== REGISTERED VOLUNTEERS =====\n";

        for (const auto &v : volunteers)
        {
            cout << "ID: " << v.id << "\n";
            cout << "Name: " << v.name << "\n";
            cout << "Phone: " << v.phone << "\n";
            cout << "----------------------\n";
        }
    }

    void displayActiveSOS()
    {
        if (sosQueue.empty())
        {
            cout << "\nNo active SOS alerts!\n";
            return;
        }

        queue<SOS> temp = sosQueue;

        cout << "\n===== ACTIVE SOS QUEUE =====\n";

        while (!temp.empty())
        {
            SOS s = temp.front();
            temp.pop();

            cout << "SOS ID: " << s.sosId << "\n";
            cout << "Latitude: " << s.latitude << "\n";
            cout << "Longitude: " << s.longitude << "\n";
            cout << "Status: PENDING RESPONSE\n";

            printGoogleMapsLink(s.latitude, s.longitude);
            cout << "----------------------\n";
        }
    }

    void resolveNextSOS()
    {
        if (sosQueue.empty())
        {
            cout << "\nNo active SOS alerts to resolve!\n";
            return;
        }

        SOS s = sosQueue.front();


        ofstream resolved("resolved_sos.txt", ios::app);

        if (!resolved)
        {
            cout << "Unable to save resolved SOS!\n";
            return;
        }

        resolved << s.sosId << "|"
                 << s.latitude << "|"
                 << s.longitude << "|RESOLVED\n";
        resolved.close();

        sosQueue.pop();

        
        saveActiveSOS();

        cout << "\nSOS ID " << s.sosId
             << " marked as resolved.\n";

        printGoogleMapsLink(s.latitude, s.longitude);
    }

    void simulateIncomingFrontendSOS(
        int id, double lat, double lon)
    {
        sosQueue.push({id, lat, lon});
        saveActiveSOS();
    }
};

int main()
{
    VolunteerModule module;
    int choice;

    do
    {
        cout << "\n===== RESCUE OPS VOLUNTEER MODULE =====\n";
        cout << "1. Register Volunteer\n";
        cout << "2. Display Volunteers\n";
        cout << "3. View Active SOS Queue\n";
        cout << "4. Resolve Next SOS Alert\n";
        cout << "5. Exit\n";
        cout << "Enter choice: ";

        if (!(cin >> choice))
        {
            cout << "Invalid input. Exiting.\n";
            break;
        }

        switch (choice)
        {
            case 1:
                module.addVolunteer();
                break;

            case 2:
                module.displayVolunteers();
                break;

            case 3:
                module.displayActiveSOS();
                break;

            case 4:
                module.resolveNextSOS();
                break;

            case 5:
                cout << "Exiting Volunteer Module.\n";
                break;

            default:
                cout << "Invalid choice! Select 1-5.\n";
        }

    } while (choice != 5);

    return 0;
}

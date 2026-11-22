#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

class SOSRecord
{
public:
    int sosId;
    string name;
    string phone;
    string disasterType;
    string location;
    double latitude;
    double longitude;

    void input()
    {
        cout << "Enter SOS ID: ";
        cin >> sosId;
        cin.ignore();

        cout << "Enter Name: ";
        getline(cin, name);

        cout << "Enter Phone: ";
        getline(cin, phone);

        cout << "Enter Disaster Type: ";
        getline(cin, disasterType);

        cout << "Enter Location: ";
        getline(cin, location);

        cout << "Enter Latitude: ";
        cin >> latitude;

        cout << "Enter Longitude: ";
        cin >> longitude;
    }

    void display()
    {
        cout << "\nSOS ID: " << sosId;
        cout << "\nName: " << name;
        cout << "\nPhone: " << phone;
        cout << "\nDisaster: " << disasterType;
        cout << "\nLocation: " << location;
        cout << "\nLatitude: " << latitude;
        cout << "\nLongitude: " << longitude;
        cout << "\n-------------------------\n";
    }
};

void addSOS()
{
    SOSRecord s;
    s.input();

    ofstream fout("active_sos.txt", ios::app);

    if (!fout)
    {
        cout << "File opening failed!";
        return;
    }

    fout << s.sosId << "|"
         << s.name << "|"
         << s.phone << "|"
         << s.disasterType << "|"
         << s.location << "|"
         << s.latitude << "|"
         << s.longitude << endl;

    fout.close();

    cout << "\nSOS Record Saved Successfully!\n";
}

void displaySOS()
{
    ifstream fin("active_sos.txt");

    if (!fin)
    {
        cout << "No SOS records found!";
        return;
    }

    string line;
    int count = 0;

    while (getline(fin, line))
    {
        stringstream ss(line);
        SOSRecord s;

        string lat, lon, id;

        getline(ss, id, '|');
        s.sosId = stoi(id);

        getline(ss, s.name, '|');
        getline(ss, s.phone, '|');
        getline(ss, s.disasterType, '|');
        getline(ss, s.location, '|');
        getline(ss, lat, '|');
        getline(ss, lon, '|');

        s.latitude = stod(lat);
        s.longitude = stod(lon);

        s.display();
        count++;
    }

    cout << "\nTotal SOS Records: " << count;

    fin.close();
}

void searchSOS()
{
    ifstream fin("active_sos.txt");

    if (!fin)
    {
        cout << "File not found!";
        return;
    }

    int searchId, found = 0;
    string line;

    cout << "Enter SOS ID to search: ";
    cin >> searchId;

    while (getline(fin, line))
    {
        stringstream ss(line);
        SOSRecord s;

        string id, lat, lon;

        getline(ss, id, '|');
        s.sosId = stoi(id);

        getline(ss, s.name, '|');
        getline(ss, s.phone, '|');
        getline(ss, s.disasterType, '|');
        getline(ss, s.location, '|');
        getline(ss, lat, '|');
        getline(ss, lon, '|');

        s.latitude = stod(lat);
        s.longitude = stod(lon);

        if (s.sosId == searchId)
        {
            cout << "\nRecord Found!";
            s.display();
            found = 1;
            break;
        }
    }

    if (found == 0)
        cout << "\nRecord Not Found!";

    fin.close();
}

int main()
{
    int choice;

    do
    {
        cout << "\n\n===== DISASTER MANAGEMENT SYSTEM =====";
        cout << "\n1. Add SOS Record";
        cout << "\n2. Display SOS Records";
        cout << "\n3. Search SOS Record";
        cout << "\n4. Exit";

        cout << "\nEnter Choice: ";
        cin >> choice;

        switch (choice)
        {
            case 1:
                addSOS();
                break;

            case 2:
                displaySOS();
                break;

            case 3:
                searchSOS();
                break;

            case 4:
                cout << "\nThank You!";
                break;

            default:
                cout << "\nInvalid Choice!";
        }

    } while (choice != 4);

    return 0;
}
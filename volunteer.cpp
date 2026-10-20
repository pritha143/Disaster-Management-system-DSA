#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <iomanip>
#include <limits>

struct Volunteer
{
    int id;
    std::string name;
    std::string phone;
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
    std::vector<Volunteer> volunteers;
    std::queue<SOS> sosQueue;

    void printGoogleMapsLink(double lat, double lon) const
    {
        std::cout << "Maps Link : https://www.google.com/maps?q="<< std::fixed << std::setprecision(6)<< lat << "," << lon << "\n";
    }

public:
    void addVolunteer()
    {
        Volunteer v;
        std::cout << "\nEnter Volunteer ID: ";
        while (!(std::cin >> v.id))
        {
            std::cout << "Invalid ID. Enter a numeric ID: ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::cout << "Enter Volunteer Name: ";
        std::getline(std::cin, v.name);

        std::cout << "Enter Phone: ";
        std::cin >> v.phone;

        volunteers.push_back(v);
        std::cout << "\nVolunteer registered successfully!\n";
    }

    void displayVolunteers() const
    {
        if (volunteers.empty())
        {
            std::cout << "\nNo volunteers registered yet!\n";
            return;
        }

        std::cout << "\n===== REGISTERED VOLUNTEERS =====\n";
        for (const auto &v : volunteers)
        {
            std::cout << "ID    : " << v.id << "\n";
            std::cout << "Name  : " << v.name << "\n";
            std::cout << "Phone : " << v.phone << "\n";
            std::cout << "---------------------------\n";
        }
    }

    void displayActiveSOS() const
    {
        if (sosQueue.empty())
        {
            std::cout << "\nNo active SOS alerts in queue!\n";
            return;
        }

        std::cout << "\n===== ACTIVE SOS QUEUE =====\n";
        
        std::queue<SOS> tempQueue = sosQueue;
        while (!tempQueue.empty())
        {
            SOS current = tempQueue.front();
            tempQueue.pop();

            std::cout << "SOS ID    : " << current.sosId << "\n";
            std::cout << "Latitude  : " << std::fixed << std::setprecision(6) << current.latitude << "\n";
            std::cout << "Longitude : " << current.longitude << "\n";
            printGoogleMapsLink(current.latitude, current.longitude);
            std::cout << "Status    : PENDING RESPONSE\n";
            std::cout << "---------------------------\n";
        }
    }

    void resolveNextSOS()
    {
        if (sosQueue.empty())
        {
            std::cout << "\nNo active SOS alerts to resolve!\n";
            return;
        }

        SOS current = sosQueue.front();
        sosQueue.pop();

        std::cout << "\n[ALERT ASSIGNED] Responding to SOS ID: " << current.sosId << "\n";
        std::cout << "Location  : (" << std::fixed << std::setprecision(6)<< current.latitude << ", " << current.longitude << ")\n";

        printGoogleMapsLink(current.latitude, current.longitude);
        std::cout << "SOS successfully marked as resolved and removed from queue.\n";
    }

    void simulateIncomingFrontendSOS(int id, double lat, double lon)
    {
        sosQueue.push({id, lat, lon});
    }
};

int main()
{
    VolunteerModule module;
    int choice;

    module.simulateIncomingFrontendSOS(101, 30.28962700796281, 77.99738168726064);
    module.simulateIncomingFrontendSOS(102, 30.26703651254309, 78.08192638625611);

    while (true)
    {
        std::cout << "\n-------- VOLUNTEER MODULE --------";
        std::cout << "\n1. Register Volunteer";
        std::cout << "\n2. Display Volunteers";
        std::cout << "\n3. View Active SOS Queue";
        std::cout << "\n4. Resolve Next SOS Alert";
        std::cout << "\n5. Exit";
        std::cout << "\n\nEnter choice: ";

        if (!(std::cin >> choice))
        {
            std::cout << "\nInvalid input! Exiting program.\n";
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
                std::cout << "\nExiting Volunteer Module.\n";
                return 0;
            default:
                std::cout << "\nInvalid choice! Please select 1-5.\n";
        }
    }

    return 0;
}
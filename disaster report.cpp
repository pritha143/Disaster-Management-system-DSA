
#include <iostream>
#include <string>
using namespace std;

struct Report
{
    int id;
    string disasterType;
    string location;
    int peopleAffected;
};

int main()
{
    Report reports[100];
    int count = 0;
    int choice;

    do
    {
        cout << "\n===== DISASTER REPORT SYSTEM =====\n";
        cout << "1. Add Disaster Report\n";
        cout << "2. View All Reports\n";
        cout << "3. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        if (choice == 1)
        {
            if (count == 100)
            {
                cout << "Report storage is full!\n";
                continue;
            }

            reports[count].id = count + 1;

            cout << "Enter disaster type: ";
            cin >> ws;
            getline(cin, reports[count].disasterType);

            cout << "Enter location: ";
            getline(cin, reports[count].location);

            cout << "Enter people affected: ";
            cin >> reports[count].peopleAffected;

            if (reports[count].peopleAffected < 0)
            {
                cout << "Invalid number of people!\n";
                continue;
            }

            count++;
            cout << "Report added successfully!\n";
        }
        else if (choice == 2)
        {
            if (count == 0)
            {
                cout << "No reports available.\n";
            }
            else
            {
                for (int i = 0; i < count; i++)
                {
                    cout << "\nReport ID: "
                         << reports[i].id << endl;
                    cout << "Disaster: "
                         << reports[i].disasterType << endl;
                    cout << "Location: "
                         << reports[i].location << endl;
                    cout << "People Affected: "
                         << reports[i].peopleAffected << endl;
                }
            }
        }
        else if (choice == 3)
        {
            cout << "Exiting report system.\n";
        }
        else
        {
            cout << "Invalid choice!\n";
        }

    } while (choice != 3);

    return 0;
}

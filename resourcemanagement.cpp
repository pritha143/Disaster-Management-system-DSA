
#include <iostream>
#include <queue>
#include <string>
using namespace std;

struct Resource
{
    int id;
    string name;
    int quantity;
    int priority;
};

struct Compare
{
    bool operator()(Resource a, Resource b)
    {
        return a.priority < b.priority;
    }
};

int main()
{
    priority_queue<Resource, vector<Resource>, Compare> resources;
    int choice;
    int id = 1;

    do
    {
        cout << "\n===== RESOURCE MANAGEMENT =====\n";
        cout << "1. Add Emergency Resource\n";
        cout << "2. View Next Priority Resource\n";
        cout << "3. Distribute Resource\n";
        cout << "4. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        if (choice == 1)
        {
            Resource r;

            r.id = id++;

            cout << "Enter resource name: ";
            cin >> ws;
            getline(cin, r.name);

            cout << "Enter quantity: ";
            cin >> r.quantity;

            cout << "Enter priority (1-5, 5 is highest): ";
            cin >> r.priority;

            if (r.quantity <= 0 || r.priority < 1 ||
                r.priority > 5)
            {
                cout << "Invalid quantity or priority!\n";
                continue;
            }

            resources.push(r);

            cout << "Resource added successfully!\n";
        }
        else if (choice == 2)
        {
            if (resources.empty())
            {
                cout << "No resources available!\n";
            }
            else
            {
                Resource r = resources.top();

                cout << "\nNext Priority Resource\n";
                cout << "ID: " << r.id << endl;
                cout << "Name: " << r.name << endl;
                cout << "Quantity: " << r.quantity << endl;
                cout << "Priority: " << r.priority << endl;
            }
        }
        else if (choice == 3)
        {
            if (resources.empty())
            {
                cout << "No resources to distribute!\n";
            }
            else
            {
                Resource r = resources.top();
                resources.pop();

                cout << "\nResource distributed successfully!\n";
                cout << "Resource: " << r.name << endl;
                cout << "Quantity distributed: "
                     << r.quantity << endl;
            }
        }
        else if (choice == 4)
        {
            cout << "Exiting Resource Management System.\n";
        }
        else
        {
            cout << "Invalid choice! Select 1-4.\n";
        }

    } while (choice != 4);

    return 0;
}

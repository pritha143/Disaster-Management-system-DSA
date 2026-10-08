#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Volunteer
{
    int id;
    char name[50];
    char phone[15];
    struct Volunteer *next;
};

struct SOS
{
    int sosId;
    float latitude;
    float longitude;
    struct SOS *next;
};

struct Volunteer *vhead = NULL;
struct SOS *front = NULL;
struct SOS *rear = NULL;

void printGoogleMapsLink(float lat, float lon)
{
    printf("Maps Link : https://www.google.com/maps?q=%.6f,%.6f\n", lat, lon);
}

void addVolunteer()
{
    struct Volunteer *newnode = (struct Volunteer *)malloc(sizeof(struct Volunteer));
    if (!newnode)
    {
        printf("\nMemory allocation failed!\n");
        return;
    }

    printf("\nEnter Volunteer ID: ");
    scanf("%d", &newnode->id);

    printf("Enter Volunteer Name: ");
    scanf(" %[^\n]", newnode->name);

    printf("Enter Phone: ");
    scanf("%s", newnode->phone);

    newnode->next = vhead;
    vhead = newnode;

    printf("\nVolunteer registered successfully!\n");
}

void displayVolunteers()
{
    struct Volunteer *p = vhead;

    if (p == NULL)
    {
        printf("\nNo volunteers registered yet!\n");
        return;
    }

    printf("\n===== REGISTERED VOLUNTEERS =====\n");
    while (p != NULL)
    {
        printf("ID    : %d\n", p->id);
        printf("Name  : %s\n", p->name);
        printf("Phone : %s\n", p->phone);
        printf("---------------------------\n");
        p = p->next;
    }
}

void displayActiveSOS()
{
    struct SOS *p = front;

    if (p == NULL)
    {
        printf("\nNo active SOS alerts in queue!\n");
        return;
    }

    printf("\n===== ACTIVE SOS QUEUE =====\n");
    while (p != NULL)
    {
        printf("SOS ID    : %d\n", p->sosId);
        printf("Latitude  : %.4f\n", p->latitude);
        printf("Longitude : %.4f\n", p->longitude);
        printf("Status    : PENDING RESPONSE\n");
        printf("---------------------------\n");
        p = p->next;
    }
}

void resolveNextSOS()
{
    if (front == NULL)
    {
        printf("\nNo active SOS alerts to resolve!\n");
        return;
    }

    struct SOS *temp = front;
    printf("\n[ALERT ASSIGNED] Responding to SOS ID: %d\n", temp->sosId);
    printf("Location  : (%.6f, %.6f)\n", temp->latitude, temp->longitude);
    
    printGoogleMapsLink(temp->latitude, temp->longitude);

    front = front->next;
    if (front == NULL)
    {
        rear = NULL;
    }

    free(temp);
    printf("SOS successfully marked as resolved and removed from queue.\n");
}

void simulateIncomingFrontendSOS(int id, float lat, float lon)
{
    struct SOS *newnode = (struct SOS *)malloc(sizeof(struct SOS));
    newnode->sosId = id;
    newnode->latitude = lat;
    newnode->longitude = lon;
    newnode->next = NULL;

    if (front == NULL)
    {
        front = rear = newnode;
    }
    else
    {
        rear->next = newnode;
        rear = newnode;
    }
}

int main()
{
    int choice;
    simulateIncomingFrontendSOS(101,30.28962700796281, 77.99738168726064);
    simulateIncomingFrontendSOS(102, 30.26703651254309, 78.08192638625611);

    while (1)
    {
        printf("\n-------- VOLUNTEER MODULE --------");
        printf("\n1. Register Volunteer");
        printf("\n2. Display Volunteers");
        printf("\n3. View Active SOS Queue");
        printf("\n4. Resolve Next SOS Alert");
        printf("\n5. Exit");

        printf("\n\nEnter choice: ");
        if (scanf("%d", &choice) != 1)
        {
            printf("\nInvalid input! Exiting program.\n");
            break;
        }

        switch (choice)
        {
            case 1:
                addVolunteer();
                break;

            case 2:
                displayVolunteers();
                break;

            case 3:
                displayActiveSOS();
                break;

            case 4:
                resolveNextSOS();
                break;

            case 5:
                printf("\nExiting Volunteer Module.\n");
                exit(0);

            default:
                printf("\nInvalid choice! Please select 1-5.\n");
        }
    }

    return 0;
}
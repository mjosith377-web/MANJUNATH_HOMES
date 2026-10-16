#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_TENANTS 100
#define NAME_LEN 80
#define PHONE_LEN 24
#define ADDRESS_LEN 160
#define FILE_LEN 120
#define DATA_FILE "tenants.dat"

typedef struct {
    int roomNo;
    char name[NAME_LEN];
    char phone[PHONE_LEN];
    char address[ADDRESS_LEN];
    char paymentMonth[20];
    char paymentReference[FILE_LEN];
    char screenshotFile[FILE_LEN];
    int paymentSubmitted;
    int paymentStatus; /* 0 = Not submitted, 1 = Pending, 2 = Approved, 3 = Rejected */
} Tenant;

Tenant tenants[MAX_TENANTS];
int tenantCount = 0;

int rentForRoom(int roomNo) {
    if (roomNo >= 1 && roomNo <= 10) return 5200;
    if (roomNo >= 11 && roomNo <= 18) return 6200;
    return 0;
}

void readLine(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    if (fgets(buffer, (int)size, stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';
    } else {
        buffer[0] = '\0';
    }
}

int readInt(const char *prompt) {
    char line[64], *end;
    long value;
    while (1) {
        readLine(prompt, line, sizeof(line));
        value = strtol(line, &end, 10);
        if (end != line && *end == '\0') return (int)value;
        printf("Please enter a valid number.\n");
    }
}

void saveData(void) {
    FILE *fp = fopen(DATA_FILE, "wb");
    if (!fp) {
        printf("Warning: Could not save records to file.\n");
        return;
    }
    fwrite(&tenantCount, sizeof(tenantCount), 1, fp);
    fwrite(tenants, sizeof(Tenant), (size_t)tenantCount, fp);
    fclose(fp);
}

void loadData(void) {
    FILE *fp = fopen(DATA_FILE, "rb");
    if (!fp) return;
    if (fread(&tenantCount, sizeof(tenantCount), 1, fp) != 1 ||
        tenantCount < 0 || tenantCount > MAX_TENANTS) {
        tenantCount = 0;
        fclose(fp);
        return;
    }
    if (fread(tenants, sizeof(Tenant), (size_t)tenantCount, fp) != (size_t)tenantCount) {
        tenantCount = 0;
    }
    fclose(fp);
}

int findTenantByRoom(int roomNo) {
    for (int i = 0; i < tenantCount; i++) {
        if (tenants[i].roomNo == roomNo) return i;
    }
    return -1;
}

const char *paymentStatusText(int status) {
    switch (status) {
        case 1: return "Pending verification";
        case 2: return "Approved";
        case 3: return "Rejected";
        default: return "Not submitted";
    }
}

void registerTenant(void) {
    if (tenantCount >= MAX_TENANTS) {
        printf("Tenant storage is full.\n");
        return;
    }

    int roomNo = readInt("Enter room number (1-18): ");
    if (roomNo < 1 || roomNo > 18) {
        printf("Invalid room number. Choose 1 to 18.\n");
        return;
    }
    if (findTenantByRoom(roomNo) != -1) {
        printf("That room is already allocated.\n");
        return;
    }

    Tenant t = {0};
    t.roomNo = roomNo;
    readLine("Enter tenant full name: ", t.name, sizeof(t.name));
    readLine("Enter phone number: ", t.phone, sizeof(t.phone));
    readLine("Enter address: ", t.address, sizeof(t.address));

    if (strlen(t.name) == 0 || strlen(t.phone) == 0 || strlen(t.address) == 0) {
        printf("Name, phone and address cannot be empty. Registration cancelled.\n");
        return;
    }

    t.paymentStatus = 0;
    tenants[tenantCount++] = t;
    saveData();

    printf("\nTenant registered successfully.\n");
    printf("Allocated room: %d\n", roomNo);
    printf("Monthly rent: Rs. %d\n", rentForRoom(roomNo));
}

void viewTenantDetails(void) {
    int roomNo = readInt("Enter your room number: ");
    int i = findTenantByRoom(roomNo);
    if (i == -1) {
        printf("No tenant found for room %d.\n", roomNo);
        return;
    }

    Tenant *t = &tenants[i];
    printf("\n--- TENANT DETAILS ---\n");
    printf("Name             : %s\n", t->name);
    printf("Room number      : %d\n", t->roomNo);
    printf("Phone            : %s\n", t->phone);
    printf("Address          : %s\n", t->address);
    printf("Monthly rent     : Rs. %d\n", rentForRoom(t->roomNo));
    printf("Payment status   : %s\n", paymentStatusText(t->paymentStatus));
    if (t->paymentSubmitted) {
        printf("Payment month    : %s\n", t->paymentMonth);
        printf("Payment reference: %s\n", t->paymentReference);
        printf("Screenshot file  : %s\n", t->screenshotFile);
    }
}

void submitPaymentProof(void) {
    int roomNo = readInt("Enter your room number: ");
    int i = findTenantByRoom(roomNo);
    if (i == -1) {
        printf("No tenant found for room %d.\n", roomNo);
        return;
    }

    Tenant *t = &tenants[i];
    readLine("Payment month (e.g. 2026-10): ", t->paymentMonth, sizeof(t->paymentMonth));
    readLine("Payment reference / transaction ID: ",
             t->paymentReference, sizeof(t->paymentReference));
    readLine("Screenshot filename (e.g. rent_oct.jpg): ",
             t->screenshotFile, sizeof(t->screenshotFile));

    if (strlen(t->paymentMonth) == 0 || strlen(t->screenshotFile) == 0) {
        printf("Payment month and screenshot filename are required.\n");
        return;
    }

    t->paymentSubmitted = 1;
    t->paymentStatus = 1;
    saveData();
    printf("Payment details recorded. Status: Pending verification.\n");
    printf("Note: This C program records the screenshot filename; it does not upload the image.\n");
}

void viewAllTenants(void) {
    if (tenantCount == 0) {
        printf("No tenant records available.\n");
        return;
    }

    printf("\n%-7s %-22s %-16s %-10s %-22s\n",
           "Room", "Name", "Phone", "Rent", "Payment status");
    printf("-------------------------------------------------------------------------------\n");
    for (int i = 0; i < tenantCount; i++) {
        printf("%-7d %-22.22s %-16.16s Rs.%-6d %-22s\n",
               tenants[i].roomNo, tenants[i].name, tenants[i].phone,
               rentForRoom(tenants[i].roomNo), paymentStatusText(tenants[i].paymentStatus));
    }
}

void reviewPayment(void) {
    int roomNo = readInt("Enter room number to review: ");
    int i = findTenantByRoom(roomNo);
    if (i == -1) {
        printf("No tenant found for that room.\n");
        return;
    }

    Tenant *t = &tenants[i];
    if (!t->paymentSubmitted) {
        printf("No payment proof has been submitted for this room.\n");
        return;
    }

    printf("\nTenant: %s | Room: %d | Rent: Rs. %d\n",
           t->name, t->roomNo, rentForRoom(t->roomNo));
    printf("Month: %s\nReference: %s\nScreenshot filename: %s\n",
           t->paymentMonth, t->paymentReference, t->screenshotFile);
    printf("Current status: %s\n", paymentStatusText(t->paymentStatus));
    printf("1. Approve payment\n2. Reject payment\n0. Cancel\n");
    int choice = readInt("Choose an action: ");

    if (choice == 1) {
        t->paymentStatus = 2;
        printf("Payment marked as approved.\n");
    } else if (choice == 2) {
        t->paymentStatus = 3;
        printf("Payment marked as rejected. Ask the tenant to resubmit proof.\n");
    } else {
        printf("No changes made.\n");
        return;
    }
    saveData();
}

void showRules(void) {
    printf("\n--- PROPERTY RULES ---\n");
    printf("1. Follow the property's lawful alcohol policy.\n");
    printf("2. Additional occupants require prior owner approval.\n");
    printf("3. Fighting and disruptive behaviour are prohibited under the property rules.\n");
    printf("4. Pay rent by the agreed due date and keep payment proof.\n");
    printf("5. Follow the signed rental agreement and applicable law.\n");
}

void tenantMenu(void) {
    int choice;
    do {
        printf("\n=== TENANT MENU ===\n");
        printf("1. Register tenant\n");
        printf("2. View tenant details\n");
        printf("3. Submit rent payment proof details\n");
        printf("4. View property rules\n");
        printf("0. Back to main menu\n");
        choice = readInt("Enter choice: ");

        switch (choice) {
            case 1: registerTenant(); break;
            case 2: viewTenantDetails(); break;
            case 3: submitPaymentProof(); break;
            case 4: showRules(); break;
            case 0: break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 0);
}

void ownerMenu(void) {
    int choice;
    do {
        printf("\n=== OWNER MENU ===\n");
        printf("1. View all tenants\n");
        printf("2. Review payment proof\n");
        printf("3. View room occupancy\n");
        printf("0. Back to main menu\n");
        choice = readInt("Enter choice: ");

        switch (choice) {
            case 1: viewAllTenants(); break;
            case 2: reviewPayment(); break;
            case 3: {
                printf("\n--- ROOM OCCUPANCY ---\n");
                for (int room = 1; room <= 18; room++) {
                    int i = findTenantByRoom(room);
                    printf("Room %2d | Rent Rs. %-4d | %s",
                           room, rentForRoom(room), i == -1 ? "VACANT" : "OCCUPIED");
                    if (i != -1) printf(" | %s", tenants[i].name);
                    printf("\n");
                }
                break;
            }
            case 0: break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 0);
}

int main(void) {
    loadData();
    int choice;

    printf("=========================================\n");
    printf("   RENTAL ROOMS MANAGEMENT SYSTEM\n");
    printf("=========================================\n");
    printf("Rent rules: Rooms 1-10 = Rs. 5200/month\n");
    printf("            Rooms 11-18 = Rs. 6200/month\n");
    printf("Note: This is a basic educational prototype.\n");

    do {
        printf("\n=== MAIN MENU ===\n");
        printf("1. Tenant interface\n");
        printf("2. Owner interface\n");
        printf("0. Exit\n");
        choice = readInt("Enter choice: ");

        switch (choice) {
            case 1: tenantMenu(); break;
            case 2: ownerMenu(); break;
            case 0: saveData(); printf("Records saved. Goodbye!\n"); break;
            default: printf("Invalid choice. Try again.\n");
        }
    } while (choice != 0);

    return 0;
}

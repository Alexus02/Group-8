#include <stdio.h>
#include <string.h>

#define MAX_CONTACTS 100
#define NAME_LEN 100
#define PHONE_LEN 30
#define EMAIL_LEN 100

typedef struct {
	char name[NAME_LEN];
	char phone[PHONE_LEN];
	char email[EMAIL_LEN];
	int isActive;
} Contact;

void removeNewline(char text[]) {
	int i;
	for (i = 0; text[i] != '\0'; i++) {
		if (text[i] == '\n') {
			text[i] = '\0';
			break;
		}
	}
}

void inputText(char label[], char value[], int size) {
	printf("%s", label);
	fgets(value, size, stdin);
	removeNewline(value);
}

int findByName(Contact contacts[], int count, char name[]) {
	int i;
	for (i = 0; i < count; i++) {
		if (contacts[i].isActive == 1 && strcmp(contacts[i].name, name) == 0) {
			return i;
		}
	}
	return -1;
}

void addContact(Contact contacts[], int *count) {
	if (*count == MAX_CONTACTS) {
		printf("\nList is full.\n");
		return;
	}

	inputText("Enter name : ", contacts[*count].name, NAME_LEN);

	if (strlen(contacts[*count].name) == 0) {
		printf("Name cannot be empty.\n");
		return;
	}

	if (findByName(contacts, *count, contacts[*count].name) != -1) {
		printf("A contact with this name already exists.\n");
		return;
	}

	inputText("Enter phone: ", contacts[*count].phone, PHONE_LEN);
	inputText("Enter email: ", contacts[*count].email, EMAIL_LEN);
	contacts[*count].isActive = 1;
	(*count)++;

	printf("Contact added successfully.\n");
}

void searchContact(Contact contacts[], int count) {
	char name[NAME_LEN];
	int index = -1;

	if (count == 0) {
		printf("\nNo contacts found.\n");
		return;
	}

	inputText("Enter name to search: ", name, NAME_LEN);
	index = findByName(contacts, count, name);

	if (index == -1) {
		printf("Contact not found.\n");
		return;
	}

	printf("\nContact found:\n");
	printf("Name : %s\n", contacts[index].name);
	printf("Phone: %s\n", contacts[index].phone);
	printf("Email: %s\n", contacts[index].email);
}

void updateContact(Contact contacts[], int count) {
	char name[NAME_LEN];
	int index = -1;

	if (count == 0) {
		printf("\nNo contacts found.\n");
		return;
	}

	inputText("Enter name to update: ", name, NAME_LEN);
	index = findByName(contacts, count, name);

	if (index == -1) {
		printf("Contact not found.\n");
		return;
	}

	printf("Enter new details for %s\n", contacts[index].name);
	inputText("New phone: ", contacts[index].phone, PHONE_LEN);
	inputText("New email: ", contacts[index].email, EMAIL_LEN);

	printf("Contact updated successfully.\n");
}

void deleteContact(Contact contacts[], int count) {
	char name[NAME_LEN];
	int index = -1;

	if (count == 0) {
		printf("\nNo contacts found.\n");
		return;
	}

	inputText("Enter name to delete: ", name, NAME_LEN);
	index = findByName(contacts, count, name);

	if (index == -1) {
		printf("Contact not found.\n");
		return;
	}

	contacts[index].isActive = 0;
	printf("Contact deleted successfully.\n");
}

void listContacts(Contact contacts[], int count) {
	int i;
	int serial = 1;

	printf("\n---- Contact List ----\n");
	for (i = 0; i < count; i++) {
		if (!contacts[i].isActive) {
			continue;
		}

		printf("\nContact %d\n", serial++);
		printf("Name : %s\n", contacts[i].name);
		printf("Phone: %s\n", contacts[i].phone);
		printf("Email: %s\n", contacts[i].email);
	}

	if (serial == 1) {
		printf("No active contacts to display.\n");
	}
}

int readChoice() {
	int choice;
	int valid;

	printf("\n===== Contact Management System =====\n");
	printf("1. Add Contact\n");
	printf("2. Search Contact\n");
	printf("3. Update Contact\n");
	printf("4. Delete Contact\n");
	printf("5. List Contacts\n");
	printf("6. Exit\n");
	printf("Enter your choice: ");

	valid = scanf("%d", &choice);
	while (getchar() != '\n') {
	}

	if (valid != 1) {
		return -1;
	}

	return choice;
}

int main() {
	Contact contacts[MAX_CONTACTS] = {0};
	int count = 0;
	int choice;

	while (1) {
		choice = readChoice();

		switch (choice) {
			case 1:
				addContact(contacts, &count);
				break;
			case 2:
				searchContact(contacts, count);
				break;
			case 3:
				updateContact(contacts, count);
				break;
			case 4:
				deleteContact(contacts, count);
				break;
			case 5:
				listContacts(contacts, count);
				break;
			case 6:
				printf("Exiting...\n");
				return 0;
			default:
				printf("Invalid choice. Please try again.\n");
		}
	}
}

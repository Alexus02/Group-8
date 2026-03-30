/*
 * Additions:
 *   - if name/phone/email is invalid, re-prompt instead of breaking.
 *   - Search shows exact match first, then best matches
 *   - Update can rename (with duplicate-name check); delete is soft-delete.
 *   - Improved input safety for fgets/scanf.
 */

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
	/* If fgets fails (EOF), set empty string so value isn't garbage - 4106024 */
	if (fgets(value, size, stdin) == NULL) {
		value[0] = '\0';
		return;
	}
	removeNewline(value);
}

/* Checks for whitespace (spaces/tabs/newlines) and digits - 4106024 */
static int charIsSpace(unsigned char c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

static int charIsDigit(unsigned char c) {
	return c >= '0' && c <= '9';
}

/* Strips edges so strcmp exact match is not broken by stray spaces - 4106024 */
void trimWhitespace(char s[]) {
	char *start;
	char *end;

	if (s == NULL) {
		return;
	}
	start = s;
	while (*start != '\0' && charIsSpace((unsigned char)*start)) {
		start++;
	}
	if (*start == '\0') {
		s[0] = '\0';
		return;
	}
	end = start + strlen(start);
	while (end > start && charIsSpace((unsigned char)end[-1])) {
		end--;
	}
	*end = '\0';
	if (start != s) {
		memmove(s, start, (size_t)(end - start + 1));
	}
}

int isBlank(const char text[]) {
	int i;
	/* Returns 1 if the string is only whitespace; used for re-prompt rules - 4106024 */
	for (i = 0; text[i] != '\0'; i++) {
		if (!charIsSpace((unsigned char)text[i])) {
			return 0;
		}
	}
	return 1;
}

/* Rough phone check; digits + common separators, at least 7 digits - 4106024 */
int isValidPhone(const char phone[]) {
	int i;
	int digitCount = 0;
	for (i = 0; phone[i] != '\0'; i++) {
		char ch = phone[i];
		if (charIsDigit((unsigned char)ch)) {
			digitCount++;
			continue;
		}
		if (ch == '+' || ch == '-' || ch == ' ' || ch == '(' || ch == ')') {
			continue;
		}
		return 0;
	}
	return digitCount >= 7;
}

/* Minimal “has @ and a dot after it” check - 4106024 */
int isValidEmail(const char email[]) {
	int i;
	int atPos = -1;
	int dotAfterAt = 0;

	for (i = 0; email[i] != '\0'; i++) {
		if (email[i] == '@') {
			if (atPos != -1) {
				return 0;
			}
			atPos = i;
		}
		if (email[i] == '.' && atPos != -1 && i > atPos + 1) {
			dotAfterAt = 1;
		}
	}

	return atPos > 0 && dotAfterAt && email[atPos + 1] != '\0';
}

/* Prefer an inactive slot before growing count */
int findAvailableSlot(Contact contacts[], int count) {
	int i;
	for (i = 0; i < count; i++) {
		if (!contacts[i].isActive) {
			return i;
		}
	}
	if (count < MAX_CONTACTS) {
		return count;
	}
	return -1;
}

int hasActiveContacts(Contact contacts[], int count) {
	int i;
	for (i = 0; i < count; i++) {
		if (contacts[i].isActive) {
			return 1;
		}
	}
	return 0;
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

/* Find duplicate name among other active rows (skip row excludeIndex) */
int findOtherActiveByName(Contact contacts[], int count, char name[], int excludeIndex) {
	int i;
	for (i = 0; i < count; i++) {
		if (i == excludeIndex) {
			continue;
		}
		if (contacts[i].isActive && strcmp(contacts[i].name, name) == 0) {
			return i;
		}
	}
	return -1;
}

void addContact(Contact contacts[], int *count) {
	char name[NAME_LEN];
	char phone[PHONE_LEN];
	char email[EMAIL_LEN];

	/* Store the new contact at contacts[*count]. */
	if (*count == MAX_CONTACTS) {
		printf("\nList is full.\n");
		return;
	}

	/* Added re-prompt on invalid/duplicate name (don't exit to menu) - 4106024 */
	while (1) {
		inputText("Enter name : ", name, NAME_LEN);
		trimWhitespace(name);

		if (strlen(name) == 0 || isBlank(name)) {
			printf("Name cannot be empty.\n");
			continue;
		}

		if (findByName(contacts, *count, name) != -1) {
			printf("A contact with this name already exists.\n");
			continue;
		}

		break;
	}

	/* Added keep asking until phone passes validation - 4106024 */
	while (1) {
		inputText("Enter phone: ", phone, PHONE_LEN);
		if (strlen(phone) > 0 && isValidPhone(phone)) {
			break;
		}
		printf("Invalid phone number. Try again.\n");
	}

	/* Added keep asking until email passes validation - 4106024 */
	while (1) {
		inputText("Enter email: ", email, EMAIL_LEN);
		if (strlen(email) > 0 && isValidEmail(email)) {
			break;
		}
		printf("Invalid email address. Try again.\n");
	}

	strcpy(contacts[*count].name, name);
	strcpy(contacts[*count].phone, phone);
	strcpy(contacts[*count].email, email);
	contacts[*count].isActive = 1;
	(*count)++;

	printf("Contact added successfully.\n");
}

/*
 * One prompt: exact full-name match prints Exact match; then prefix matches, then substring.
 * If nothing is exact, the same tiers appear under Best matches - 4106024)
 */
void searchContact(Contact contacts[], int count) {
	char query[NAME_LEN];
	int exactIdx;
	int i;
	int anyClose;
	int qLen;
	int otherHeader;
	int shown[MAX_CONTACTS];

	if (count == 0) {
		printf("\nNo contacts found.\n");
		return;
	}

	inputText("Enter name to search: ", query, NAME_LEN);
	trimWhitespace(query);
	if (strlen(query) == 0) {
		printf("Search text cannot be empty.\n");
		return;
	}

	memset(shown, 0, sizeof shown);
	exactIdx = findByName(contacts, count, query);
	if (exactIdx != -1) {
		printf("\nExact match:\n");
		printf("Name : %s\n", contacts[exactIdx].name);
		printf("Phone: %s\n", contacts[exactIdx].phone);
		printf("Email: %s\n", contacts[exactIdx].email);
		shown[exactIdx] = 1;
	}

	anyClose = 0;
	otherHeader = 0;
	qLen = (int)strlen(query);

	if (exactIdx == -1) {
		printf("\nNo exact match for \"%s\". Best matches:\n", query);
	}

	for (i = 0; i < count; i++) {
		if (!contacts[i].isActive || shown[i]) {
			continue;
		}
		if (qLen > (int)strlen(contacts[i].name)) {
			continue;
		}
		if (strncmp(contacts[i].name, query, (size_t)qLen) == 0) {
			if (exactIdx != -1 && !otherHeader) {
				printf("\nOther close matches:\n");
				otherHeader = 1;
			}
			printf("\nName : %s\n", contacts[i].name);
			printf("Phone: %s\n", contacts[i].phone);
			printf("Email: %s\n", contacts[i].email);
			shown[i] = 1;
			anyClose = 1;
		}
	}

	for (i = 0; i < count; i++) {
		if (!contacts[i].isActive || shown[i]) {
			continue;
		}
		if (strstr(contacts[i].name, query) != NULL) {
			if (exactIdx != -1 && !otherHeader) {
				printf("\nOther close matches:\n");
				otherHeader = 1;
			}
			printf("\nName : %s\n", contacts[i].name);
			printf("Phone: %s\n", contacts[i].phone);
			printf("Email: %s\n", contacts[i].email);
			shown[i] = 1;
			anyClose = 1;
		}
	}

	if (exactIdx == -1 && !anyClose) {
		printf("No contacts matched.\n");
	}
}

void updateContact(Contact contacts[], int count) {
	char name[NAME_LEN];
	char newName[NAME_LEN];
	char newPhone[PHONE_LEN];
	char newEmail[EMAIL_LEN];
	int index = -1;

	if (count == 0) {
		printf("\nNo contacts found.\n");
		return;
	}

	/* Lookup key only (name used to find row; new values overwrite fields) - 4106024 */
	inputText("Enter name to update: ", name, NAME_LEN);
	trimWhitespace(name);
	index = findByName(contacts, count, name);

	if (index == -1) {
		printf("Contact not found.\n");
		return;
	}

	printf("Enter new details for %s\n", contacts[index].name);

	/* Added optional rename (empty keeps current); prevents duplicate names - 4106024 */
	while (1) {
		inputText("New name (leave empty to keep current): ", newName, NAME_LEN);
		trimWhitespace(newName);
		if (strlen(newName) == 0) {
			break;
		}
		if (isBlank(newName)) {
			printf("Name cannot be empty or only spaces.\n");
			continue;
		}
		if (strcmp(newName, contacts[index].name) == 0) {
			break;
		}
		if (findOtherActiveByName(contacts, count, newName, index) != -1) {
			printf("Another contact already uses that name.\n");
			continue;
		}
		strcpy(contacts[index].name, newName);
		break;
	}

	/* Added: keep asking until phone is valid - 4106024 */
	while (1) {
		inputText("New phone: ", newPhone, PHONE_LEN);
		if (strlen(newPhone) > 0 && isValidPhone(newPhone)) {
			break;
		}
		printf("Invalid phone number. Try again.\n");
	}

	/* Added: keep asking until email is valid - 4106024 */
	while (1) {
		inputText("New email: ", newEmail, EMAIL_LEN);
		if (strlen(newEmail) > 0 && isValidEmail(newEmail)) {
			break;
		}
		printf("Invalid email address. Try again.\n");
	}

	strcpy(contacts[index].phone, newPhone);
	strcpy(contacts[index].email, newEmail);

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
	trimWhitespace(name);
	index = findByName(contacts, count, name);

	if (index == -1) {
		printf("Contact not found.\n");
		return;
	}

	/* Confirm before marking the contact as deleted (soft-delete) - 4106024 */
	{
		char confirm[16];
		printf("Delete \"%s\"? Type y to confirm, anything else cancels: ", contacts[index].name);
		inputText("", confirm, (int)sizeof confirm);
		if (confirm[0] != 'y' && confirm[0] != 'Y') {
			printf("Delete cancelled.\n");
			return;
		}
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "macros.h"

static struct School school;

// ================== Functions prototypes ==================
FILE* openFile(const char* filename,const char* mode);
void initDB();
void printDB();
void addToDB(struct Student* newStudent, int i, int j);
void freeDB();


// ================== Main function =========================
int main() {
    initDB();
    printDB();
    atexit(freeDB);
    return EXIT_SUCCESS;
}

// ================== Functions implementations =============
/**
 * Print the school database
 */
void printDB()
{
    printf("Printing DB:\n");
    for (int level = 0; level < NUM_OF_LEVELS; level++) {
        for (int class = 0; class < NUM_OF_CLASSES; class++) {
            struct Student* curr = school.DB[level][class];
            while (curr != NULL) {
                printf("=======================================\n");
                printf("Student: %s %s\n", curr->firstName, curr->lastName);
                printf("Phone: %s\n", curr->phone);
                printf("Level ID: %d\n", curr->levelID);
                printf("Class ID: %d\n", curr->classID);
                printf("Grades:");
                for (int i = 0; i < NUM_OF_COURSES; i++) {
                    printf(" %d", curr->grades[i]);
                }
                printf("\n");
                printf("=======================================\n\n");

                // Move to the next student
                curr = curr->next;
            }
        }
    }
}

/**
 * Open a file and check for errors
 * @param filename
 * @param mode
 * @return FILE* pointer to the file
 */
FILE* openFile(const char* filename,const char* mode)
{
    FILE* fp = fopen(filename,mode);
    if(fp == NULL)
    {
        printf("Error opening file %s\n",filename);
        exit(EXIT_FAILURE);
    }
    return fp;
}

/**
 * Initialize the school database - read the data from the file
 */
void initDB()
{
    FILE* fp = openFile(FILENAME,"r");
    char firstName[NAME_LEN], lastName[NAME_LEN], phone[PHONE_LEN];
    int levelID, classID;

    while (fscanf(fp, "%s %s %s %d %d", firstName, lastName,
                  phone, &levelID, &classID) == 5) {

        // Allocate memory for new student
        struct Student* newStudent = (struct Student*)malloc(sizeof(struct Student));
        if (newStudent == NULL) {
            printf("Error allocating memory for new student\n");
            exit(EXIT_FAILURE);
        }

        // use snprintf so we won't have buffer overflow
        snprintf(newStudent->firstName, NAME_LEN, "%s", firstName);
        snprintf(newStudent->lastName, NAME_LEN, "%s", lastName);
        snprintf(newStudent->phone, PHONE_LEN, "%s", phone);
        newStudent->levelID = levelID;
        newStudent->classID = classID;

        for (int i = 0; i < NUM_OF_COURSES; i++) {
            if(fscanf(fp, "%d", &newStudent->grades[i]) != 1) {
                printf("Error reading grades\n");
                free(newStudent);
                exit(EXIT_FAILURE);
            }
        }

        // Add the new student to the database
        addToDB(newStudent, levelID-1, classID-1);
    }

    fclose(fp);
}

/**
 * Add a new student to the database
 * @param newStudent
 * @param i,j - the position in the database
 */
void addToDB(struct Student* newStudent, int i, int j) {
    newStudent->next = school.DB[i][j];
    newStudent->prev = NULL;

    if (school.DB[i][j] != NULL) {
        school.DB[i][j]->prev = newStudent;
    }

    school.DB[i][j] = newStudent;
}

/**
 * Free the memory allocated for the database
 */
void freeDB() {
    for (int level = 0; level < NUM_OF_LEVELS; level++) {
        for (int class = 0; class < NUM_OF_CLASSES; class++) {
            struct Student* curr = school.DB[level][class];
            while (curr != NULL) {
                struct Student* temp = curr;
                curr = curr->next;
                free(temp);
            }
        }
    }
}
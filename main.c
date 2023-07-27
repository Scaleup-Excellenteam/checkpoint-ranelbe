#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "macros.h"

static struct School school = { .DB = { { NULL } } };
//static struct topStudents* topStudents[NUM_OF_LEVELS][TOP10];

// ================== Functions prototypes ==================
FILE* openFile(const char* filename,const char* mode);
void initDB();
void printDB();
void printStudent(struct Student* student);
void addToDB(struct Student* newStudent, int i, int j);
void deleteFromDB(struct Student* student, int i, int j);
void menu();
void registerStudent();
void deleteStudent();
struct Student* searchStudentByName();
void printStudentByName();
void top10Students();
void freeDB();


// ================== Main function =========================
int main()
{
    initDB();
    menu();
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
    for (int level = 0; level < NUM_OF_LEVELS; level++) {
        for (int class = 0; class < NUM_OF_CLASSES; class++) {
            struct Student* curr = school.DB[level][class];
            while (curr != NULL) {
                printStudent(curr);
                // Move to the next student
                curr = curr->next;
            }
        }
    }
}

/**
 * Print a student record
 * @param student
 */
void printStudent(struct Student* student)
{
    printf("=======================================\n");
    printf("Student: %s %s\n", student->firstName, student->lastName);
    printf("Phone: %s\n", student->phone);
    printf("Level ID: %d\n", student->levelID);
    printf("Class ID: %d\n", student->classID);
    printf("Grades:");
    for (int i = 0; i < NUM_OF_COURSES; i++) {
        printf(" %d", student->grades[i]);
    }
    printf("\nAverage grade: %d\n", student->avgGrade);
    printf("\n");
    printf("=======================================\n\n");
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
    char firstName[NAME_LEN], lastName[NAME_LEN], phone[NAME_LEN];
    int levelID, classID;

    while (fscanf(fp, "%s %s %s %d %d", firstName, lastName, phone, &levelID, &classID) == 5) {
        // Allocate memory for new student
        struct Student* newStudent = (struct Student*)malloc(sizeof(struct Student));
        if (newStudent == NULL) {
            printf("Error allocating memory for new student\n");
            exit(EXIT_FAILURE);
        }

        // use snprintf so we won't have buffer overflow
        snprintf(newStudent->firstName, NAME_LEN, "%s", firstName);
        snprintf(newStudent->lastName, NAME_LEN, "%s", lastName);
        snprintf(newStudent->phone, PHONE_LEN+1, "%s", phone);
        newStudent->levelID = levelID;
        newStudent->classID = classID;

        int sumOfGrades = 0;
        for (int i = 0; i < NUM_OF_COURSES; i++) {
            if (fscanf(fp, "%d", &newStudent->grades[i]) != 1) {
                printf("Error reading grades\n");
                free(newStudent);
                fclose(fp);
                exit(EXIT_FAILURE);
            }
            sumOfGrades += newStudent->grades[i];
        }
        newStudent->avgGrade = sumOfGrades / NUM_OF_COURSES;
        // Add the new student to the database
        addToDB(newStudent, levelID-1, classID-1);
    }

    fclose(fp);
}

/**
 * Add a new student to the database
 * in the correct position based on the average grade
 * @param newStudent
 * @param i,j - the position in the database
 */
void addToDB(struct Student* newStudent, int i, int j) {
     if (school.DB[i][j] == NULL || newStudent->avgGrade > school.DB[i][j]->avgGrade) {
        newStudent->next = school.DB[i][j];
        newStudent->prev = NULL;
        if (school.DB[i][j] != NULL) {
            school.DB[i][j]->prev = newStudent;
        }
        school.DB[i][j] = newStudent;
        return;
    }

    struct Student* curr = school.DB[i][j];
    while (curr->next != NULL && newStudent->avgGrade <= curr->next->avgGrade) {
        curr = curr->next;
    }
    newStudent->next = curr->next;
    newStudent->prev = curr;

    if (curr->next != NULL) {
        curr->next->prev = newStudent;
    }
    curr->next = newStudent;
}

/**
 * Delete a student from the database
 * @param student
 */
void deleteFromDB(struct Student* student, int i, int j)
{
    // Student found, delete the student from the linked list
    if (student->prev != NULL) {
        student->prev->next = student->next;
    }
    else {
        school.DB[i][j] = student->next;
    }

    if (student->next != NULL) {
        student->next->prev = student->prev;
    }

    // Free memory occupied by the student
    free(student);
    printf("Student deleted successfully.\n");
}

/**
 * user interface to perform operations on the database
 */
void menu()
{
    int choice;
    do {
        printf("\n===== Menu =====\n");
        printf("0. Exit\n");
        printf("1. Admission of a new student\n");
        printf("2. Delete a student\n");
        printf("4. Search student by name\n");
        printf("5. top10 students\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                registerStudent();
                break;
            case 2:
                deleteStudent();
                break;
            case 4:
                printStudentByName();
                break;

            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    } while (choice != 0);
}

/**
 * Read student details from the user and add the student to the database
 */
void registerStudent()
{
    struct Student* newStudent = (struct Student*)malloc(sizeof(struct Student));
    if (newStudent == NULL) {
        printf("Error allocating memory for new student\n");
        exit(EXIT_FAILURE);
    }
    // Get student details from the user
    printf("Enter student details:\n");
    printf("Enter first name: ");
    scanf("%s", newStudent->firstName);
    printf("Enter last name: ");
    scanf("%s", newStudent->lastName);
    printf("Enter phone number: ");
    scanf("%s", newStudent->phone);
    printf("Enter level ID: ");
    scanf("%d", &newStudent->levelID);
    printf("Enter class ID: ");
    scanf("%d", &newStudent->classID);

    // Get grades from the user
    int sumOfGrades = 0;
    for (int i = 0; i < NUM_OF_COURSES; i++) {
        printf("Enter grade for course %d: ", i+1);
        scanf("%d", &newStudent->grades[i]);
        sumOfGrades += newStudent->grades[i];
    }
    newStudent->avgGrade = sumOfGrades / NUM_OF_COURSES;

    // Add the new student to the database
    addToDB(newStudent, newStudent->levelID-1, newStudent->classID-1);
}

/**
 * Delete a student record from the database
 */
void deleteStudent()
{
    // Read student phone number from user
    int level, class;
    char phone[PHONE_LEN];

    printf("Student's level: ");
    scanf("%d", &level);
    printf("Student's class: ");
    scanf("%d", &class);
    printf("student's phone: ");
    scanf("%s", phone);

    // Find the student in the specified level and class
    struct Student* curr = school.DB[level - 1][class - 1];
    while (curr != NULL)
    {
        if (strcmp(curr->phone, phone) == 0){
            deleteFromDB(curr, level - 1, class - 1);
            return; // Exit the function after deletion
        }
        curr = curr->next;
    }

    // If the function reaches this point, the student was not found
    printf("Student with the given level, class, and phone number not found.\n");
}


/**
 * Search for a student in the database
 * @return pointer to the student if found, NULL otherwise
 */
struct Student* searchStudentByName()
{
    char firstName[NAME_LEN];
    char lastName[NAME_LEN];

    printf("Enter student's first name: ");
    scanf("%s", firstName);

    printf("Enter student's last name: ");
    scanf("%s", lastName);

    for (int level = 0; level < NUM_OF_LEVELS; level++) {
        for (int class = 0; class < NUM_OF_CLASSES; class++) {
            struct Student* curr = school.DB[level][class];
            while (curr != NULL) {
                if (strcmp(curr->firstName, firstName) == 0 &&
                    strcmp(curr->lastName, lastName) == 0) {
                    return curr; // Found the student
                }
                curr = curr->next;
            }
        }
    }
    return NULL; // Student not found
}

/**
 * Print a student record by name
 */
void printStudentByName() {
    struct Student *student = searchStudentByName();
    if (student == NULL) {
        printf("Student not found.\n");
        return;
    }
    printStudent(student);
}

/**
 * Print the top 10 students in each level for the given course
 */
void top10Students()
{
    //read course number from user
    int courseNum;
    printf("Enter course number: ");
    scanf("%d", &courseNum);

    for (int level = 0; level < NUM_OF_LEVELS; level++) {


    }

}

/**
 * Free the memory allocated for the database
 */
void freeDB()
{
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

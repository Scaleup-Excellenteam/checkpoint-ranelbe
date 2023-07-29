#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/aes.h>
#include <unistd.h>
#include "macros.h"

static School school = { .DB = { { NULL } } };
static CourseStudent* courseStudents[NUM_OF_LEVELS][NUM_OF_COURSES] = { { NULL } };

// ================== Main function =========================
int main()
{
    initDB();
    menu();
    atexit(freeDB);
    return EXIT_SUCCESS;
}

// ================== Functions implementations =============
/**
 * Open a file and check for errors
 * @param filename
 * @param mode
 * @return FILE* pointer to the file
 */
FILE* openFile(const char* filename,const char* mode)
{
    FILE* fp = fopen(filename,mode);
    if(fp == NULL) {
        error("Error opening file");
    }
    return fp;
}

/**
 * Initialize the school database - read the data from the file
 * If the encrypted file exists, read from it, otherwise read from the regular file.
 */
void initDB()
{
    if (access(ENCRYPTED_FILENAME, F_OK) == 0) {
        readEncryptedFile();
    } else {
        readRegularFile();
    }
}

/**
 * Read the regular file
 */
void readRegularFile()
{
    FILE* fp = openFile(FILENAME, "r");
    char* line = NULL;
    size_t len = 0;

    // Read the file line by line and parse the data
    while ((getline(&line, &len, fp)) != -1) {
        Student* newStudent = parseStudent(line);
        if (newStudent == NULL) {
            fclose(fp);
            free(line);
            error("Error parsing student data from file");
        }
        addToDB(newStudent);
        addToCourseStudents(newStudent);
    }
    exportToFile(); // Export the database to an encrypted file
    remove(FILENAME);  //remove the file from disk, because we have the encrypted file
    fclose(fp);
    free(line);
}

/**
 * Read the encrypted file
 */
void readEncryptedFile()
{
    // Encrypted file exists, read and decrypt from it
    FILE* fp = openFile(ENCRYPTED_FILENAME, "rb");
    unsigned char ciphertext[128], plaintext[128];
    int ciphertext_len;

    while (!feof(fp)) {
        fread(&ciphertext_len, sizeof(int), 1, fp);
        fread(ciphertext, sizeof(unsigned char), ciphertext_len, fp);
        decrypt(ciphertext, ciphertext_len, key, iv, (unsigned char*)plaintext);
        Student* newStudent = parseStudent((const char*)plaintext);
        if (newStudent == NULL) {
            fclose(fp);
            error("Error parsing student data from file");
        }
        addToDB(newStudent);
        addToCourseStudents(newStudent);
    }
    fclose(fp);
}

/**
 * Parse the line and create a new student
 * @param line - the student line data
 * @return pointer to the new student
 */
Student* parseStudent(const char* line)
{
    Student *newStudent = allocateStudent();
    int sumOfGrades = 0, bytesConsumed = 0, result;

    // read student data
    result = sscanf(line, "%s %s %s %d %d%n", newStudent->firstName, newStudent->lastName,
                    newStudent->phone, &newStudent->levelID, &newStudent->classID, &bytesConsumed);

    if(result != 5 || newStudent->levelID < 1 || newStudent->levelID > NUM_OF_LEVELS ||
                      newStudent->classID < 1 || newStudent->classID > NUM_OF_CLASSES) {
        free(newStudent);
        return NULL;
    }

    // read student grades
    int i, bytes;
    for (i = 0; i < NUM_OF_COURSES; i++) {
        if (sscanf(line+bytesConsumed, "%d%n", &newStudent->grades[i], &bytes) != 1) {
            free(newStudent);
            return NULL;
        }
        bytesConsumed += bytes;
        sumOfGrades += newStudent->grades[i];
    }
    newStudent->avgGrade = sumOfGrades / NUM_OF_COURSES;
    return newStudent;
}

/**
 * Allocate memory for a new student
 * @return pointer to the new student
 */
Student* allocateStudent(){
    Student* student = malloc(sizeof(Student));
    if(student == NULL) {
        error("Error allocating memory for new student");
    }
    student->next = student->prev = NULL;
    return student;
}

/**
 * Print the school database
 */
void printDB()
{
    for (int level = 0; level < NUM_OF_LEVELS; level++) {
        for (int class = 0; class < NUM_OF_CLASSES; class++) {
            Student* curr = school.DB[level][class];
            while (curr != NULL) {
                printStudent(curr);
                curr = curr->next;
            }
        }
    }
}

/**
 * Print a student record
 * @param student
 */
void printStudent(Student* student)
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
    printf("=======================================\n\n");
}

/**
 * Add a new student to the database
 * in the correct position based on the average grade
 * @param newStudent
 */
void addToDB(Student* newStudent) {
    int i = newStudent->levelID - 1;
    int j = newStudent->classID - 1;

    if (school.DB[i][j] == NULL || newStudent->avgGrade < school.DB[i][j]->avgGrade) {
        newStudent->next = school.DB[i][j];
        newStudent->prev = NULL;
        if (school.DB[i][j] != NULL) {
            school.DB[i][j]->prev = newStudent;
        }
        school.DB[i][j] = newStudent;
        return;
    }

    Student* curr = school.DB[i][j];
    while (curr->next != NULL && newStudent->avgGrade >= curr->next->avgGrade) {
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
 * Add a new student to the course students list
 * in the correct position based on the grade in the current course
 * @param newStudent
 */
void addToCourseStudents(Student* newStudent)
{
    int levelID = newStudent->levelID-1;
    for(int i = 0; i < NUM_OF_COURSES; i++)
    {
        CourseStudent* courseStudent = allocateCourseStudent();
        courseStudent->student = newStudent;
        CourseStudent *prev = NULL;
        CourseStudent *curr = courseStudents[levelID][i];
        // find the correct position to insert the new student based on the grade in the current course
        while (curr != NULL && newStudent->grades[i] <= curr->student->grades[i]) {
            prev = curr;
            curr = curr->next;
        }
        if (prev == NULL) { // insert at the beginning of the list
            courseStudent->next = courseStudents[levelID][i];
            courseStudents[levelID][i] = courseStudent;
        } else {
            courseStudent->next = curr;
            prev->next = courseStudent;
        }
    }
}

/**
 * Allocate memory for a new course student
 */
CourseStudent* allocateCourseStudent()
{
    CourseStudent* courseStudent = malloc(sizeof(CourseStudent));
    if(courseStudent == NULL) {
        error("Error allocating memory.");
    }
    courseStudent->next = NULL;
    return courseStudent;
}

/**
 * Delete a student from the database
 * @param student
 */
void deleteFromDB(Student* student)
{
    // Student found, delete the student from the linked list
    if (student->prev == NULL) {
        school.DB[student->levelID-1][student->classID-1] = student->next;
    } else {
        student->prev->next = student->next;
    }
    if (student->next != NULL) {
        student->next->prev = student->prev;
    }
    // Free memory occupied by the student
    free(student);
    printf("Student deleted successfully.\n");
}

/**
 * Delete a student from the course students list
 * @param student
 */
void deleteFromCourseStudents(Student *student)
{
    int levelID = student->levelID-1;
    for(int i = 0; i < NUM_OF_COURSES; i++)
    {
        CourseStudent *prev = NULL;
        CourseStudent *curr = courseStudents[levelID][i];
        while (curr != NULL && curr->student != student) {
            prev = curr;
            curr = curr->next;
        }
        if (curr == NULL) {
            continue;
        }
        if (prev == NULL) {
            courseStudents[levelID][i] = curr->next;
        } else {
            prev->next = curr->next;
        }
        free(curr);
    }
}

/**
 * user interface to perform operations on the database
 */
void menu()
{
    int command, result;
    do {
        showMenu();
        result = scanf("%d", &command);
        printf("\n");
        clearBuffer();
        if (result != 1) {
            fprintf(stderr, "Error reading user input.\n");
            continue;
        }
        executeCommand(command);
    } while (command != 0);
}


/**
 * Print the menu options
 */
void showMenu()
{
    printf("\n================= Menu =================\n");
    printf("0. Exit\n");
    printf("1. register new student\n");
    printf("2. Delete a student\n");
    printf("3. Update student details\n");
    printf("4. Search student by name\n");
    printf("5. Top10 students in a course for each level\n");
    printf("6. Potential dropouts students\n");
    printf("7. Average grade per course per level\n");
    printf("8. Print database\n");
    printf("9. Export to file\n");
    printf("Enter your choice: ");
}

/**
 * Execute the user's choice
 * @param command
 */
void executeCommand(int command)
{
    switch (command) {
        case 0:
            printf("Goodbye!\n");
            break;
        case 1:
            registerStudent();
            break;
        case 2:
            deleteStudent();
            break;
        case 3:
            updateStudent();
            break;
        case 4:
            searchStudentByName();
            break;
        case 5:
            excellentStudents(TOP10);
            break;
        case 6:
            potentialDropouts();
            break;
        case 7:
            averagePerCoursesPerLevel();
            break;
        case 8:
            printDB();
            break;
        case 9:
            exportToFile();
            break;
        default:
            printf("Invalid choice. Please try again.\n");
            break;
    }
}

/**
 * Read student details from the user and add the student to the database
 */
void registerStudent()
{
    char* line = NULL;
    size_t len = 0;
    printf("Enter student details in the following format:\n"
                  "(firstName, lastName, phone, level, class, grades)\n");
    if(getline(&line, &len, stdin) == -1) {
        fprintf(stderr, "Error reading user input.\n");
        free(line);
        return;
    }
    Student* newStudent = parseStudent(line);
    if (newStudent == NULL) {
        fprintf(stderr, "Invalid input.\n");
        free(line);
        return;
    }
    addToDB(newStudent);
    addToCourseStudents(newStudent);
    free(line);
}

/**
 * Delete a student record from the database
 */
void deleteStudent()
{
    // Read student phone number from user
    int level, class;
    char phone[PHONE_LEN];

    printf("Enter student details in the following format:\n"
                  "(level, class, phone)\n");
    if(scanf("%d %d %s", &level, &class, phone) != 3) {
        fprintf(stderr, "Invalid input.\n");
        clearBuffer();
        return;
    }
    // Find the student in the specified level and class
    Student* curr = school.DB[level-1][class-1];
    while (curr != NULL) {
        if (strcmp(curr->phone, phone) == 0){
            deleteFromCourseStudents(curr);
            deleteFromDB(curr);
            return;
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
Student* searchStudentByName()
{
    char firstName[NAME_LEN], lastName[NAME_LEN];

    printf("Enter student name in the following format:\n"
                  "(firstName, lastName)\n");
    if(scanf("%s %s", firstName, lastName) != 2) {
        fprintf(stderr, "Invalid input.\n");
        clearBuffer();
        return NULL;
    }
    for (int level = 0; level < NUM_OF_LEVELS; level++) {
        for (int class = 0; class < NUM_OF_CLASSES; class++) {
            Student* curr = school.DB[level][class];
            while (curr != NULL) {
                if (strcmp(curr->firstName, firstName) == 0 &&
                    strcmp(curr->lastName, lastName) == 0) {
                    // Found the student
                    printStudent(curr);
                    return curr;
                }
                curr = curr->next;
            }
        }
    }
    printf("Student not found.\n");
    return NULL;
}

/**
 * Update a student record
 * @param student
 */
void updateStudent()
{
    int choice;

    printf("Update student details:\n");
    Student* student = searchStudentByName(); //search for the student to update
    if (student == NULL) {
        return;
    }
    do {
        showUpdateMenu();
        printf("Enter your choice: ");
        if(scanf("%d", &choice) != 1) {
            fprintf(stderr, "Invalid input.\n");
            clearBuffer();
            continue;
        }
        clearBuffer();
        executeUpdate(student, choice);
    } while (choice != 0);
}

/**
 * Print the update menu options
 */
void showUpdateMenu()
{
    printf("\n============= Update Menu =============\n");
    printf("What field would you like to update?\n");
    printf("0. Done update\n");
    printf("1. First name\n");
    printf("2. Last name\n");
    printf("3. Phone\n");
    printf("4. Level\n");
    printf("5. Class\n");
    printf("6. Grades\n");
}

/**
 * Execute the user's update choice
 * @param student
 * @param choice
 */
void executeUpdate(Student* student, int choice) {
    switch (choice)
    {
        case 0:
            break;
        case 1:
            printf("Enter new first name: ");
            scanf("%s", student->firstName);
            break;
        case 2:
            printf("Enter new last name: ");
            scanf("%s", student->lastName);
            break;
        case 3:
            printf("Enter new phone number: ");
            scanf("%s", student->phone);
            break;
        case 4:
            printf("Enter new level: ");
            scanf("%d", &student->levelID);
            break;
        case 5:
            printf("Enter new class: ");
            scanf("%d", &student->classID);
            break;
        case 6:
            printf("Enter new grades: ");
            for (int i = 0; i < NUM_OF_COURSES; i++) {
                scanf("%d", &student->grades[i]);
            }
            break;
        default:
            printf("Invalid choice. Please try again.\n");
            break;
    }
}

/**
 * Print the top students in a given course
 * @param numOfStudents
 */
void excellentStudents(int numOfStudents)
{
    int course;
    printf("Enter course number: ");
    if(scanf("%d", &course) != 1 || course < 1 || course > NUM_OF_COURSES) {
        fprintf(stderr, "Invalid input.\n");
        clearBuffer();
        return;
    }
    for (int level = 0; level < NUM_OF_LEVELS; level++) {
        printf("Level %d excellent students, in course %d:\n", level+1, course);
        CourseStudent* curr = courseStudents[level][course-1];
        int i = 0;
        while (curr != NULL && i < numOfStudents) {
            printStudent(curr->student);
            curr = curr->next;
            i++;
        }
    }
}

/**
 * Print the potential dropouts students of the school
 * the worst student of each class (only if his average is below 65)
 */
void potentialDropouts()
{
    printf("Potential dropouts students:\n");
    for(int level = 0; level < NUM_OF_LEVELS; level++) {
        for (int class = 0; class < NUM_OF_CLASSES; class++) {
            //the student with the lowest average grade in his class
            Student* curr = school.DB[level][class];
            if (curr != NULL && curr->avgGrade < 65) {
                printStudent(curr);
            }
        }
    }
}

/**
 * Print the average grade of each course in each level
 */
void averagePerCoursesPerLevel()
{
    printf("Average grade per course:\n");
    for(int level = 0; level < NUM_OF_LEVELS; level++) {
        for (int course = 0; course < NUM_OF_COURSES; course++) {
            int sum = 0, numOfStudents = 0;
            CourseStudent* curr = courseStudents[level][course];
            while (curr != NULL) {
                sum += curr->student->grades[course];
                numOfStudents++;
                curr = curr->next;
            }
            if (numOfStudents > 0) {
                printf("Level: %d, Course: %d: Average: %d\n", level+1, course+1, sum/numOfStudents);
            }
        }
    }
}

/**
 * Export the database to a file
 */
void exportToFile()
{
    FILE* fp = openFile(ENCRYPTED_FILENAME, "wb"); // Open in binary mode for encryption
    for (int level = 0; level < NUM_OF_LEVELS; level++) {
        for (int class = 0; class < NUM_OF_CLASSES; class++) {
            Student* curr = school.DB[level][class];
            while (curr != NULL) {
                unsigned char plaintext[128], ciphertext[128];
                int ciphertext_len = 0, plaintext_len;
                plaintext_len = snprintf((char*)plaintext, sizeof(plaintext),
                                         "%s %s %s %d %d %d %d %d %d %d %d %d %d %d %d\n",
                                         curr->firstName, curr->lastName, curr->phone, curr->levelID, curr->classID,
                                         curr->grades[0], curr->grades[1], curr->grades[2], curr->grades[3],
                                         curr->grades[4], curr->grades[5], curr->grades[6], curr->grades[7],
                                         curr->grades[8], curr->grades[9]);
                ciphertext_len = encrypt(plaintext, plaintext_len, key, iv, ciphertext);
                fwrite(&ciphertext_len, sizeof(int), 1, fp);
                fwrite(ciphertext, sizeof(unsigned char), ciphertext_len, fp);
                curr = curr->next;
            }
        }
    }
    fclose(fp);
}

/**
 * Clear the input buffer
 */
void clearBuffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/**
 * Free the memory allocated for the database
 */
void freeDB()
{
    for (int level = 0; level < NUM_OF_LEVELS; level++) {
        freeCourseStudents(level);
        freeStudents(level);
    }
}

/**
 * Free the memory allocated for the course students list
 * @param level
 */
void freeCourseStudents(int level)
{
    for (int course = 0; course < NUM_OF_COURSES; course++) {
        CourseStudent* curr = courseStudents[level][course];
        while (curr != NULL) {
            CourseStudent* temp = curr;
            curr = curr->next;
            free(temp);
        }
    }
}

/**
 * Free the memory allocated for the students list
 * @param level
 */
void freeStudents(int level)
{
    for (int class = 0; class < NUM_OF_CLASSES; class++) {
        Student* curr = school.DB[level][class];
        while (curr != NULL) {
            Student* temp = curr;
            curr = curr->next;
            free(temp);
        }
    }
}

/**
 * Print an error message and exit the program
 * @param msg
 */
void error(const char* msg)
{
    fprintf(stderr, "Error: %s\n", msg);
    freeDB();
    exit(EXIT_FAILURE);
}


/**
 * Encrypt the plaintext with the key and iv
 * @param plaintext - the plaintext to be encrypted
 * @param plaintext_len - the length of the plaintext
 * @param key - the key used to encrypt the plaintext
 * @param iv - the iv used to encrypt the plaintext
 * @param ciphertext - the ciphertext after encryption
 * @return int - the length of the ciphertext
 */
int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
            unsigned char *iv, unsigned char *ciphertext)
{
    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;

    // create and initialise the context
    if(!(ctx = EVP_CIPHER_CTX_new())) {
        handleEncryptErrors();
    }

    // initialise the encryption operation
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
        handleEncryptErrors();
    }

    // provide the message to be encrypted, and obtain the encrypted output
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len)) {
        handleEncryptErrors();
    }
    ciphertext_len = len;

    //Finalise the encryption
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) {
        handleEncryptErrors();
    }
    ciphertext_len += len;

    /// clean up
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}

/**
 * Decrypt the ciphertext with the key and iv
 * @param ciphertext - the ciphertext to be decrypted
 * @param ciphertext_len - the length of the ciphertext
 * @param key - the key used to decrypt the ciphertext
 * @param iv - the iv used to decrypt the ciphertext
 * @param plaintext - the plaintext after decryption
 * @return int - the length of the plaintext
 */
int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
            unsigned char *iv, unsigned char *plaintext)
{
    EVP_CIPHER_CTX *ctx;
    int len;
    int plaintext_len;

    // create and initialise the context
    if(!(ctx = EVP_CIPHER_CTX_new())) {
        handleEncryptErrors();
    }

    // initialise the decryption operation
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
        handleEncryptErrors();
    }

    // provide the message to be decrypted
    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        handleEncryptErrors();
    plaintext_len = len;

    // finalise the decryption
    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
        handleEncryptErrors();
    plaintext_len += len;
    plaintext[plaintext_len] = '\0'; // Add null terminator

    // clean up
    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}

/**
 * Handle errors of the encryption
 */
void handleEncryptErrors()
{
    ERR_print_errors_fp(stderr);
    abort();
}

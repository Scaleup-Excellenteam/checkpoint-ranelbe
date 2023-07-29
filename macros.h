#define NUM_OF_LEVELS 12
#define NUM_OF_CLASSES 10
#define NUM_OF_COURSES 10
#define NAME_LEN 50
#define PHONE_LEN 50
#define TOP10 10
#define FILENAME "students.txt"
#define ENCRYPTED_FILENAME "students.enc"


typedef struct Student {
    char firstName[NAME_LEN];
    char lastName[NAME_LEN];
    char phone[PHONE_LEN];
    int grades[NUM_OF_COURSES];
    int avgGrade;
    int levelID;
    int classID;

    // Linked list pointers - bidirectional
    struct Student* next;
    struct Student* prev;
} Student;

typedef struct School {
    struct Student* DB[NUM_OF_LEVELS][NUM_OF_CLASSES];
} School;

typedef struct CourseStudent {
    struct Student* student;
    struct CourseStudent* next;
} CourseStudent;

FILE* openFile(const char* filename,const char* mode);
void initDB();
void readRegularFile();
void readEncryptedFile();
Student* parseStudent(const char* line);
Student* allocateStudent();
void printDB();
void printStudent(Student* student);
void addToDB(Student* newStudent);
void addToCourseStudents(Student* newStudent);
CourseStudent* allocateCourseStudent();
void deleteFromDB(Student* student);
void deleteFromCourseStudents(Student* student);
void menu();
void showMenu();
void executeCommand(int command);
void registerStudent();
void deleteStudent();
Student* searchStudentByName();
void updateStudent();
void showUpdateMenu();
void executeUpdate(Student* student, int choice);
void excellentStudents(int numOfStudents);
void potentialDropouts();
void averagePerCoursesPerLevel();
void exportToFile();
void clearBuffer();
void freeDB();
void freeStudents(int level);
void freeCourseStudents(int level);
void error(const char* msg);
int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
            unsigned char *iv, unsigned char *ciphertext);
int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
            unsigned char *iv, unsigned char *plaintext);
void handleEncryptErrors();



// for debugging purposes I used hardcoded key and iv
unsigned char key[32] = { 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
                          0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
                          0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
                          0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31
};
unsigned char iv[16] = { 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
                         0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35
};
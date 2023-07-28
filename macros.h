#define NUM_OF_LEVELS 12
#define NUM_OF_CLASSES 10
#define NUM_OF_COURSES 10
#define NAME_LEN 50
#define PHONE_LEN 50
#define TOP10 10
#define FILENAME "students.txt"


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
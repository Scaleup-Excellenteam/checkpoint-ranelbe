#define NUM_OF_LEVELS 12
#define NUM_OF_CLASSES 10
#define NUM_OF_COURSES 10
#define NAME_LEN 128
#define PHONE_LEN 10
#define FILENAME "students.txt"


struct Student {
    char firstName[NAME_LEN];
    char lastName[NAME_LEN];
    char phone[PHONE_LEN];
    int grades[NUM_OF_COURSES];
    int levelID;
    int classID;

    // Linked list pointers - bidirectional
    struct Student* next;
    struct Student* prev;
};

struct School {
    struct Student* DB[NUM_OF_LEVELS][NUM_OF_CLASSES];
};
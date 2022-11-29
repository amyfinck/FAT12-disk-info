#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

typedef struct subdirectory Subdirectory;

struct subdirectory{
    int offset;
    char* subdirName;
    Subdirectory* next;
};

typedef struct customer_info CustomerInfo;

Subdirectory* addSubdir(int offset, char* subdirName, Subdirectory* head);
Subdirectory* deleteSub(int offset, Subdirectory* head);
#endif


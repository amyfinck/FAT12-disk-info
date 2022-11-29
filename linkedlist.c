#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "linkedlist.h"


Subdirectory* addSubdir(int offset, char* subdirName, Subdirectory* head)
{
    Subdirectory * new_sub = (Subdirectory *) malloc(sizeof(Subdirectory));

    new_sub->offset = offset;
    new_sub->subdirName = subdirName;
    new_sub->next = NULL;


    if(head == NULL)
    {
        head = new_sub;
    }
    else
    {
        Subdirectory * temp = head;
        while(temp->next != NULL)
        {
            temp = temp->next;
        }
        temp->next = new_sub;
    }
    return head;
}

Subdirectory* deleteSub(int offset, Subdirectory* head)
{
    Subdirectory * temp;
    temp = head;

    if(head->offset == offset)
    {
        head = head->next;
    }
    else
    {
        do
        {
            if (temp->next->offset == offset)
            {
                temp->next = temp->next->next;
            }
            temp = temp->next;
        }
        while (temp != NULL);
    }
    return head;
}

//void printList()
//{
//    int count = 0;
//    CustomerInfo* temp = head;
//
//    while(temp != NULL)
//    {
//        printf("customer %d - Class:%d, arrival time: %d, service time: %d, \n", temp->user_id, temp->class_type, temp->arrival_time, temp->service_time);
//        count++;
//        temp = temp->next;
//    }
//    printf("Total customers: %d\n", count);
//}
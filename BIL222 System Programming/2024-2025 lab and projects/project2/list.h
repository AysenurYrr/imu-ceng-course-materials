/**
 * @file list.h
 * @author adaskin
 * @brief 
 * @version 0.1
 * @date 2024-04-21
 * 
 * @copyright Copyright (c) 2024
 */
#ifndef LIST_H
#define LIST_H

#include <time.h>
#include <pthread.h>
#include <semaphore.h>


typedef struct node {
    struct node *prev;
    struct node *next;
    // size_t size; /*sizes are fixed for convenience*/
    char occupied;
    char data[];
} Node;

typedef struct list {
    Node *head;
    Node *tail;
    int number_of_elements; /*TODO: make semaphore*/
    int capacity;           /*TODO make semaphore*/
    int datasize; /*only data[] size in the node, e.g. sizeof(Survivor)*/
    int nodesize;     /*this includes next, prev pointer sizes*/
    char *startaddress;
    char *endaddress;
    Node *lastprocessed;
    Node *free_list; /* Track freed nodes */

    pthread_mutex_t lock; /*controls all access to the list*/
    sem_t empty;     /* tracks empty slots */
    sem_t full;      /* tracks filled slots */
    
    /*ops on the list*/
    Node *(*add)(struct list *list, void *data);
    int  (*removedata)(struct list *list, void *data);
    int (*removenode)(struct list *list, Node *node); 
    void *(*pop)(struct list *list, void* dest);
    void *(*peek)(struct list *list);
    void (*destroy)(struct list *list);
    void (*printlist)(struct list *list, void (*print)(void*));
    void (*printlistfromtail)(struct list *list, void (*print)(void*));
} List;

List *create_list(size_t datasize, int capacity);
int removenode(List *list, Node *node);
Node *add(List *list, void *data);
int removedata(List *list, void *data);
void *pop(List *list, void *dest);
void *peek(List *list);
void destroy(List *list);
void printlist(List *list, void (*print)(void*));
void printlistfromtail(List *list, void (*print)(void*));

#endif
// LIST_H
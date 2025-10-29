/**
 * @file list.c
 * @author adaskin
 * @brief  a simple doubly linked list stored in an array(contigous
 * memory). this program is written for educational purposes 
 * and may include some bugs.
 * TODO: add synchronization
 * @version 0.1
 * @date 2024-04-21
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "headers/list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>


/**
 * @brief Create a list object, allocates new memory for list, and
 * sets its data members
 *
 * @param datasize: size of data in each node
 * @param capacity: maximum number of nodes can be stored in this list
 * @return List*
 */
List *create_list(size_t datasize, int capacity) {
    printf("[DEBUG] create_list çağrıldı (datasize=%zu, capacity=%d)\n", datasize, capacity);
    List *list = malloc(sizeof(List));
    memset(list, 0, sizeof(List));

    printf("[DEBUG] create_list: mutex init öncesi (mutex addr: %p)\n", &list->lock);
    int mutex_init_result = pthread_mutex_init(&list->lock, NULL);
    printf("[DEBUG] create_list: mutex init sonrası (result: %d)\n", mutex_init_result);
    
    if (mutex_init_result != 0) {
        printf("[ERROR] Mutex initialization failed with error: %d\n", mutex_init_result);
        free(list);
        return NULL;
    }

    int sem_empty_result = sem_init(&list->empty, 0, capacity);
    if (sem_empty_result != 0) {
        printf("[ERROR] Empty semaphore initialization failed with error: %d\n", sem_empty_result);
        pthread_mutex_destroy(&list->lock);
        free(list);
        return NULL;
    }

    int sem_full_result = sem_init(&list->full, 0, 0);
    if (sem_full_result != 0) {
        printf("[ERROR] Full semaphore initialization failed with error: %d\n", sem_full_result);
        sem_destroy(&list->empty);
        pthread_mutex_destroy(&list->lock);
        free(list);
        return NULL;
    }

    list->datasize = datasize;
    list->nodesize = sizeof(Node) + datasize;

    list->startaddress = malloc(list->nodesize * capacity);
    if (list->startaddress == NULL) {
        printf("[ERROR] Memory allocation failed for startaddress\n");
        sem_destroy(&list->empty);
        sem_destroy(&list->full);
        pthread_mutex_destroy(&list->lock);
        free(list);
        return NULL;
    }

    list->endaddress = list->startaddress + (list->nodesize * capacity);
    memset(list->startaddress, 0, list->nodesize * capacity);

    list->lastprocessed = (Node *)list->startaddress;

    list->number_of_elements = 0;
    list->capacity = capacity;

    /*ops*/
    list->add = add;
    list->removedata = removedata;
    list->removenode = removenode;
    list->pop = pop;
    list->peek = peek;
    list->destroy = destroy;
    list->printlist = printlist;
    list->printlistfromtail = printlistfromtail;
    printf("[DEBUG] create_list: fonksiyon sonu\n");
    return list;
}
/**
 * @brief finds a memory cell in the mem area of list
 * @param list
 * @return Node*
 */
static Node *find_memcell_fornode(List *list) {
    Node *node = NULL;
    /*search lastprocessed---end*/
    Node *temp = list->lastprocessed;
    while ((char *)temp < list->endaddress) {
        if (temp->occupied == 0) {
            node = temp;
            break;
        } else {
            temp = (Node *)((char *)temp + list->nodesize);
        }
    }
    if (node == NULL) {
        /*search startaddress--lastprocessed*/
        temp = (Node *)list->startaddress;
        while (temp < list->lastprocessed) {
            if (temp->occupied == 0) {
                node = temp;
                break;
            } else {
                temp = (Node *)((char *)temp + list->nodesize);
            }
        }
    }
    return node;
}

/**
 * @brief find an unoccupied node in the array, and makes a node with
 * the given data and ADDS it to the HEAD of the list
 * @param list:
 * @param data: a data addrress, its size is determined from
 * list->datasize
 * @return * find,*
 */
Node *add(List *list, void *data) {
    printf("[DEBUG] add fonksiyonu çağrıldı (thread: %lu)\n", pthread_self());
    Node *node = NULL;
    
    printf("[DEBUG] add: sem_wait öncesi (thread: %lu)\n", pthread_self());
    // Boş slot bekle
    sem_wait(&list->empty);
    printf("[DEBUG] add: sem_wait sonrası (thread: %lu)\n", pthread_self());
    
    printf("[DEBUG] add: mutex lock öncesi (thread: %lu, mutex addr: %p)\n", pthread_self(), &list->lock);
    int lock_result = pthread_mutex_lock(&list->lock);
    printf("[DEBUG] add: mutex lock sonrası (thread: %lu, result: %d)\n", pthread_self(), lock_result);

    if (lock_result != 0) {
        printf("[ERROR] Mutex lock failed with error: %d\n", lock_result);
        sem_post(&list->empty);
        return NULL;
    }

    /*TODO use semaphores..!*/
    if (list->number_of_elements >= list->capacity) {
        perror("list is full!");
        pthread_mutex_unlock(&list->lock);
        sem_post(&list->empty);
        return NULL;
    }

    /*first find an unoccupied memcell and insert into it*/
    node = find_memcell_fornode(list);

    if (node != NULL) {
        /*create_node*/
        node->occupied = 1;
        memcpy(node->data, data, list->datasize);

        /* Liste boşsa */
        if (list->head != NULL) {
            Node *oldhead = list->head;
            oldhead->prev = node;
            node->prev = NULL;
            node->next = oldhead;
        }else {                          /* Liste doluysa */
            node->prev = NULL;
            node->next = NULL;            
        }

        list->head = node;
        list->lastprocessed = node;
        list->number_of_elements += 1;
        if (list->tail == NULL) {
            list->tail = list->head;
        }
    } else {
        perror("list is full!");
    }
    
    printf("[DEBUG] add: mutex unlock öncesi (thread: %lu)\n", pthread_self());
    int unlock_result = pthread_mutex_unlock(&list->lock);
    printf("[DEBUG] add: mutex unlock sonrası (thread: %lu, result: %d)\n", pthread_self(), unlock_result);
    
    if (node != NULL) {
        // Dolu slot sayısını artır
        sem_post(&list->full);
    } else {
        // Eğer node eklenemediyse boş slotu geri ver
        sem_post(&list->empty);
    }
    printf("[DEBUG] add fonksiyonu bitiyor (thread: %lu)\n", pthread_self());
    return node;
}
/**
 * @brief finds the node with the value same as the mem pointed by
 * data and removes that node. it returns temp->node
 * @param list
 * @param data
 * @return int: in success, it returns 0; if not found it returns 1.
 */
int removedata(List *list, void *data)
{
    pthread_mutex_lock(&list->lock);

     /* 1) Aranan düğümü bul */
    Node *temp = list->head;
    while (temp && memcmp(temp->data, data, list->datasize) != 0)
        temp = temp->next;

     if (!temp) {                       /* bulunamadı  */
        pthread_mutex_unlock(&list->lock);
        return 1;
    }

     /* 2) Bağlantıları kopar */
    Node *prev = temp->prev;
    Node *next = temp->next;
    if (prev) prev->next = next;
    if (next) next->prev = prev;

     /* 3) HEAD / TAIL düzelt */
    if (temp == list->head) list->head = next;
    if (temp == list->tail) list->tail = prev;

     /* 4) Metadatayı sıfırla */
    temp->next = temp->prev = NULL;
    temp->occupied          = 0;
    list->number_of_elements--;

     pthread_mutex_unlock(&list->lock);
    sem_post(&list->empty);            /* boş slot açıldı */
    return 0;
 }

/**
 * @brief removes the node from list->head, and copies its data into
 * dest, also returns it.
 * @param list
 * @param dest: address to cpy data
 * @return void*: if there is data, it returns address of dest; else
 * it returns NULL.
 */
void *pop(List *list, void *dest) {
    void *result = NULL;
    
    // Dolu slot bekle
    sem_wait(&list->full);
    
    pthread_mutex_lock(&list->lock);
    
    if (list->head != NULL) {
        Node *node = list->head;
        if (removenode(list, node) == 0) {
            memcpy(dest, node->data, list->datasize);
            result = dest;
        }
    }
    
    pthread_mutex_unlock(&list->lock);
    
    if (result != NULL) {
        // Boş slot sayısını artır
        sem_post(&list->empty);
    } else {
        // Eğer pop başarısız olduysa dolu slotu geri ver
        sem_post(&list->full);
    }
    
    return result;
}
/**
 * @brief returns the data stored in the head of the list
 * @param list
 * @return void*: returns the address of head->data
 */
void *peek(List *list) {
    if (list->head != NULL) return list->head->data;

    return NULL;
}

/**
 * @brief removes the given node from the list, it returns removed
 * node.
 * @param list
 * @param node
 * @return int: in sucess, it returns 0; if node not found, it
 * returns 1.
 */
int removenode(List *list, Node *node) {
    pthread_mutex_lock(&list->lock);
    
    if (node != NULL) {
        Node *prevnode = node->prev;
        Node *nextnode = node->next;
        if (prevnode != NULL) {
            prevnode->next = nextnode;
        }
        if (nextnode != NULL) {
            nextnode->prev = prevnode;
        }
        node->next = NULL;
        node->prev = NULL;
        node->occupied = 0;

        list->number_of_elements--;

        if (node == list->tail) {
            list->tail = prevnode;
        }

        if (node == list->head) {
            list->head = nextnode;
        }
        list->lastprocessed = node;
        
        // Dolu slot sayısını azalt
        sem_post(&list->empty);
        
        pthread_mutex_unlock(&list->lock);
        return 0;
    }
    
    pthread_mutex_unlock(&list->lock);
    return 1;
}

/**
 * @brief deletes everything
 *
 * @param list
 */
void destroy(List *list) {
    pthread_mutex_destroy(&list->lock);
    sem_destroy(&list->empty);
    sem_destroy(&list->full);
    
    free(list->startaddress);
    memset(list, 0, sizeof(List));
    free(list);
}

/**
 * @brief prints list starting from head
 *
 * @param list
 * @param print: aprint function for the object data.
 */
void printlist(List *list, void (*print)(void *)) {
    pthread_mutex_lock(&list->lock);
    Node *temp = list->head;
    while (temp != NULL) {
        print(temp->data);
        temp = temp->next;
    }
    pthread_mutex_unlock(&list->lock);
}
/**
 * @brief print list starting from tail
 *
 * @param list
 * @param print: print function
 */
void printlistfromtail(List *list, void (*print)(void *)) {
    pthread_mutex_lock(&list->lock);
    Node *temp = list->tail;
    while (temp != NULL) {
        print(temp->data);
        temp = temp->prev;
    }
    pthread_mutex_unlock(&list->lock);
}
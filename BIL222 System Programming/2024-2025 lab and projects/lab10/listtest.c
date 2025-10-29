/*controller file, creates drones
and assigns them*/

#include "../headers/list.h"
#include "../headers/survivor.h"
#include <stdlib.h>
#include <stdio.h>
void printsurvivor(Survivor *s) {
    printf("info: %.25s\n", s->info);
    printf("Location: (%d, %d)\n", s->coord.x, s->coord.y);
}

int main() {
    /*EXAMPLE USE OF list.c*/
    int n = 20;
    int m = 10;
    List *list = create_list(sizeof(Survivor), 100);
 
    printf("\n\nadd elements to the list\n");
    for (int i = 0; i < n; i++) {
        Survivor s;
        sprintf(s.info, "id:%d-aname", i);
        s.coord.x = rand() % 1000;
        s.coord.y = rand() % 100;
        Node *n = list->add(list, &s);
    }

    printlist(list, (void (*)(void *))printsurvivor);
    printf("from tail\n\n");
    printlistfromtail(list, (void (*)(void *))printsurvivor);

    printf("\ntest by removing m elements\n");
     Survivor s;

    for (int i = 0; i < m; i++) {
        if (list->pop(list, &s) != NULL) printf("(%d,%d)-", s.coord.x, s.coord.y);
    }
    printf("\nremaining elements\n");
    printlist(list, (void (*)(void *))printsurvivor);
    list->destroy(list);
    printf("\n");
}

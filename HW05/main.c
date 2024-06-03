#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> 

long ar[] = {4, 8, 15, 16, 23, 42}; 

// Структура односвязный список
struct _one_way_list_element {
    long data;
    struct _one_way_list_element * next;
};
typedef struct _one_way_list_element list_element;

//Реализация функции добавления элемента в односвязный список
void add_element(list_element **head, int data){
    list_element * tmp = (list_element*) malloc(sizeof(list_element));
    tmp->data = data;
    tmp->next = (*head);
    (*head) = tmp;
}


//Реализация аналога функции m из ассемблерного листинга c рекурсией
void print_list(const list_element *head){
    if (head) {
        printf("%ld ", head->data);
        head = head->next;
        print_list(head);
    }
    else {
        printf("\n");
    }
}


//Реализация аналога функции p из ассемблерного листинга
bool IsOdd(long num){
    return num & 1;
}

//Реализация аналога функции f из ассемблерного листинга
void getodd(const list_element *head, list_element **odd_list){
    if (head) {
        if (IsOdd(head->data)) {
            add_element(odd_list, head->data);
        }
        head = head->next;
        getodd(head, odd_list);
    }
}

//Очистка списка и освобождение памяти
void free_list(list_element **head) {
    list_element* prev = NULL;
    while ((*head)->next) {
        prev = (*head);
        (*head) = (*head)->next;
        free(prev);
    }
    free(*head);
}

int main (){
    list_element *list = NULL;
    int length = sizeof(ar)/sizeof(ar[0]);
    for(int i=length-1; i >= 0; --i){
        add_element(&list, ar[i]);
    }
    print_list(list);
    list_element *list2 = NULL;
    getodd(list, &list2);
    print_list(list2);
    free_list(&list);
    free_list(&list2);
}
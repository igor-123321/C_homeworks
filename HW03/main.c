#include "hashtable.h"
#include <stdio.h>
#include <locale.h>
#include <string.h>
//Максимальная длинна слова
#define MAX_LENGTH 45
int main(int argc, char *argv[]){
    if(argc != 2) 
    {
        printf("Use main <path_to_text_file>\n");
        return 0;
    }
    
    FILE *textf = fopen(argv[1], "r");
    //FILE *textf = fopen("text.txt", "r");
    if(textf == NULL){
        //perror("text.txt");
        perror(argv[1]);
        return 1;
    }
    char ch;
    int i = 0;
    char word[MAX_LENGTH]="";
    hashtable *mytable = hashtable_create(10);
    
    while((ch = fgetc(textf)) != EOF){
        if(ch != ' ' && ch != '\t' && ch != '\n' && ch != '.' && ch != '?' && ch != '!' && ch != ',' && ch != ':' && ch != ';')
        { 
            word[i]=ch;
            i++;
        }
        else{
            word[i]='\0';
            i = 0;
            if(strlen(word) == 0){
                continue;
            }
            //if value = 0 then just add word
            int count = hashtable_get(mytable,word);
            if(!count){
                mytable = hashtable_insert(mytable, word, 1);
            }
            else{
                count++;
                hashtable_modify(mytable, word, count);
            }
        }
    }
    fclose(textf);
    for(size_t j=0; j < mytable->size; j++){
        if(mytable->element[j].key != NULL){
            printf("Word %s: count %d.\n", mytable->element[j].key, mytable->element[j].value);
        }
    }
    hashtable_free(mytable);
}
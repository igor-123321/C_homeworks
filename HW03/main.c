#include "hashtable.h"
#include <locale.h>
#include <stdio.h>
#include <bsd/string.h>
//Максимальная длина слова
#define MAX_LENGTH 45
//Начальный размер таблицы
#define START_TABLE_SIZE 10
int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Use main <path_to_text_file>\n");
    return 0;
  }

  FILE *textf = fopen(argv[1], "r");
  // FILE *textf = fopen("text.txt", "r");
  if (textf == NULL) {
    // perror("text.txt");
    perror(argv[1]);
    return 1;
  }
  char cur_ch;
  int i = 0;
  char word[MAX_LENGTH] = "";
  hashtable *mytable = hashtable_create(START_TABLE_SIZE);

  while ((cur_ch = fgetc(textf)) != EOF) {
    if (cur_ch != ' ' && cur_ch != '\t' && cur_ch != '\n' && cur_ch != '.' &&
        cur_ch != '?' && cur_ch != '!' && cur_ch != ',' && cur_ch != ':' &&
        cur_ch != ';') {
      if(i <  MAX_LENGTH){
        word[i] = cur_ch;
        i++;
      } else {
        printf("The maximum word length has been exceeded");
      }
    } else {
      word[i] = '\0';
      i = 0;
      if (strlen(word) == 0) {
        continue;
      }
      // if value = 0 then just add word
      int word_count = hashtable_get(mytable, word);
      if (!word_count) {
        mytable = hashtable_insert(mytable, word, 1);
      } else {
        word_count++;
        hashtable_modify(mytable, word, word_count);
      }
    }
  }
  fclose(textf);
  for (size_t j = 0; j < mytable->size; ++j) {
    if (mytable->element[j].key != NULL) {
      printf("Word %s: word_count %d.\n", mytable->element[j].key,
             mytable->element[j].value);
    }
  }
  hashtable_free(mytable);
}
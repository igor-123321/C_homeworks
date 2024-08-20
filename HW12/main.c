#define _POSIX_C_SOURCE 200809L //для использования функции getline
#include "hashtable.h"
#include <bsd/string.h>
#include <dirent.h>
#include <pthread.h>
#include <regex.h>
#include <semaphore.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define REG_MATCH_SIZE 1
#define START_TABLE_SIZE 500
#define MAX_BUF_SIZE 5000
#define MAX_ITEM_FORSTAT 10
#define THREAD_MAX 12
#define EMPTY_REFFER1 "-"
#define EMPTY_REFFER2 ""

struct _doubletable {
  char *result; //Нужен что бы не затерло адрес table_reffer после возврата из
                //потока
  hashtable *table_reffer;
  hashtable *table_url;
};
typedef struct _doubletable doubletable;

enum LOG_ROW {
  HTTP_TYPE,
  HTTP_PATH,
  HTTP_VER,
  HTTP_CODE,
  HTTP_BYTES,
  HTTP_REFFER,
  LOG_SIZE
};

pthread_mutex_t m; // мьютекс для разграничения доступа
//Общий ресурс для потоков в виде индекса в массиве имен файлов для обработки
int fnum = 0;
//Список файлов для парсинга
char **filesarr;
sem_t sem; // семафор для ограничения кол-ва потоков

char *strndup(const char *s, size_t n) {
  char *p;
  size_t n1;
  for (n1 = 0; n1 < n && s[n1] != '\0'; n1++)
    continue;
  p = (char *)malloc(n + 1);
  if (p != NULL) {
    memcpy(p, s, n1);
    p[n1] = '\0';
  }
  return p;
}

char *reg_strstr(const char *str, const char *pattern) {
  char *result = NULL;
  regex_t re;
  regmatch_t match[REG_MATCH_SIZE];
  if (str == NULL)
    return NULL;
  if (regcomp(&re, pattern, REG_ICASE | REG_EXTENDED) != 0) {
    regfree(&re);
    return NULL;
  }
  if (!regexec(&re, str, (size_t)REG_MATCH_SIZE, match, 0)) {
    /*
    fprintf( stderr, "@@@@@@ Match from %2d to %2d @@@@@@@@@\n",
         match[0].rm_so,
         match[0].rm_eo);
    total_matches++;
    */
    if ((str + match[0].rm_so) != NULL) {
      result = strndup(str + match[0].rm_so, match[0].rm_eo - match[0].rm_so);
    }
  }
  regfree(&re);
  return result;
}

int parce_string(const char *line, char **url, char **reffer,
                 size_t *datasize) {
  //Парсим строку регулярным выдажением
  char *tmp = reg_strstr(
      line, "\"[A-Z]{3,7}\\s\\S*\\sHTTP\\S*\\s[0-9]*\\s[0-9]*\\s\\S*\\s");
  if (tmp == NULL) {
    return 1;
  }
  int start = 0, end = 0;
  int j = 0;
  //Массив строк, разбитых по пробелам
  char *space_addr_array[LOG_SIZE];
  for (size_t i = 0; i < strlen(tmp); ++i) {
    if (tmp[i] == ' ') {
      end = i;
      //Заполнили массив
      space_addr_array[j] = (char *)malloc(end - start);
      if (space_addr_array[j] == NULL) {
        perror("Error while malloc func");
        return 1;
      }
      //Если начало и конец в кавычках - их исключаем
      if (tmp[start + 1] == '\"' && tmp[end - 1] == '\"')
        strlcpy(space_addr_array[j], &(tmp[start + 2]), end - start - 2);
      //Если начало в кавычках
      else if (tmp[start] == '\"')
        strlcpy(space_addr_array[j], &(tmp[start + 1]), end - start);
      //Если конец в кавычках
      else if (tmp[end - 1] == '\"')
        strlcpy(space_addr_array[j], &(tmp[start + 1]), end - start - 1);
      else
        strlcpy(space_addr_array[j], &(tmp[start + 1]), end - start);
      // if(strcmp(space_addr_array[j],"")==0){
      //     printf("%s", line);
      // }
      start = end;
      j++;
    }
  };
  *url = space_addr_array[HTTP_PATH];
  *reffer = space_addr_array[HTTP_REFFER];
  *datasize = strtol(space_addr_array[HTTP_BYTES], NULL, 10);
  free(space_addr_array[HTTP_TYPE]);
  free(space_addr_array[HTTP_CODE]);
  free(space_addr_array[HTTP_VER]);
  free(tmp);
  return 0;
}

void print_statictics(hashtable *table, char *tableinfo) {
  char url_array[MAX_ITEM_FORSTAT][MAX_BUF_SIZE] = {"\0"};
  size_t count_array[MAX_ITEM_FORSTAT];
  size_t len;
  for (size_t j = 0; j < table->size; ++j) {
    if (table->element[j].key != NULL) {
      //Отсекаем пустые reffer`ы
      if (strcmp(table->element[j].key, EMPTY_REFFER1) == 0 ||
          strcmp(table->element[j].key, EMPTY_REFFER2) == 0) {
        continue;
      }
      len = strlen(table->element[j].key);
      for (int i = 0; i < MAX_ITEM_FORSTAT; ++i) {
        if (url_array[i][0] == '\0') {
          // url_array[i] = (char *)malloc(len);
          strlcpy(url_array[i], table->element[j].key, len + 1);
          count_array[i] = table->element[j].value;
          break;
        } else {
          if (count_array[i] < table->element[j].value) {
            for (int k = MAX_ITEM_FORSTAT - 1; k > i; --k) {
              strlcpy(url_array[k], url_array[k - 1],
                      strlen(url_array[k - 1]) + 1);
              count_array[k] = count_array[k - 1];
            }
            strlcpy(url_array[i], table->element[j].key, len + 1);
            count_array[i] = table->element[j].value;
            break;
          }
        }
      }
      // table->element[j].key,
      // table->element[j].value
    }
  }
  printf("%s\n", tableinfo);
  for (int i = 0; i < MAX_ITEM_FORSTAT; ++i) {
    printf("Item %d: url: %s || count: %ld\n", i + 1, url_array[i],
           count_array[i]);
  }
}

void *parce_file(void *statistictable) {
  sem_wait(&sem);
  doubletable *maintable = (doubletable *)statistictable;

  pthread_mutex_lock(&m);
  printf("Working with file %s\n", filesarr[fnum]);
  FILE *textf = fopen(filesarr[fnum], "r");
  if (textf == NULL) {
    perror(filesarr[fnum]);
    pthread_mutex_unlock(&m);
    return NULL;
  }
  fnum++;
  pthread_mutex_unlock(&m);

  size_t datasize, len = 0;
  int line_num = 0;
  char *line = NULL, *url = NULL, *reffer = NULL;

  hashtable *reffer_table = hashtable_create(START_TABLE_SIZE);
  hashtable *url_table = hashtable_create(START_TABLE_SIZE);

  while (getline(&line, &len, textf) != EOF) {
    line_num++;
    // printf("Parsing line %d\n", line_num);
    if (parce_string(line, &url, &reffer, &datasize)) {
      // printf("problem with parce line %d\n", line_num);
      // printf("%s", line);
      continue;
    }
    //Заполняем таблицу reffer
    filltable(&reffer_table, reffer, 1);
    //Заполняем таблицу url
    filltable(&url_table, url, datasize);
    free(url);
    free(reffer);
  }
  free(line);
  fclose(textf);

  pthread_mutex_lock(&m);
  hashtable_merge(&(maintable->table_reffer), reffer_table);
  hashtable_merge(&(maintable->table_url), url_table);
  pthread_mutex_unlock(&m);
  sem_post(&sem);
  return NULL;
}

char *concat(int count, ...) {
  va_list ap;
  // Find required length to store merged string
  int len = 1; // room for NULL
  va_start(ap, count);
  for (int i = 0; i < count; i++)
    len += strlen(va_arg(ap, char *));
  va_end(ap);

  // Allocate memory to concat strings
  char *merged = (char *)calloc(sizeof(char), len);
  int null_pos = 0;

  // Actually concatenate strings
  va_start(ap, count);
  for (int i = 0; i < count; i++) {
    char *s = va_arg(ap, char *);
    size_t s_length = strlen(s);
    strlcpy(merged + null_pos, s, s_length + 1);
    null_pos += strlen(s);
  }
  va_end(ap);

  return merged;
}

char **filelist(char *dirpath, size_t *size) {
  DIR *dir;
  struct dirent *ent;
  char **filesarr = NULL;
  int i = 0;
  if (!(dir = opendir(dirpath))) {
    *size = 0;
    return NULL;
  }
  while ((ent = readdir(dir)) != NULL) {
    if (ent->d_type == 4) {
      continue;
    }
    if (i == 0) {
      filesarr = malloc(sizeof(char *) * (i + 1));
    } else {
      filesarr = realloc(filesarr, sizeof(char *) * (i + 1));
    }
    filesarr[i] = concat(3, dirpath, "/", ent->d_name);
    i++;
  }
  *size = i;
  return filesarr;
}

int main(int argc, char *argv[]) {
  int threadnum = 1;
  size_t filelen;
  if (argc != 3) {
    printf("Use main <log_dir> <num_of_threads>\n");
    return 0;
  }

  doubletable *statistictable = malloc(sizeof(statistictable));
  if (statistictable == NULL) {
    perror("Problem with malloc for statistictable");
    return 1;
  }
  //Основные таблицы
  statistictable->table_reffer = hashtable_create(START_TABLE_SIZE);
  statistictable->table_url = hashtable_create(START_TABLE_SIZE);
  statistictable->result = NULL;
  //список файлов с полным путём
  filesarr = filelist(argv[1], &filelen);
  if (filesarr == NULL) {
    printf("Directory is empty!\n");
    return 0;
  }

  size_t input_thread_num = atoi(argv[2]);
  if (input_thread_num > filelen) {
    threadnum = filelen;
    printf("We got only %ld files, so we can`t\
 use %ld threads and will use %ld threads!\n",
           filelen, input_thread_num, filelen);
  } else if (input_thread_num < 1) {
    threadnum = 1;
  } else {
    threadnum = input_thread_num;
  }

  pthread_mutex_init(&m, NULL);
  //создаем потоки по числу файлов в любом случае
  pthread_t thread_arr[filelen];
  //инициализируем семафор с начальным значением равным максимуму одновременно
  //работающих потоков
  sem_init(&sem, 0, threadnum);

  for (size_t i = 0; i < filelen; ++i) {
    pthread_create(&thread_arr[i], NULL, parce_file, (void *)statistictable);
  }

  for (size_t i = 0; i < filelen; ++i) {
    pthread_join(thread_arr[i], (void *)statistictable);
  }

  print_statictics(statistictable->table_url, "Top 10 URL with bytes sended:");
  print_statictics(statistictable->table_reffer, "Top 10 reffers with count:");
  hashtable_free(statistictable->table_url);
  hashtable_free(statistictable->table_reffer);
  free(statistictable);
  for (size_t i = 0; i < filelen; ++i) {
    free(filesarr[i]);
  }
  free(filesarr);
  pthread_mutex_destroy(&m);
  sem_destroy(&sem);
}
#include "json.h"
#include <bsd/string.h>
#include <curl/curl.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define __STDC_WANT_LIB_EXT1__ 1
#define HTTP_OK 200
#define JSON_WEATHER_LANG 4
#define JSON_WEATHER_TEMP_C 11
#define JSON_WEATHER_WINDDIR 19
#define JSON_WEATHER_WINDSPEEDKMPH 21

struct MemoryStruct {
  char *memory;
  size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb,
                                  void *userp) {
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if (!ptr) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
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
  char *merged = calloc(sizeof(char), len);
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

void cleanup(CURL *curl_handle, struct MemoryStruct chunk, char* url){
  curl_easy_cleanup(curl_handle);
  free(chunk.memory);
  free(url);
  /* we are done with libcurl, so clean it up */
  curl_global_cleanup();
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Введите ./<имя_программы> <город>\n");
    return 0;
  }
  CURL *curl_handle;
  CURLcode res;

  struct MemoryStruct chunk;

  chunk.memory = malloc(1); /* grown as needed by the realloc above */
  chunk.size = 0;           /* no data at this point */
  long http_code = 0;       // server resp status code

  curl_global_init(CURL_GLOBAL_ALL);

  /* init the curl session */
  curl_handle = curl_easy_init();

  /* specify URL to get */
  char *url = concat(3, "https://wttr.in/", argv[1], "?format=j1&lang=ru");
  curl_easy_setopt(curl_handle, CURLOPT_URL, url);

  /* send all data to this function  */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

  /* we pass our 'chunk' struct to the callback function */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

  /* some servers do not like requests that are made without a user-agent
     field, so we provide one */
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT,
                   "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:109.0) "
                   "Gecko/20100101 Firefox/121.0");

  /* get it! */
  res = curl_easy_perform(curl_handle);
  // get status code
  curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);

  /* check for connection errors */
  if (res != CURLE_OK) {
    printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
  } else {
    if (http_code != HTTP_OK) {
      printf("Город не найден!\n");
    } else {
      json_value *jarr = json_parse(chunk.memory, chunk.size);
      if (jarr == NULL) {
        printf("Unable to parse data\n");
        cleanup(curl_handle, chunk, url);
        return 1;
      }
      json_value *weather_info;
      char *city;
      char *weather;
      char *temp;
      char *windspeedKmph;
      char *winddirDirection;

      weather_info = jarr->u.object.values[0].value->u.array.values[0];
      weather = weather_info->u.object.values[JSON_WEATHER_LANG]
                    .value->u.array.values[0]
                    ->u.object.values[0]
                    .value->u.string.ptr;
      temp =
          weather_info->u.object.values[JSON_WEATHER_TEMP_C].value->u.string.ptr;
      windspeedKmph = weather_info->u.object.values[JSON_WEATHER_WINDSPEEDKMPH]
                          .value->u.string.ptr;
      winddirDirection =
          weather_info->u.object.values[JSON_WEATHER_WINDDIR].value->u.string.ptr;
      city = jarr->u.object.values[1]
                 .value->u.array.values[0]
                 ->u.object.values[0]
                 .value->u.array.values[0]
                 ->u.object.values[0]
                 .value->u.string.ptr;
      printf("Прогноз погоды на текущий день по городу %s:\n"
             "Текстовое описание погоды: %s\n"
             "Направление ветра: %s \n"
             "Скорость ветра: %s км/ч\n"
             "Температура: %s градусов Цельсия\n",
             city, weather, winddirDirection, windspeedKmph, temp);
      json_value_free(jarr);
    }  
  }
  cleanup(curl_handle, chunk, url);
  return 0;
}
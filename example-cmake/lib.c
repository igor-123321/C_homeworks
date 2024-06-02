#include <stdio.h>
#include <curl/curl.h>

int print_data(const char* data)
{
    return printf("%s %s\n", data, curl_version());
}

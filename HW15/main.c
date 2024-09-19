#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sqlite3.h"


int print_result(void * args, int colCount, char **columns, char **colNames){
    if(args){}; //Заглушка что бы не было warning`а -Wunused-but-set-parameter
    for (int i = 0; i < colCount; i++)
    {
        printf("%s = %s\n", colNames[i], columns[i] ? columns[i] : "NULL");
    }
    return EXIT_SUCCESS;
}

int main(int argc, char * argv[]){
    if(argc != 4){
        printf("Use ./main db_name, table_name, column_name\n");
        return(EXIT_FAILURE);
    }
    sqlite3 *db;
    int res;
    const char * db_name = argv[1];
    const char * table_name = argv[2];
    const char * column_name = argv[3];

    FILE *fp;

    if (!(fp = fopen(db_name, "r"))) {
        printf("Database with name %s not exists!\n", db_name);
        exit(EXIT_FAILURE);
        /* Handle error, return, exit, try again etc. */
    }

    if ((res = sqlite3_open(db_name, &db)) != SQLITE_OK){
        perror("sqlite3_open");
        res = sqlite3_close(db);
        exit(EXIT_FAILURE);
    }
    
    char *err_msg = NULL;
    // выполняемый код SQL
    char sql[400];

    snprintf(sql, sizeof(sql), "SELECT AVG(%s) as AVG, \
max(%s) as MAX, min(%s) as MIN, \
(AVG(%s*%s) - AVG(%s)*AVG(%s)) as DEVIATION, \
sum(%s) as SUM from %s;", column_name, column_name, column_name, column_name, column_name, column_name, column_name, column_name, table_name);
    
    res = sqlite3_exec(db, sql, print_result, 0, &err_msg);
    if (res != SQLITE_OK )
    {
        printf("SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return EXIT_FAILURE;
    }
    sqlite3_close(db);
}

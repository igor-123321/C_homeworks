#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint8_t cp1251;
    uint8_t koi8_r;
    uint8_t iso_8859_5;
    uint32_t unicode;
} Convtable;

const char *encoding_names[]={
    "CP-1251", "KOI8-R", "ISO-8859-5"
};

const Convtable ConvTable[] = {
 /* for your task we use only all russian alphabed sorted by unicode*/
    {0xA8, 0xB3, 0xA1, 0x0401}, //Ё
    {0xC0, 0xE1, 0xB0, 0x0410}, //A
    {0xC1, 0xE2, 0xB1, 0x0411}, //Б
    {0xC2, 0xF7, 0xB2, 0x0412}, //В
    {0xC3, 0xE7, 0xB3, 0x0413}, //Г
    {0xC4, 0xE4, 0xB4, 0x0414}, //Д
    {0xC5, 0xE5, 0xB5, 0x0415}, //Е
    {0xC6, 0xF6, 0xB6, 0x0416}, //Ж
    {0xC7, 0xFA, 0xB7, 0x0417}, //З
    {0xC8, 0xE9, 0xB8, 0x0418}, //И
    {0xC9, 0xEA, 0xB9, 0x0419}, //Й
    {0xCA, 0xEB, 0xBA, 0x041A}, //К
    {0xCB, 0xEC, 0xBB, 0x041B}, //Л
    {0xCC, 0xED, 0xBC, 0x041C}, //М
    {0xCD, 0xEE, 0xBD, 0x041D}, //Н
    {0xCE, 0xEF, 0xBE, 0x041E}, //О 
    {0xCF, 0xF0, 0xBF, 0x041F}, //П
    {0xD0, 0xF2, 0xC0, 0x0420}, //Р
    {0xD1, 0xF3, 0xC1, 0x0421}, //С
    {0xD2, 0xF4, 0xC2, 0x0422}, //Т
    {0xD3, 0xF5, 0xC3, 0x0423}, //У
    {0xD4, 0xE6, 0xC4, 0x0424}, //Ф
    {0xD5, 0xE8, 0xC5, 0x0425}, //Х
    {0xD6, 0xE3, 0xC6, 0x0426}, //Ц
    {0xD7, 0xFE, 0xC7, 0x0427}, //Ч
    {0xD8, 0xFB, 0xC8, 0x0428}, //Ш
    {0xD9, 0xFD, 0xC9, 0x0429}, //Щ
    {0xDA, 0xFF, 0xCA, 0x042A}, //Ъ
    {0xDB, 0xF9, 0xCB, 0x042B}, //Ы
    {0xDC, 0xF8, 0xCC, 0x042C}, //Ь
    {0xDD, 0xFC, 0xCD, 0x042D}, //Э
    {0xDE, 0xE0, 0xCE, 0x042E}, //Ю     
    {0xDF, 0xF1, 0xCF, 0x042F}, //Я
    {0xE0, 0xC1, 0xD0, 0x0430}, //а
    {0xE1, 0xC2, 0xD1, 0x0431}, //б
    {0xE2, 0xD7, 0xD2, 0x0432}, //в
    {0xE3, 0xC7, 0xD3, 0x0433}, //г
    {0xE4, 0xC4, 0xD4, 0x0434}, //д
    {0xE5, 0xC5, 0xD5, 0x0435}, //е
    {0xE6, 0xD6, 0xD6, 0x0436}, //ж
    {0xE7, 0xDA, 0xD7, 0x0437}, //з
    {0xE8, 0xC9, 0xD8, 0x0438}, //и
    {0xE9, 0xCA, 0xD9, 0x0439}, //й
    {0xEA, 0xCB, 0xDA, 0x043A}, //к
    {0xEB, 0xCC, 0xDB, 0x043B}, //л
    {0xEC, 0xCD, 0xDC, 0x043C}, //м
    {0xED, 0xCE, 0xDD, 0x043D}, //н
    {0xEE, 0xCF, 0xDE, 0x043E}, //о 
    {0xEF, 0xD0, 0xDF, 0x043F}, //п
    {0xF0, 0xD2, 0xE0, 0x0440}, //р
    {0xF1, 0xD3, 0xE1, 0x0441}, //с
    {0xF2, 0xD4, 0xE2, 0x0442}, //т
    {0xF3, 0xD5, 0xE3, 0x0443}, //у
    {0xF4, 0xC6, 0xE4, 0x0444}, //ф
    {0xF5, 0xC8, 0xE5, 0x0445}, //х
    {0xF6, 0xC3, 0xE6, 0x0446}, //ц
    {0xF7, 0xDE, 0xE7, 0x0447}, //ч
    {0xF8, 0xDB, 0xE8, 0x0448}, //ш
    {0xF9, 0xDD, 0xE9, 0x0449}, //щ
    {0xFA, 0xDF, 0xEA, 0x044A}, //ъ
    {0xFB, 0xD9, 0xEB, 0x044B}, //ы
    {0xFC, 0xD8, 0xEC, 0x044C}, //ь
    {0xFD, 0xDC, 0xED, 0x044D}, //э
    {0xFE, 0xC0, 0xEE, 0x044E}, //ю 
    {0xFF, 0xD1, 0xEF, 0x044F}, //я
    {0xB8, 0xA3, 0xF1, 0x0451}, //ё

};

int uft8_encode(unsigned char *utf8, uint32_t unicode){
    if(unicode <= 0x7F){
        utf8[0] = (unsigned char) unicode >> 2;//(unsigned char) unicode;
        utf8[1] = '\0';
        return 1;
    }
    else if (unicode > 0x7F && unicode <= 0x7FF)
    {
        //uint32_t tmp = (unicode & 0x3F);
        utf8[0] = (unsigned char) ((unicode >> 6  ) | 0xC0);
        utf8[1] = (unsigned char) ((unicode & 0x3F) | 0x80);
        utf8[2] = '\0';
        return 2;
    }
    else if (unicode > 0x7FF && unicode <= 0xFFFF)
    {
        utf8[0] = (unsigned char) ((unicode >> 12 ) | 0xE0);
        utf8[1] = (unsigned char) (((unicode >> 6 ) & 0x3F) | 0x80);
        utf8[2] = (unsigned char) ((unicode & 0x3F) | 0x80);
        utf8[3] = '\0';
        return 3;
    }
    else if (unicode > 0xFFFF && unicode <= 0x10FFFF)
    {
        utf8[0] = (unsigned char) ((unicode >> 18 ) | 0xF0);
        utf8[1] = (unsigned char) (((unicode >> 12) & 0x3F) | 0x80);
        utf8[2] = (unsigned char) (((unicode >> 6 ) & 0x3F) | 0x80);
        utf8[3] = (unsigned char) ((unicode & 0x3F) | 0x80);
        utf8[4] = '\0';
        return 4;
    }
    else
        utf8 = NULL;
        return 0;
    
}

int main(int argc, char *argv[])
{
    /*if(argc != 4) 
    {
        printf("Use main <path_to_file>\n");
        return 0;
    }
    */
    //printf("argc - %d\n", argc);
    //printf("args - %s\n", argv[1]);
    //FILE *zipf = fopen(argv[1], "rb");
    FILE *fromf = fopen(".//text//cp1251.txt", "rb"); 
    if(fromf == NULL){
        perror(".//text//cp1251.txt");
        //perror(argv[1]);
        return 1;
    }
    FILE *tof = fopen(".//text//result_utf-8.txt", "w+b");
    fseek(tof, 0, SEEK_SET);
    if(tof == NULL){
        perror(".//text//result_utf-8.txt");
        //perror(argv[1]);
        return 1;
    }

    char * file_encoding = "CP-1251";
    int j = 0;
    for(j; j<sizeof encoding_names/sizeof encoding_names[0]; j++){
        if(!strcmp(file_encoding,encoding_names[j])){
            break;
        }
    }
    

    unsigned char symbol;
    while(fread(&symbol, 1, 1, fromf) == 1) {
        if(symbol < 128){
                fwrite(&symbol, sizeof symbol, 1, tof);
            } 
        for(int i = 0; i < sizeof ConvTable / sizeof ConvTable[0]; i++)
        {
            
            if(symbol == ConvTable[i].cp1251){
                //printf("We found symbol %d", ConvTable[i].utf_8);
               
                unsigned char utf8[4];
                int num_of_bytes = uft8_encode(utf8,ConvTable[i].unicode);
                fwrite(utf8, num_of_bytes, 1, tof);
                break;
            }
        }
    }
    fclose(tof);
    //fseek(fromf, 0L, SEEK_END);
    //file size in bytes
    //size_t fsize = ftell(zipf);
    //size_t current_pos = 0;
    /*struct character {
        char name;
        char windows_1251;
        char koi8_r;
        char iso_8859_5;
        int unicode;
    };*/
    
    /*
    FILE *tof = fopen(".//text//result_utf-8.txt", "wb");
    char a[4];
    //char * b = &a;
    uft8_encode(a, 8364);
    //int b = 0xFFE445AE;
    //int * a = &b;
    //char * c = "Рр";
    fwrite(a, 10, 1, tof); 
    fclose(tof);
    */
    puts(&symbol);
    fclose(fromf);
    return 0;
}
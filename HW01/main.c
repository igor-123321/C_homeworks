#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#define MAX_FILENAME_LENGTH 255
//Central Directory file header ctructure
typedef struct {
    //https://users.cs.jmu.edu/buchhofp/forensics/formats/pkzip.html
    uint32_t signature;
    uint16_t version_made_by;
    uint16_t version_needed;
    uint16_t flags;
    uint16_t compression;
    uint16_t modif_time;
    uint16_t modif_date;
    uint32_t crc_32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t fname_len;
    uint16_t extra_field_len;
    uint16_t file_comment_len;
    uint16_t disk_number;
    uint16_t internal_attrs;
    uint32_t external_attrs;
    uint32_t localheader_offset;
    
} __attribute__((packed)) CDFH;

//End of Central Directory
typedef struct {
    //https://users.cs.jmu.edu/buchhofp/forensics/formats/pkzip.html
    uint32_t signature;
    uint16_t disk_number;
    uint16_t start_disk_number;
    uint16_t number_central_directory_records;
    uint16_t total_central_directory_records;
    uint32_t central_directory_size;
    uint32_t centralDirectoryOffset;
    uint16_t commentLength;
    
} __attribute__((packed)) EOCD;

int is_le_system(void)
{
  uint16_t x = 0x0001;
  return *((uint8_t *) &x);
}

int main(int argc, char *argv[]) {
    if(argc != 2) 
    {
        printf("Use main.exe <path_to_file>\n");
        return 0;
    }
    //printf("argc - %d\n", argc);
    //printf("args - %s\n", argv[1]);
    FILE *zipf = fopen(argv[1], "rb");
    //FILE *zipf = fopen(".//files//zippng.png", "rb"); //zipjpeg1.jpg
    if(zipf == NULL){
        //perror(".//files//zipjpeg1.jpg");
        perror(argv[1]);
        return 1;
    }
    fseek(zipf, 0L, SEEK_END);
    //file size in bytes
    size_t fsize = ftell(zipf);
    size_t current_pos = 0;
    int found_eocd=0;
    for(size_t i = fsize-sizeof(EOCD); i != 0; i--){
        if(!is_le_system()){
            printf("Program works only for LE systems!");
            return 0;
        }
        const uint32_t eocd_singnature_le = 0x06054b50; //0x504b0506 - BE
        uint32_t singnature = 0;
        fseek(zipf, i, SEEK_SET);
        fread(&singnature, sizeof(singnature), 1, zipf);
        if(singnature == eocd_singnature_le){
            found_eocd = 1;
            printf("Zip archive found!\n");
            //return seek to start bytes of EOCD structure
            fseek(zipf, i, SEEK_SET);
            break;
        }
    }
    if(!found_eocd)
        {   
            printf("Zip archive is not present!\n");
            return 0;
        }
    EOCD eocd;
    fread(&eocd, sizeof(eocd), 1, zipf);
    printf("Total files/dirs - %d\n", eocd.total_central_directory_records);
    fseek(zipf, eocd.centralDirectoryOffset, SEEK_SET);
    //find CDFH records
    int count = 0;
    const uint32_t cdfh_singnature_le = 0x02014b50; //0x504b0102 - BE
    uint32_t singnature = 0;
    for(size_t i = eocd.centralDirectoryOffset; i < fsize-sizeof(EOCD); i++){
        fseek(zipf, i, SEEK_SET);
        fread(&singnature, sizeof(singnature), 1, zipf);
        if(singnature == cdfh_singnature_le){
            count++;
            //return seek to start bytes of CDFH structure
            fseek(zipf, i, SEEK_SET);
            CDFH cdfh;
            fread(&cdfh, sizeof(cdfh), 1, zipf);
            //int pos = ftell(zipf);
            char filename[MAX_FILENAME_LENGTH];
            fread(&filename, cdfh.fname_len, 1, zipf);
            filename[cdfh.fname_len]='\0';
            if(filename[cdfh.fname_len-1] == '/')
                printf("%d. dir name - %s\n", count, filename);
            else
                printf("%d. file name - %s\n", count, filename);
        }
    }
    
    fclose(zipf);
    return 0;
}
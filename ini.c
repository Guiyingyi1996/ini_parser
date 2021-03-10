/*------------------ini.c------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ini.h"

static void FreeSectionEnty(SectionEntyType *enty)
{
    if (enty->section != NULL) 
    {
        free(enty->section);
    }
    if (enty->key != NULL) 
    {
        free(enty->key);
    }
    if (enty->value!= NULL) 
    {
        free(enty->value);
    }
    free(enty);
}

static void FreeInifile(InifileType *file)
{
    SectionEntyType *tmp, *p;
    tmp = file->enty;
    while (tmp != NULL) 
    {  
        p = tmp->next;
        FreeSectionEnty(tmp);
        tmp = p;
    }
}

void ShowIniInfo(InifileType *file) 
{   
    SectionEntyType *enty;

    if (NULL == file) 
    {
        return;
    }
    
    printf("inifile name:%s\n", file->filename);
    enty = file->enty;
    while (enty != NULL) {  
        printf("section:%s\n", enty->section);
        printf("key    :%s\n", enty->key);
        printf("value  :%s\n", enty->value);
        enty = enty->next;
    }
}

static void AddSection2Ini(InifileType *file, SectionEntyType *enty) 
{    
    enty->next = file->enty;
    file->enty = enty;    
}

static int Write2Ini(InifileType *file, SectionEntyType *enty)
{
    FILE *input, *output;
    char buffer[MAX_BUFFER_LEN];
    char *ch, *tmp, *tmp_file;
    int  filename_len, wflags;

    /*Add a new name for tmp as "_tmp"*/

    filename_len = strlen(file->filename);
    tmp_file = (char *)malloc(filename_len + 5);// "_tmp/0" 5 in total 
    if (NULL == tmp_file) {
        printf("no enough memory!\n");
    }
    sprintf(tmp_file, "%s%s", file->filename, "_tmp");

    input = fopen(file->filename, "r");
    if (NULL == input) 
    {
        return -1;
    }
    output = fopen(tmp_file, "w");
    if (NULL == output) 
    {
        return -1;
    }

    while ((ch = fgets(buffer, MAX_BUFFER_LEN, input)) != NULL)
    {  
       if (strstr(buffer, enty->section) != NULL)
       {               
            wflags = 1;                                          
            fprintf(output, "%s", buffer);
            continue;
        }

        if ( strstr(buffer, enty->key) != NULL) 
        {
            tmp = strchr(buffer, '=');
            if (tmp != NULL) 
            {
                tmp++;
                strcpy(tmp, enty->value);
                wflags = 0;
                fprintf(output, "%s\n", buffer);
                continue;
            }
        }
        
        fprintf(output, "%s", buffer); 
    }

    fclose(input);
    fclose(output);

    /* change name */
    (void)remove(file->filename);
    (void)rename(tmp_file, file->filename);
    free(tmp_file);
    
    return 0;
}

int LoadIni(InifileType *file, char *filename)
{
    FILE *input;
    int  buffer_len, section_len; //sect_flag;
    char buffer[MAX_BUFFER_LEN];
    char section[MAX_SECTION_LEN];
    char *ch, *tmp;
    char *begin, *end;
    SectionEntyType *enty;

    if ( NULL == file || NULL == filename) 
    {  
        return -1;
    }
    file->filename = (char *)malloc(strlen(filename));
    if (NULL == file->filename) 
    {   
        return -1;
    }
    strcpy(file->filename, filename);
    input = fopen(filename, "r");
    if (NULL == input) 
    {  
        printf("Error! Fail to open ini file!\n");
        return -1;
    }
    /* read a line from file buffer */
    memset(buffer, 0, MAX_BUFFER_LEN);
    memset(section, 0, MAX_SECTION_LEN);

    while ((ch = fgets(buffer, MAX_BUFFER_LEN, input)) != NULL) 
    {    
        begin = buffer;
        while(' ' == *begin) 
        {    
            begin++;
        }
        if ('\n' == *begin|| ';' == *begin) 
        {   
            continue;
        }
        /*find the postion of '['*/
        tmp = strchr(buffer, '[');  
        if (tmp != NULL) 
        {    
            begin = tmp + 1;
            end = strchr(begin, ']');
            if (NULL == end) 
            {   printf("the inifile is not entire!\n");
                continue;
            }
            section_len = end - begin;
            strncpy(section, begin, section_len);
            continue;
        }
        /* get value */
        if ((tmp = strchr(buffer, '=')) != NULL) 
        {    
            enty = (SectionEntyType *)malloc(sizeof(SectionEntyType));
            if (NULL == enty) 
            {   printf("malloc section enty error!\n");
                fclose(input);
                FreeInifile(file);
                return -1;
            }
            
            enty->section = (char *)malloc(section_len + 1 * sizeof(char));
            if (NULL == enty->section) 
            {    
                printf("malloc section buffer error!\n");
                free(enty);
                fclose(input);
                FreeInifile(file);
                return -1;
            }  
            strncpy(enty->section, section, section_len);
            *(enty->section+section_len)='\0';

            buffer_len = tmp - begin;
            enty->key = (char *)malloc(buffer_len + 1 * sizeof(char));
            if (NULL == enty->key) 
            {   
                printf("malloc key buffer error!\r\n");
                free(enty->section);
                free(enty);
                fclose(input);
                FreeInifile(file);
                return -1;
            }
            strncpy(enty->key, begin, buffer_len);
            *(enty->key + buffer_len)='\0';

            end = tmp++;
            while (*end != '\0' && *end != '\n') 
            {
                end++;
            }
            buffer_len = end - tmp;
            enty->value = (char *)malloc(buffer_len + 1 * sizeof(char));
            if (NULL == enty->value) 
            {   
                printf("malloc value buffer error!\n");
                free(enty->section);
                free(enty->key);
                free(enty);
                fclose(input);
                FreeInifile(file);
                return -1;
            }
            strncpy(enty->value, tmp, buffer_len);
            *(enty->value+buffer_len)='\0';

            AddSection2Ini(file, enty);
        }
    }

    fclose(input);

    return 0;
}

int ReadIni(InifileType *file, const char *section, const char *key, char *stored_value, int vlen)
{
    SectionEntyType *enty, *tmp;

    if (NULL == file||NULL == section||NULL == key||NULL == stored_value) 
    {   
        return -1;
    }
    tmp = file->enty;
    while (tmp != NULL) {
        enty = tmp;
        if ((strcmp(enty->section, section) == 0) && (strcmp(enty->key, key) == 0)) 
         {   
            strncpy(stored_value, enty->value, vlen);
            break;
        }   
        tmp = tmp->next;
    }
    
    return 0;
}

int WriteIni(InifileType *file, const char *section, const char *key, char *value_write)
{
    SectionEntyType *enty, *tmp;
    char *ch;
    int  vlen;

    if (NULL == file||NULL == section||NULL == key||NULL == value_write) 
    {
        return -1;
    }

    tmp = file->enty;
    while (tmp != NULL) {
        enty = tmp;
        if ((strcmp(enty->section, section) == 0) && (strcmp(enty->key, key) == 0)) {
            vlen = strlen(value_write);
            ch = (char *)malloc(vlen+1*sizeof(char));
            if (NULL == ch) {
                printf("malloc value error!\r\n");
                return -1;
            }
            free(enty->value);
            enty->value = ch;
            strncpy(enty->value, value_write, vlen);
            *(enty->value+vlen)='\0';
            Write2Ini(file, enty);
            break;
        }   
        tmp = tmp->next;
    }
    
    return 0;
}

int CloseIni(InifileType *file)
{
    if (NULL == file) {
        return -1;
    }
    
    FreeInifile(file);

    return 0;
}
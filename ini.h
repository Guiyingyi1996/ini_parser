/*------------------ini.h------------------*/


#ifndef _INI_H_
#define _INI_H_

#define MAX_SECTION_LEN  128
#define MAX_KEY_LEN      128
#define MAX_VALUE_LEN    128
#define MAX_BUFFER_LEN   128
#define TRUE             1
#define FALSE            0


typedef struct SectionEnty {
    char *section;
    char *key;
    char *value;
    struct SectionEnty *next;
} SectionEntyType;

typedef struct Inifile {
    char *filename;
    struct SectionEnty *enty;
} InifileType;

/* get information from ini */
extern int LoadIni(InifileType *file, char *filename);

/* get value */
extern int ReadIni(InifileType *file, const char *section, const char *key, char *stored_value, int vlen);

/* set value */
extern int WriteIni(InifileType *file, const char *section, const char *key, char *value_write);

/* show ini */
extern void ShowIniInfo(InifileType *file);

/* close ini*/
extern int CloseIni(InifileType *file);

#endif
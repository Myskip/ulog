#ifndef __ULOG_H__
#define __ULOG_H__

#include <libgen.h>

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

#ifndef u32
#define u32 unsigned long
#endif

#ifndef u16
#define u16 unsigned short
#endif

#ifndef u8
#define u8 unsigned char
#endif

#ifndef ERROR
#define ERROR (-1)
#endif

#ifndef SUCCESS
#define SUCCESS (0)
#endif

typedef enum {
    LEVEL_INFO = 0,
    LEVEL_DEBUG,
    LEVEL_WARNING,
    LEVEL_ERROR,
}ULOG_LEVEL;

typedef struct
{
    char name[32];
    int log_level;
}ULOG_MODULE;

typedef enum{
    ULOG_ERR_SUCCESS = 0,
    ULOG_ERR = -1,
    ULOG_ERR_ALREADY_INIT = -2,
}ULOG_ERR_CODE;

#define ULOG(module, level, fmt, args...) ulog(__FILE__, __LINE__, module, level, fmt, ##args)
#define UPRINT(fmt, args...)    printf("[%s][%d]"fmt"\n", basename(__FILE__), __LINE__, ##args)
#define UNIX_ERROR(func)    UPRINT(func" errno:%d\n", errno)

#define ULOG_MODULE_SET_LEVEL(module, level)    (module->log_level = level)

int ulog_init(int buff_size);
void ulog_deinit();
ULOG_MODULE *ulog_register(char *name);
void ulog_unregister(ULOG_MODULE *module);
int ulog_set_bufsize(int buf_size);
void ulog(char *file, int line, const ULOG_MODULE *module, unsigned int level, const char *fmt, ...);
int ulog_is_inited();
void ulog_show_modules();
#endif
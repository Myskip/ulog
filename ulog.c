#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <semaphore.h>
#include <time.h>
#include <errno.h>

#include "ulog.h"

#define DEFAULT_BUF_SIZE (1024)

static u8 *m_buf;
static u32 m_buf_size = 0;
static sem_t m_sem;
static u8 m_init = FALSE;
static char *level_names[4] = {
    "INFO",
    "DEBUG",
    "WARNING",
    "ERROR"
};

int ulog_init(int buff_size)
{
    if(m_init)
        return ULONG_ERR_ALREADY_INIT;

    m_buf = (u8 *)malloc(buff_size);
    if (NULL == m_buf)
    {
        UPRINT("malloc failed\n");
        return ERROR;
    }
    int ret = sem_init(&m_sem, 0, 0);
    if(ret != 0)
    {
        UNIX_ERROR("sem_init");
        free(m_buf);
        return ERROR;
    }
    
    if(0 != sem_post(&m_sem))
    {
        UNIX_ERROR("sem_post");
        ulog_deinit();
        return ERROR;
    }
    m_buf_size = buff_size;

    return SUCCESS;
}

void ulog_deinit()
{
    if(0 != sem_destroy(&m_sem))
    {
        UNIX_ERROR("sem_destroy");
    }

    if(m_buf)
    {
        free(m_buf);
        m_buf = NULL;
        m_buf_size = 0;
    }
}

ULOG_MODULE *ulog_register(char *name)
{
    if(FALSE == ulog_is_inited())
    {
        if(0 != ulog_init(DEFAULT_BUF_SIZE))
        {
            printf("ulong init error.\n");
            return NULL;
        }
    }

    ULOG_MODULE *module = (ULOG_MODULE *)malloc(sizeof(ULOG_MODULE));
    if(NULL == module)
    {
        UPRINT("malloc failed\n");
        return NULL;
    }

    strncpy(module->name, name, 32);
    module->log_level = LEVEL_WARNING;

    return module;
}

void ulog_unregister(ULOG_MODULE *module)
{
    if(NULL == module)
        return;

    free(module);
}

int ulog_set_bufsize(int buf_size)
{
    if(buf_size == m_buf_size)
    {
        return 0;
    }

    m_buf = realloc(m_buf, buf_size);
    if(NULL == m_buf)
    {
        UPRINT("realloc failed\n");
        return -1;
    }
    m_buf_size = buf_size;
    return 0;
}

void ulog(char *file, int line, const ULOG_MODULE *module, unsigned int level, const char *fmt, ...)
{
    int len = 0;
    if(NULL == module)
        return;

    if(module->log_level > level)
        return;

    if(0 != sem_wait(&m_sem))
    {
        UNIX_ERROR("sem_wait");
        return;
    }

    time_t t = time(NULL);
    struct tm *_tm = localtime(&t);

    va_list ap;
    va_start(ap, fmt);
    len += snprintf(m_buf, m_buf_size, "[%d-%d-%d %d:%d:%d]", _tm->tm_year + 1900, _tm->tm_mon, _tm->tm_mday, _tm->tm_hour, _tm->tm_min, _tm->tm_sec);
    len += snprintf(m_buf + len, m_buf_size - len, "[%s][%d][%s][%s]", basename(file), line, module->name,level_names[level]);
    vsnprintf(m_buf + len, m_buf_size - len, fmt, ap);
    va_end(ap);
    printf("%s\n", m_buf);
    if(0 != sem_post(&m_sem))
    {
        UNIX_ERROR("sem_post");
    }

    return;
}        

int ulog_is_inited()
{
    return m_init;
}
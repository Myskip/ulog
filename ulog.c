#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <semaphore.h>
#include <time.h>
#include <errno.h>
#include <ulist.h>

#include "ulog.h"

#define DEFAULT_BUF_SIZE (1024)

static char *level_names[4] = {
    "INFO",
    "DEBUG",
    "WARNING",
    "ERROR"
};

typedef struct _ulog_manager
{
    u8 *m_buf;
    u32 m_buf_size;
    sem_t m_sem;
    LIST *module_list;
}ULOG_MANAGER;

static ULOG_MANAGER *m_manager = NULL;

static void _ulog_manager_free(ULOG_MANAGER *manager)
{
    if(manager)
    {
        if(manager->m_buf)
            SAFE_FREE(manager->m_buf);

        if(manager->module_list)
            ulist_destroy(manager->module_list);

        sem_destroy(&manager->m_sem);

        SAFE_FREE(manager);
    }
}

static ULOG_MANAGER *_ulog_manager_create(int buff_size)
{
    ULOG_MANAGER *manager = (ULOG_MANAGER *)malloc(sizeof(ULOG_MANAGER));
    if(!manager)
        return NULL;

    memset(manager, 0, sizeof(ULOG_MANAGER));

    manager->m_buf = (u8 *)malloc(buff_size);
    if(!manager->m_buf)
        goto err;
    manager->m_buf_size = buff_size;

    manager->module_list = ulist_new();
    if(!manager->module_list)
        goto err;

    if(0 != sem_init(&manager->m_sem, 0, 0))
        goto err;

    if(0 != sem_post(&manager->m_sem))
        goto err;

    return manager;

err:
    _ulog_manager_free(manager);
    return NULL;
}

static LIST *ulog_get_module_list()
{
    return m_manager->module_list;
}

int ulog_init(int buff_size)
{
    m_manager = _ulog_manager_create(DEFAULT_BUF_SIZE);
    if(!m_manager)
        return ULOG_ERR;

    return SUCCESS;
}

void ulog_deinit()
{
    _ulog_manager_free(m_manager);
    m_manager = NULL;

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

    int ret = ulist_add(m_manager->module_list, module, sizeof(ULOG_MODULE));
    if(ret)
    {
        UPRINT("ulist_add err, ret:%d\n", ret);
        ulog_unregister(module);
        return NULL;
    }

    return module;
}

void ulog_unregister(ULOG_MODULE *module)
{
    if(NULL == module)
        return;
    int ret = ulist_remove(m_manager->module_list, module);
    if(ret)
    {
        UPRINT("ulist_remove [%s] err, ret:%d\n", module->name, ret);
    }
    SAFE_FREE(module);

    if(0 == LIST_SIZE(m_manager->module_list))
    {
        ulog_deinit();
    }
}

int ulog_set_bufsize(int buf_size)
{
    if(buf_size == m_manager->m_buf_size)
    {
        return ULOG_ERR_SUCCESS;
    }

    m_manager->m_buf  = realloc(m_manager->m_buf, buf_size);
    if(NULL == m_manager->m_buf)
    {
        UPRINT("realloc failed\n");
        return ULOG_ERR;
    }
    m_manager->m_buf_size = buf_size;
    return ULOG_ERR_SUCCESS;
}

void ulog(char *file, int line, const ULOG_MODULE *module, unsigned int level, const char *fmt, ...)
{
    int len = 0;
    if(NULL == module)
        return;

    if(module->log_level > level)
        return;

    if(0 != sem_wait(&m_manager->m_sem))
    {
        UNIX_ERROR("sem_wait");
        return;
    }

    time_t t = time(NULL);
    struct tm *_tm = localtime(&t);

    va_list ap;
    va_start(ap, fmt);
    len += snprintf(m_manager->m_buf, m_manager->m_buf_size, "[%d-%d-%d %d:%d:%d]", _tm->tm_year + 1900, _tm->tm_mon, _tm->tm_mday, _tm->tm_hour, _tm->tm_min, _tm->tm_sec);
    len += snprintf(m_manager->m_buf + len, m_manager->m_buf_size - len, "[%s][%d][%s][%s]", basename(file), line, module->name,level_names[level]);
    vsnprintf(m_manager->m_buf + len, m_manager->m_buf_size - len, fmt, ap);
    va_end(ap);
    printf("%s\n", m_manager->m_buf);
    if(0 != sem_post(&m_manager->m_sem))
    {
        UNIX_ERROR("sem_post");
    }

    return;
}        

int ulog_is_inited()
{
    return (NULL != m_manager);
}

void ulog_show_modules()
{
    ULOG_MODULE *module = NULL;
    printf("ulog [%d] modules:\n", LIST_SIZE(m_manager->module_list));
    FOR_EACH_NODE(node, m_manager->module_list)
    {
        module = (ULOG_MODULE *)node->obj;
        printf("module[%s]\n", module->name);
    }
}
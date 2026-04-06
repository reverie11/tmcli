#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>

#define MSG_MAXLEN 64
#define TASKNAME_MAXLEN 64
#define LINE_MAXLEN 64

#define NTASKS_MAX 10

#define EXPORT_FILE "tmcli_export.ics"

#ifndef STATE_DIR
#define STATE_DIR
#endif

typedef struct time{
    int8_t hour;
    int8_t min;
}Time;

typedef struct date{
    int8_t day;
    int8_t month;
    int16_t year;
}Date;

/**
 * @brief typedef struct task
 *
 * NOTES: 
 * day-month-year   (31-12-9999)
 * hour:min         (23:59)
 */
typedef struct task {
    int8_t id;
    int8_t order_id;
    char name[TASKNAME_MAXLEN];
    float duration_h;
    Time start, end;
}Task;

typedef struct task_manager{
    Date task_date;
    Task* task_list[NTASKS_MAX];
    int8_t n_created_tasks;
    int8_t n_active_tasks;
    int8_t initialized;
}TaskManager;

typedef struct tm_state{
    TaskManager tm;
    Task tm_task[NTASKS_MAX];
}TM_state;

extern bool g_verbose;

#endif

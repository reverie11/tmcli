#ifndef TMCLI_H
#define TMCLI_H

#include <stdint.h>
#include <stdbool.h>

#define MSG_MAXLEN 64
#define TASKNAME_MAXLEN 32
#define LINE_MAXLEN 64

#define NTASKS_MAX 10

#ifndef STATE_FILE
#define STATE_FILE 
#endif

typedef struct time{
    int8_t hour;
    int8_t min;
}Time;

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

/**************************** Functions START *********************************/

/**
 * @brief validate_task_time
 * @return 0 if valid, 1 otherwise
 */
int validate_task_time(const Task* task);

/**
 * @brief validate_str_format
 * this function just validate the string, is NOT responsible for the
 * semantic validation of the time.
 *
 * @notes
 * OK: "20:01", "99:99", "20", "2"
 * NOT_OK: "100", "2:", "20:", "20:1", "20:100"
 *
 * @return 0 if valid, 1 otherwise
 */
int validate_str_format(const char* str);

/**
 * @brief calculate_task_duration
 */
float calculate_task_duration(const Task* task);

Time calculate_end_time(const Time start, float duration_h);

/**
 * @brief compare_task
 * helper function for qsort()
 * @notes
 * =0 if priority(a) = priority(b)
 * <0 if priority(a) > priority(b)
 * >0 if priority(a) < priority(b)
 */
int compare_and_reorder_tasks(const void* a, const void* b);
int compare_time(const void* a, const void* b);

/**
 * @brief str_to_time
 */
Time str_to_time(const char* str);

/**
 * @brief str_to_uint
 * @notes
 * str can be max 32 char long.
 * only accepts all_digit_string
 * @return -1 on error
 */
long str_to_uint(const char* str);

/**
 * @brief
 */
int str_is_digit(const char* str);

/**
 * @brief
 */
int ch_is_digit(int ch);

/**************************** Functions END ***********************************/
/************************ TaskManager Methods START ***************************/

/**
 * @brief TM_init
 */
int TM_init(TaskManager* tm);

/**
 * @brief TM_create_task
 * create task dynamically with malloc()
 * @return task_id on success, otherwise -1
 * @todo Add max_task_check
 */
int TM_create_task(TaskManager* tm, const Time start, const Time end, const
        char* name);

/**
 * @brief TM_delete_task
 * delete task with free()
 */
int TM_delete_task(TaskManager* tm, int task_order_id);

/**
 * @brief
 */
int TM_delete_all_tasks(TaskManager* tm);

/**
 * @brief
 * modify task_start WITHOUT sorting the task_list
 */
int TM_modify_task_start(TaskManager* tm, int task_order_id, Time start);
int TM_modify_task_end(TaskManager* tm, int task_order_id, Time end);
int TM_move_task_start(TaskManager* tm, int task_order_id, Time start);

/**
 * @brief
 */
int TM_save_state(TaskManager* tm);

/**
 * @brief
 */
int TM_restore_state(TaskManager* tm);

/**
 * @brief
 */
int TM_sort_tasks(TaskManager* tm);

/**
 * @brief
 */
void TM_print_task(TaskManager*tm, int task_order_id);

/**
 * @brief
 * @notes
 * mode = 0 (compact)
 * mode = 1 (verbose)
 */
void TM_print_all_tasks_highlight(TaskManager* tm, int mode, int highlight_id);
#define TM_print_all_tasks(tm, mode) TM_print_all_tasks_highlight(tm, mode, -1)

/**
 * @brief
 */
void TM_print_self(TaskManager* tm);

/************************ TaskManager Methods END *****************************/
#endif

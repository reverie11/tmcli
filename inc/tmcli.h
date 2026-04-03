#ifndef TMCLI_H
#define TMCLI_H

#include "types.h"

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
int TM_modify_task_name(TaskManager* tm, int task_order_id, const char* name);
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
void TM_print_task(TaskManager* tm, int task_order_id);

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

/**
 * @brief
 */
int TM_export_to_ICS(TaskManager* tm);

/**
 * @brief TM_get_curr_taskid
 * get the order_id of current ongoing task
 */
int TM_get_curr_taskid(TaskManager* tm);

#endif

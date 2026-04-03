#ifndef UTILS_H
#define UTILS_H

#include "types.h"

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
 * @brief compare_and_reorder_tasks
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

#endif

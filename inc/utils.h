#ifndef UTILS_H
#define UTILS_H

#include "types.h"

/******************************** VALIDATION *********************************/
/**
 * @brief validate_task_time
 * @return 0 if valid, 1 otherwise
 */
int validate_task_time(const Task* task);

/**
 * @brief validate_date
 * @return 0 if valid, 1 otherwise
 */
int validate_date(const Date date);

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

/******************************** CALCULATION ********************************/
/**
 * @brief calculate_task_duration
 */
float calculate_task_duration(const Task* task);

Time calculate_end_time(const Time start, float duration_h);

/******************************** COMPARISON *********************************/
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

/******************************** CONVERSION *********************************/
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

/******************************** BOOLEAN ************************************/
/**
 * @brief
 */
int str_is_digit(const char* str);

/**
 * @brief
 */
int ch_is_digit(int ch);

bool is_leap_year(const int year);

/******************************** GET ****************************************/
Date get_date_today(void);

#endif

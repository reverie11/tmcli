#ifndef UTILS_H
#define UTILS_H

#include "types.h"

/******************************** VALIDATION *********************************/
/**
 * @brief validate_task_time.
 * @return 0 if valid, 1 otherwise
 */
int validate_task_time(const Task* task);

/**
 * @brief validate_date
 * @return 0 if valid, 1 otherwise
 */
int validate_date(const Date date);

/**
 * @brief validate_time_format.
 * this function just validate the string, is NOT responsible for the
 * semantic validation of the time.
 *
 * @notes
 * OK: "20:01", "99:99", "20", "2"
 * NOT_OK: "100", "2:", "20:", "20:1", "20:100"
 *
 * @return 0 if valid, 1 otherwise
 */
int validate_time_format(const char* str);

/**
 * @brief validate_time_format.
 * this function just validate the string, is NOT responsible for the
 * semantic validation of the time.
 *
 * @notes
 * OK: "20", "99.99", "99.99.9999", "2"
 * NOT_OK: "100", "2.", "20.", "20.1", "20.100.1000", "20.20.99999"
 *
 * @return 0 if valid, 1 otherwise
 */
int validate_date_format(const char* str);

/******************************** CALCULATION ********************************/
/**
 * @brief calculate_task_duration.
 */
float calculate_task_duration(const Task* task);

/**
 * @brief calculate_task_duration.
 */
Time calculate_end_time(const Time start, float duration_h);

/******************************** COMPARISON *********************************/
/**
 * @brief compare_and_reorder_tasks.
 * helper function for qsort()
 * @notes
 * =0 if priority(a) = priority(b).
 * <0 if priority(a) > priority(b) | a is earlier.
 * >0 if priority(a) < priority(b) | a is later.
 */
int compare_and_reorder_tasks(const void* a, const void* b);
int compare_time(const void* a, const void* b);
int compare_date(const void* a, const void* b);
/******************************** CONVERSION *********************************/
/**
 * @brief str_to_time.
 */
Time str_to_time(const char* str);

/**
 * @brief str_to_time.
 * This function checks the semantic validity of the str with valid date format
 * and coverts it to date if valid. In the case of shorthand format, this
 * function will always assume future date, not one in the past. Only explicit
 * format can be converted into past date.
 * @notes examples:
 * today is 18.12.2026.
 * "17"         -> 17.01.2027.
 * "17.12"      -> 17.12.2027. 
 * "19.11"      -> 19.11.2027.
 * "18.12"      =  18.12.2026.
 * "18.12.2026" =  18.12.2026.
 */
Date str_to_date(const char* str);

/**
 * @brief str_to_uint.
 * @notes
 * str can be max 32 char long.
 * only accepts all_digit_string
 * @return -1 on error
 */
long str_to_uint(const char* str);

/******************************** BOOLEAN ************************************/
/**
 * @brief str_is_digit.
 */
int str_is_digit(const char* str);

/**
 * @brief ch_is_digit.
 */
int ch_is_digit(int ch);

/**
 * @brief is_leap_year.
 */
bool is_leap_year(const int year);

/******************************** GET ****************************************/

/**
 * @brief get_date_today.
 */
Date get_date_today(void);

/**
 * @brief get_time_now.
 */
Time get_time_now(void);

#endif

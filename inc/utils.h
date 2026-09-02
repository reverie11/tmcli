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
 * @brief validate_date_format.
 * this function just validate the string, is NOT responsible for the
 * semantic validation of the date.
 *
 * @notes
 * OK: "20", "99.99", "99.99.9999", "2"
 * NOT_OK: "100", "2.", "20.", "20.1", "20.100.1000", "20.20.99999"
 *
 * @return 0 if valid, 1 otherwise
 */
int validate_date_format(const char* str);

/**
 * @brief validate_timestamp_format.
 * this function combines validate_time_format and validate_date_format with additional timestamp format
 *
 * @notes extended
 * OK: "9/9" "99/99" "99.99/99:99" "99.99.9999/99"
 * NOT_OK: "9.9/99:99" "9.9.9/99:99"
 * NOT_OK: "99.99.9999/9:9" "99.99.9999/9:99" "99.99.9999/99:9"
 *
 * @return 0 if valid, 1 otherwise
 */
int validate_timestamp_format(const char* str);

/******************************** CALCULATION ********************************/
/**
 * @brief calculate_task_duration.
 */
float calculate_task_duration(const Task* task);

/**
 * @brief calculate_task_duration_in_days
 */
int calculate_task_duration_in_days(const Task* task);

/**
 * @brief calculate_end_timestamp
 */
Timestamp calculate_end_timestamp(const Timestamp start, float duration_h);

/******************************** SHIFT **************************************/
/**
 * @brief shift_time_by_minutes. auto-increment hour accordingly to the shifted minutes
 * @notes time.hour will be reset to 0 when it reaches 24. No carry over.
 */
Time shift_time_by_minutes(const Time* t, int mins);

/**
 * @brief shift_date_by_days. auto-increment month and year accordingly to the shifted days
 * @notes date.year will be reset to 0 when it reaches 9999. No carry over.
 */
Date shift_date_by_days(const Date* d, int days);

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

/**
 * @brief compare_time in the same day
 */
int compare_time(const void* a, const void* b);

/**
 * @brief compare_date
 * @notes is not reliable for calculating days between dates. Use calculate_task_duration_in_days instead.
 */
int compare_date(const void* a, const void* b);

/**
 * @brief compare_timestamp
 */
long compare_timestamp(const void* a, const void* b);

/******************************** CONVERSION *********************************/
/**
 * @brief str_to_time. 
 * @notes examples:
 * "1"  = 01:00 
 * "18" = 18:00 
 */
Time str_to_time(const char* str);

/**
 * @brief str_to_date.
 * checks the semantic validity of the str with valid date format
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
 * @brief str_to_timestamp.
 * @notes examples:
 * it is 17:32 and 18.12.2026.
 * "17"         -> 17:00
 * "17/19"      -> 17:00 on 19.12.2026
 * "17/17"      -> 17:00 on 17.01.2027
 */
Timestamp str_to_timestamp(const char* str);

/**
 * @brief str_to_uint.
 * @notes
 * str can be max 32 char long.
 * only accepts all_digit_string
 * @return -1 on error
 */
long str_to_uint(const char* str);

/******************************** REVERSE-CONVERSION **************************/
/**
 * @brief time_to_str
 */
const char *time_to_str(const Time* t);

/**
 * @brief date_to_str
 */
const char *date_to_str(const Date* d);

/**
 * @brief timestamp_to_str
 * dd.mm.yyyy/hh:mm
 */
const char *timestamp_to_str(const Timestamp* ts);

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

/**
 * @brief get_days_in_month.
 */
int get_days_in_month(int month, bool leap);

#endif //UTILS_H

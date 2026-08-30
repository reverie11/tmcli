#include <stdio.h>
#include <string.h>
#include <time.h>

#include "log.h"
#include "utils.h"

int validate_task_time(const Task* task)
{

    bool sh = (task->start.time.hour >= 0 && task->start.time.hour < 24);
    bool sm = (task->start.time.min >= 0 && task->start.time.min < 60);
    bool eh = (task->end.time.hour >= 0 && task->end.time.hour < 24);
    bool em = (task->end.time.min >= 0 && task->end.time.min < 60);
    bool dh = (calculate_task_duration(task)>0);

    if(sh && sm && eh && em && dh){ 
        log_ok("time is valid");
    } else {
        if(!dh){
            log_warn("end.time is INVALID: %02d:%02d", task->end.time.hour, task->end.time.min);
            goto error_handling;
        }
        if(!sh){
            log_warn("start.time.hour is INVALID: %d", task->start.time.hour);
            goto error_handling;
        }
        if(!sm){
            log_warn("start.time.min is INVALID: %d", task->start.time.min);
            goto error_handling;
        }     
        if(!eh){
            log_warn("end.time.hour is INVALID: %d", task->end.time.hour);
            goto error_handling;
        }     
        if(!em){
            log_warn("end.time.min is INVALID: %d", task->end.time.min);
            goto error_handling;
        }
    }
    return 0;

error_handling:
    return 1;
}

int validate_date(const Date date)
{
    bool dy = (date.year >= 0),
         dm = (date.month > 0 && date.month <= 12), 
         dd = (date.day > 0);

    if(!dy){
        log_warn("date.year is INVALID: %d", date.year);
        goto error_handling;
    }

    if(!dm){
        log_warn("date.month is INVALID: %d", date.month);
        goto error_handling;
    }

    if( (date.month <= 7 && date.month % 2 != 0) ||
        (date.month >= 8 && date.month % 2 == 0)) 
        dd = dd && (date.day <= 31);
    else if ( date.month == 2 && is_leap_year(date.year))
        dd = dd && (date.day <= 29);
    else if ( date.month == 2 && !is_leap_year(date.year))
        dd = dd && (date.day <= 28);
    else dd = dd && (date.day <= 30);

    if(!dd){
        log_warn("date.day is INVALID: %d", date.day);
        goto error_handling;
    }

    return 0;

error_handling:
    return 1;
}

float calculate_task_duration(const Task* task)
{
    float duration_h;

    int diff = compare_date(&task->start.date, &task->end.date);
    if(diff == 0) {
        duration_h = task->end.time.hour - task->start.time.hour + (task->end.time.min - task->start.time.min)/60.0;
    } else if (diff < 0){ 
        duration_h = (24-task->start.time.hour)+task->end.time.hour+24*(calculate_task_duration_in_days(task)-1);
    } else {
        duration_h = -1.0f;
    }
    return duration_h;
}

int calculate_task_duration_in_days(const Task* task)
{
    int duration_days = 0;
    bool leap = false;
    
    int diff = compare_date(&task->start.date, &task->end.date);

    if(diff == 0) {
        duration_days = 0;
        return duration_days;
    } else if (diff > 0){ 
        duration_days = -1;
        return duration_days;
    } 


    leap = is_leap_year(task->start.date.year);
    if(task->start.date.year != task->end.date.year){
        for(int i = task->start.date.year; i < task->end.date.year; i++){
            duration_days += (365+is_leap_year(i));
        }

        for(int i = task->start.date.month+1; i <= 12; i++){
            duration_days += get_days_in_month(i, leap);
        }

        leap = is_leap_year(task->end.date.year);
        for(int i = 1; i < task->end.date.month; i++){
            duration_days += get_days_in_month(i, leap);
        }
    } else {
        for(int i = task->start.date.month+1; i < task->end.date.month; i++){
            duration_days += get_days_in_month(i, leap);
        }
    } 

    if(task->start.date.month != task->end.date.month){
        duration_days += get_days_in_month(task->start.date.month, leap) - task->start.date.day;
        duration_days += task->end.date.day;
    } else {
        duration_days += task->end.date.day - task->start.date.day;
    }

    return duration_days;
}

Time calculate_end_time(const Time start, float duration_h){
    Time end;
    float m = (duration_h - (int)duration_h)*60;
    float h = duration_h-m/60;
    end.hour = start.hour+(int)h;
    end.min = start.min+(int)m;
    if(end.min >= 60) {
        end.min-=60;
        end.hour++;
    }
    if(end.hour >= 24) {
        end.hour-=24;
        log_warn("endtime is on the next day.");
    }
    return end;
}

Time shift_time_by_minutes(const Time* t, int mins)
{
    Time result = *t;
    long rmin = result.min + mins%60;
    if(rmin >= 60){
        rmin %= 60;
        result.hour++;
    } else if(rmin < 0){
        result.hour--;
        rmin +=60;
    }
    result.min = rmin;

    long rhour = result.hour + mins/60;
    if(rhour >= 24) rhour %= 24;
    else if(rhour < 0) rhour +=24;

    result.hour = rhour;
    return result;
}

Date shift_date_by_days(const Date* d, int days)
{
    Date result = *d;
    bool leap = is_leap_year(result.year);
    int mdays = get_days_in_month(result.month, leap);
    long rday = result.day + days;
    long ryear = result.year;
    long abs = (rday>0)?rday: rday*(-1);
    do{
        if(rday > 0) {
            rday -= mdays;
            result.month++;
            mdays = get_days_in_month(result.month, leap);
        } else {
            mdays = get_days_in_month(result.month, leap);
            result.month--;
            rday += mdays+1;
        }
        if(result.month > 12){
            result.month-= 12;
            ryear++;
        } else if (result.month <= 0){
            ryear--;
            result.month+= 12;
        }
        leap = is_leap_year(ryear);
        mdays = get_days_in_month(result.month, leap);
        abs = (rday>0)?rday: rday*(-1);
    } while(abs > mdays);
    if(ryear >= 10000) ryear %= 10000;
    else if(ryear < 0 ) ryear += 10000;
    if(rday <= 0) {
        result.month--;
        rday+=mdays+1;
    }
    result.day = rday;
    result.year = ryear;
    return result;
}

int compare_and_reorder_tasks(const void* a, const void* b)
{

    Task** task_a = ( Task**)a;
    Task** task_b = ( Task**)b;
    
    int res = compare_timestamp(&( *task_a )->start, &( *task_b )->start);
    int tmp;
    if(res > 0){
        tmp = (*task_a)->order_id;
        (*task_a)->order_id = (*task_b)->order_id;
        (*task_b)->order_id = tmp;
    }
    return res;
}

int compare_time(const void* a, const void* b)
{
    Time* time_a = (Time*)a;
    Time* time_b = (Time*)b;
    
    return ((time_a->hour - time_b->hour)*60 + (time_a->min - time_b->min));
}

int compare_date(const void* a, const void* b)
{
    Date* date_a = (Date*)a;
    Date* date_b = (Date*)b;

    return ((date_a->year - date_b->year)*365 + (date_a->month - date_b->month)*31 + (date_a->day - date_b->day));
}

long compare_timestamp(const void* a, const void* b)
{
    Timestamp* ts_a = (Timestamp*)a;
    Timestamp* ts_b = (Timestamp*)b;

    return ( (compare_date(&ts_a->date, &ts_b->date)*24*60)+(compare_time(&ts_a->time, &ts_b->time)) );
}

int validate_time_format(const char* str)
{
    int length = strlen(str);

    if(length == 2 || length == 1){
        if(!str_is_digit(str)){
            log_warn("invalid format: %s", str);
            goto error_handling;
        }
    } else if (length == 5) {
        if(str[2] != ':'){
            log_warn("invalid format: %s", str);
            goto error_handling;
        }
        for(int i=0; i<5; i++){
            if(!ch_is_digit(str[i])) {
                log_warn("invalid format: %s", str);
                goto error_handling;
            }
            if(i == 1) i++;
        }
    } else {
        if(length > 5) log_warn("string is too long");
        else log_warn("invalid format: %s", str);
        goto error_handling;
    } 
    
    if(g_verbose){
        log_ok("SUCCESS: str is valid");
    }
    return 0;
error_handling:
    return 1;
}

int validate_date_format(const char* str)
{
    int length = strlen(str);
    if(length == 2 || length == 1){
        if(!str_is_digit(str)){
            log_warn("invalid format: %s", str);
            goto error_handling;
        }
    } else if (length == 5 || length == 10) {
        if(str[2] != '.' || (length == 10 && str[5] != '.')){
            log_warn("invalid format: %s", str);
            goto error_handling;
        }
        for(int i=0; i<length; i++){
            if(i == 2 || i == 5) continue;
            if(!ch_is_digit(str[i])) {
                log_warn("invalid format: %s", str);
                goto error_handling;
            }
        }
    } else {
        if(length > 10) log_warn("string is too long");
        else log_warn("invalid format: %s", str);
        goto error_handling;
    } 
    
    log_ok("SUCCESS: str is valid");
    return 0;
error_handling:
    return 1;

}

Time str_to_time(const char* str){
    Time time;
    int length = strlen(str);
    if(validate_time_format(str) == 0){
        switch(length){
            case 1:
                time.hour = str[0]-'0';
                time.min = 0;
                break;
            case 2:
                time.hour = (str[0]-'0')*10 + (str[1]-'0');
                time.min = 0;
                break;
            case 5:
                time.hour = (str[0]-'0')*10 + (str[1]-'0');
                time.min = (str[3]-'0')*10 + (str[4]-'0');
                break;
        }

        bool h = (time.hour >= 0 && time.hour < 24);
        bool m = (time.min >= 0 && time.min < 60);
        if(!h){
            log_warn("hour is INVALID: %s", str);
            goto error_handling;
        }
        if(!m){
            log_warn("minute is INVALID: %s", str);
            goto error_handling;
        }

    } else{
        log_warn("invalid format: %s", str);
        goto error_handling;
    }

    log_ok("%02d:%02d", time.hour, time.min);
    return time;

error_handling:
    //fallback:
    time.hour = -1;
    time.min = -1;

    return time;
}

Date str_to_date(const char* str){
    Date date = get_date_today(), today = get_date_today();
    int length = strlen(str);

    if(validate_date_format(str) != 0){
        log_warn("date format is INVALID: %s", str);
        goto error_handling;
    }    

    switch(length){
        case 1:
            date.day = str[0]-'0';
            if(compare_date(&date, &today) < 0) date.month++;
            if(date.month > 12) date.year++;
            break;
        case 2:
            date.day = (str[0]-'0')*10 + (str[1]-'0');
            if(compare_date(&date, &today) < 0) date.month++;
            if(date.month > 12) date.year++;
            break;
        case 5:
            date.day = (str[0]-'0')*10 + (str[1]-'0');
            date.month = (str[3]-'0')*10 + (str[4]-'0');
            if(compare_date(&date, &today) < 0) date.year++;
            break;
        case 10:
            date.day = (str[0]-'0')*10 + (str[1]-'0');
            date.month = (str[3]-'0')*10 + (str[4]-'0');
            int tens = 1000;
            date.year = 0;
            for(int i = 0; i < 4; i++){
                date.year += (str[6+i] - '0') * tens;
                tens /= 10;
            } 
    }

    if(validate_date(date) != 0){
        log_warn("date is INVALID: %s", str);
        goto error_handling;
    }

    log_ok("%02d.%02d.%04d", date.day, date.month, date.year);
    return date;

error_handling:
    //fallback:
    date.day = -1;
    date.month = -1;
    date.year = -1;

    return date;
}

long str_to_uint(const char* str)
{
    long result = 0, factor = 1, i = strlen(str)-1;
    while(i >= 0){
        if(!ch_is_digit(str[i])) return -1;
        result += (str[i] - '0')*factor;
        factor *= 10; 
        i--;
    }

    return result;
}

const char *time_to_str(const Time* t)
{
    static char buf[6];
    snprintf(buf, sizeof(buf), "%02d:%02d", t->hour, t->min);
    return buf;
}

const char *date_to_str(const Date* d)
{
    static char buf[11];
    snprintf(buf, sizeof(buf), "%02d.%02d.%04d", d->day, d->month, d->year);
    return buf;
}

const char *timestamp_to_str(const Timestamp* ts)
{
    static char buf[17];
    snprintf(buf, sizeof(buf), "%02d.%02d.%04d/%02d:%02d", 
             ts->date.day, ts->date.month, ts->date.year,
             ts->time.hour, ts->time.min);
    return buf;
}

int str_is_digit(const char* str)
{
    for(unsigned i = 0; i < strlen(str); i++){
        if(str[i] < '0' || str[i] > '9') return 0;
    }
    return 1;
}

int ch_is_digit(int ch){return (ch < '0' || ch > '9')?0:1;}

bool is_leap_year(const int year){ return ( year%4 == 0  &&  year%100 != 0 ) || ( year%400 == 0 )?true:false;}

Date get_date_today(void)
{
   struct tm* now = localtime(&(time_t){time(NULL)}) ;
    return (Date) {.day = now->tm_mday, .month = now->tm_mon+1, .year = now->tm_year+1900};
}

Time get_time_now(void)
{
   struct tm* now = localtime(&(time_t){time(NULL)}) ;
    return (Time) {.hour = now->tm_hour, .min = now->tm_min};
}

int get_days_in_month(int month, bool leap)
{
    switch (month) {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12: 
            return 31;
        case 4: case 6: case 9: case 11:
            return 30;
		case 2: 
            return leap? 29: 28;
        default: 
            return 0;
    }
}

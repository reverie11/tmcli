#include <stdio.h>
#include <string.h>
#include <time.h>

#include "utils.h"
#include "colors.h"

int validate_task_time(const Task* task)
{
    char msg[MSG_MAXLEN];

    bool sh = (task->start.hour >= 0 && task->start.hour < 24);
    bool sm = (task->start.min >= 0 && task->start.min < 60);
    bool eh = (task->end.hour >= 0 && task->end.hour < 24);
    bool em = (task->end.min >= 0 && task->end.min < 60);
    bool dh = (calculate_task_duration(task)>0);

    if(sh && sm && eh && em && dh){ 
        if(g_verbose){
            snprintf(msg, sizeof(msg), "time is valid");
            printf(GREEN "[%-30s] SUCCESS: %s (id=%02d)\n" RESET, __func__,
                    msg, task->id);
        }
    } else {
        if(!dh){
            snprintf(msg, sizeof(msg), "endtime is INVALID: %02d:%02d",
                    task->end.hour, task->end.min);
            goto error_handling;
        }
        if(!sh){
            snprintf(msg, sizeof(msg), "start.hour is INVALID: %d",
                    task->start.hour);
            goto error_handling;
        }
        if(!sm){
            snprintf(msg, sizeof(msg), "start.min is INVALID: %d",
                    task->start.min);
            goto error_handling;
        }     
        if(!eh){
            snprintf(msg, sizeof(msg), "end.hour is INVALID: %d",
                    task->end.hour);
            goto error_handling;
        }     
        if(!em){
            snprintf(msg, sizeof(msg), "end.min is INVALID: %d", task->end.min);
            goto error_handling;
        }
    }
    return 0;

error_handling:
    if(g_verbose) fprintf(stderr, RED "[%-30s] ERROR: %s (id=%02d)\n" RESET, __func__, msg,
            task->id); 
    return 1;
}

int validate_date(const Date date)
{
    char msg[MSG_MAXLEN];
    bool dy = (date.year >= 0),
         dm = (date.month > 0 && date.month <= 12), 
         dd = (date.day > 0);

    if(!dy){
        snprintf(msg, sizeof(msg), "date.year is INVALID: %d", date.year);
        goto error_handling;
    }

    if(!dm){
        snprintf(msg, sizeof(msg), "date.month is INVALID: %d", date.month);
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
        snprintf(msg, sizeof(msg), "date.day is INVALID: %d", date.day);
        goto error_handling;
    }

    return 0;

error_handling:
    if(g_verbose) fprintf(stderr, RED "[%-30s] ERROR: %s \n" RESET, __func__, msg); 
    return 1;
}

float calculate_task_duration(const Task* task)
{
    float duration_h = task->end.hour - task->start.hour + (task->end.min -
            task->start.min)/60.0;
    return duration_h;
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
    if(end.hour >=24) {
        char msg[MSG_MAXLEN];
        end.hour-=24;
        snprintf(msg, sizeof(msg), "endtime is on the next day;,");
        if(g_verbose) printf(YELLOW "[%-30s] WARNING: %s\n" RESET, __func__, msg);
    }
    return end;
}

int compare_and_reorder_tasks(const void* a, const void* b)
{

    Task** task_a = ( Task**)a;
    Task** task_b = ( Task**)b;
    
    int res = (((*task_a)->start.hour - (*task_b)->start.hour)*60 + 
            ((*task_a)->start.min - (*task_b)->start.min));
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
    
    return ((date_a->year - date_b->year)*365 + (date_a->month -
                date_b->month)*31 + (date_a->day - date_b->day));
}

int validate_time_format(const char* str)
{
    int length = strlen(str);
    char msg[MSG_MAXLEN];

    if(length == 2 || length == 1){
        if(!str_is_digit(str)){
            snprintf(msg, sizeof(msg), "invalid format: %s", str);
            goto error_handling;
        }
    } else if (length == 5) {
        if(str[2] != ':'){
            snprintf(msg, sizeof(msg), "invalid format: %s", str);
            goto error_handling;
        }
        for(int i=0; i<5; i++){
            if(!ch_is_digit(str[i])) {
                snprintf(msg, sizeof(msg), "invalid format: %s", str);
                goto error_handling;
            }
            if(i == 1) i++;
        }
    } else {
        if(length > 5) snprintf(msg, sizeof(msg), "string is too long");
        else snprintf(msg, sizeof(msg), "invalid format: %s", str);
        goto error_handling;
    } 
    
    if(g_verbose){
        snprintf(msg, sizeof(msg), "SUCCESS: str is valid");
        printf(GREEN "[%-30s] %s\n" RESET, __func__, msg);
    }
    return 0;
error_handling:
    if(g_verbose) fprintf(stderr, RED "[%-30s] ERROR: %s\n" RESET, __func__, msg); 
    return 1;
}

int validate_date_format(const char* str)
{
    int length = strlen(str);
    char msg[MSG_MAXLEN];
    if(length == 2 || length == 1){
        if(!str_is_digit(str)){
            snprintf(msg, sizeof(msg), "invalid format: %s", str);
            goto error_handling;
        }
    } else if (length == 5 || length == 10) {
        if(str[2] != '.' || (length == 10 && str[5] != '.')){
            snprintf(msg, sizeof(msg), "invalid format: %s", str);
            goto error_handling;
        }
        for(int i=0; i<length; i++){
            if(i == 2 || i == 5) continue;
            if(!ch_is_digit(str[i])) {
                snprintf(msg, sizeof(msg), "invalid format: %s", str);
                goto error_handling;
            }
        }
    } else {
        if(length > 10) snprintf(msg, sizeof(msg), "string is too long");
        else snprintf(msg, sizeof(msg), "invalid format: %s", str);
        goto error_handling;
    } 
    
    if(g_verbose){
        snprintf(msg, sizeof(msg), "SUCCESS: str is valid");
        printf(GREEN "[%-30s] %s\n" RESET, __func__, msg);
    }
    return 0;
error_handling:
    if(g_verbose) fprintf(stderr, RED "[%-30s] ERROR: %s\n" RESET, __func__, msg); 
    return 1;

}

Time str_to_time(const char* str){
    Time time;
    char msg[MSG_MAXLEN];
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
            snprintf(msg, sizeof(msg), "hour is INVALID: %s", str);
            goto error_handling;
        }
        if(!m){
            snprintf(msg, sizeof(msg), "minute is INVALID: %s", str);
            goto error_handling;
        }

    } else{
        snprintf(msg, sizeof(msg), "invalid format: %s", str);
        goto error_handling;
    }

    if(g_verbose){
        snprintf(msg, sizeof(msg), "%02d:%02d", time.hour, time.min);
        printf(GREEN "[%-30s] SUCCESS: %s\n" RESET, __func__, msg);
    }
    return time;

error_handling:
    //fallback:
    time.hour = -1;
    time.min = -1;

    fprintf(stderr, YELLOW "[%-30s] WARNING: %s\n" RESET, __func__, msg); 
    return time;
}

Date str_to_date(const char* str){
    Date date = get_date_today(), today = get_date_today();
    char msg[MSG_MAXLEN];
    int length = strlen(str);

    if(validate_date_format(str) != 0){
        snprintf(msg, sizeof(msg), "date format is INVALID: %s", str);
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
        snprintf(msg, sizeof(msg), "date is INVALID: %s", str);
        goto error_handling;
    }

    if(g_verbose){
        snprintf(msg, sizeof(msg), "%02d.%02d.%04d", date.day, date.month, date.year);
        printf(GREEN "[%-30s] SUCCESS: %s\n" RESET, __func__, msg);
    }
    return date;

error_handling:
    //fallback:
    date.day = -1;
    date.month = -1;
    date.year = -1;

    fprintf(stderr, YELLOW "[%-30s] WARNING: %s\n" RESET, __func__, msg); 
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


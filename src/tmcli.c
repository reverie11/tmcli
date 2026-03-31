#include <libical/ical.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <libical/icalcalendar.h>

#include "colors.h"
#include "tmcli.h"

bool g_verbose = 1;

/**************************** Functions START *********************************/
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
            printf(GREEN "[%-20s] SUCCESS: %s (id=%02d)\n" RESET, __func__,
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
    fprintf(stderr, RED "[%-20s] ERROR: %s (id=%02d)\n" RESET, __func__, msg,
            task->id); 
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
        if(g_verbose) printf(YELLOW "[%-20s] WARNING: %s\n" RESET, __func__, msg);
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

int validate_str_format(const char* str)
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
        printf(GREEN "[%-20s] %s\n" RESET, __func__, msg);
    }
    return 0;
error_handling:
    fprintf(stderr, RED "[%-20s] ERROR: %s\n" RESET, __func__, msg); 
    return 1;
}

Time str_to_time(const char* str){
    Time time;
    char msg[MSG_MAXLEN];
    int length = strlen(str);
    if(validate_str_format(str) == 0){
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
        printf(GREEN "[%-20s] SUCCESS: %s\n" RESET, __func__, msg);
    }
    return time;

error_handling:
    //fallback:
    time.hour = -1;
    time.min = -1;

    fprintf(stderr, RED "[%-20s] WARNING: %s\n" RESET, __func__, msg); 
    return time;
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

/**************************** Functions END ***********************************/
/************************ TaskManager Methods START ***************************/

int TM_init(TaskManager* tm)
{
    char msg[MSG_MAXLEN];

    if(tm->initialized == 1){
        snprintf(msg, sizeof(msg), "Reinitialization detected");
        goto error_handling;
    }

    for(int i = 0; i < NTASKS_MAX; i++){
        tm->task_list[i] = NULL;
    }
    tm->n_active_tasks = 0;
    tm->n_created_tasks = 0;
    tm->initialized = 1;

    if(g_verbose){
        snprintf(msg, sizeof(msg), "TaskManager iniatialized");
        printf(GREEN "[%-20s] SUCCESS: %s\n" RESET, __func__, msg);
    }

    return 0;
error_handling:
    fprintf(stderr, RED "[%-20s] WARNING: %s\n" RESET, __func__, msg); 
    return 1;
}

int TM_create_task(TaskManager* tm, const Time start, const Time end, const
        char* name)
{
    char msg[MSG_MAXLEN];
    if(!tm->initialized){
        TM_init(tm);
    }

    Task* t = malloc(sizeof(Task));
    if(t == NULL){
        snprintf(msg, sizeof(msg), "malloc() failed");
        goto error_handling;
    }

    t->id = tm->n_created_tasks;
    t->order_id = tm->n_active_tasks;
    t->start = start;
    t->end = end;
    t->duration_h = calculate_task_duration(t);
    snprintf(t->name, sizeof(t->name), "%s", name);

    if(validate_task_time(t) != 0){
        free(t);
        snprintf(msg, sizeof(msg), "task time invalid");
        goto error_handling;
    }

    tm->task_list[t->order_id] = t;
    tm->n_created_tasks++;
    tm->n_active_tasks++;

    if(g_verbose){
        snprintf(msg, sizeof(msg), "task created with id=%d", t->id);
        printf(GREEN "[%-20s] SUCCESS: %s\n" RESET, __func__, msg);
    }
    return t->id;

error_handling:
    fprintf(stderr, RED "[%-20s] FAIL: %s\n" RESET, __func__, msg); 
    return -1;
}

void TM_print_task(TaskManager*tm, int task_order_id)
{
    Task* t = tm->task_list[task_order_id];
    if( t != NULL){
        printf("## Task %2d\n", t->id);
        printf("%-12s: %s\n", "name", t->name);
        printf("%-12s: %02d:%02d\n", "start", t->start.hour, t->start.min);
        printf("%-12s: %02d:%02d\n", "end", t->end.hour, t->end.min);
        float m = (t->duration_h - (int)t->duration_h)*60;
        float h = t->duration_h-m/60;
        printf("%-12s: %.0f hour(s) %.0f min(s)\n", "duration", h, m);
    }
    printf("\n");

}

void TM_print_all_tasks_highlight(TaskManager* tm, int mode, int highlight_id)
{
    char buf[LINE_MAXLEN];
    Task* t;

    switch(mode){
        default:
        case 0:
            goto mode_0;
            break;
        case 1:
            goto mode_1;
            break;
    }

mode_0:
    for(int i=0; i < tm->n_active_tasks; i++)
    {
        t = tm->task_list[i];
        if(t != NULL){
            snprintf(buf, sizeof(buf), 
                    "%02d:%02d - %02d:%02d    %s", 
                    t->start.hour, t->start.min,
                    t->end.hour, t->end.min,
                    t->name);
            if(highlight_id == -1 || t->id != highlight_id) printf(BLUE);
            else if(t->id == highlight_id) printf(PURPLE);
            printf("%-*s [%d]\n", (int)sizeof(buf), buf, t->order_id);
            printf(RESET);
        }
    }
    printf("\n");
    return;

mode_1:
    for(int i=0; i < tm->n_created_tasks; i++)
    {
        t = tm->task_list[i];
        if(t != NULL){
            printf(BLUE "## Task %2d\n" RESET, t->order_id);
            printf("%-12s: %s\n", "name", t->name);
            printf("%-12s: %02d:%02d\n", "start", t->start.hour, t->start.min);
            printf("%-12s: %02d:%02d\n", "end", t->end.hour, t->end.min);
            float m = (t->duration_h - (int)t->duration_h)*60;
            float h = t->duration_h-m/60;
            printf("%-12s: %.0f hour(s) %.0f min(s)\n\n", "duration", h, m);
        }
    }
    return;
}

void TM_print_self(TaskManager* tm)
{
    printf("# Task Manager\n");
    printf("n_active_tasks = %d\n", tm->n_active_tasks);
    printf("n_created_tasks = %d\n\n", tm->n_created_tasks);
}

int TM_delete_task(TaskManager* tm, int task_order_id)
{
    char msg[MSG_MAXLEN];

    if(tm->task_list[task_order_id] == NULL){
        snprintf(msg, sizeof(msg), "[task %02d] task doesnt exist", task_order_id);
        goto error_handling;
    } 

    int task_id = tm->task_list[task_order_id]->id;
    free(tm->task_list[task_order_id]);
    tm->task_list[task_order_id] = NULL;
    tm->n_active_tasks--;

    if(g_verbose){
        snprintf(msg, sizeof(msg), "[task %02d] task deleted (id=%d)", task_order_id, task_id);
        printf(GREEN "[%-20s] SUCCESS: %s\n" RESET, __func__, msg);
    }
    return 0;

error_handling:
    fprintf(stderr, RED "[%-20s] FAIL: %s\n" RESET, __func__, msg); 
    return 1;
}

int TM_delete_all_tasks(TaskManager* tm)
{
    char msg[MSG_MAXLEN];
    bool status_ok = true;
    int failed_list[NTASKS_MAX], j = 0;
    const int n_active_tasks = tm->n_active_tasks;

    for(int i=0; i < n_active_tasks; i++)
    {
        if(tm->task_list[i] != NULL){
            if(TM_delete_task(tm, i) != 0){
                status_ok = false;
                failed_list[j++] = i;
            }
        }
    }
    if(!status_ok){
        snprintf(msg, sizeof(msg), "for following task with id: ");
        for(int i = 0; i < j; i++){
            int offset = strlen(msg);
            snprintf(msg+offset, sizeof(msg)-offset+i, "%d ", failed_list[i]);
        }
        goto error_handling;
    }

    return 0;
error_handling:
    fprintf(stderr, RED "[%-20s] FAIL: %s\n" RESET, __func__, msg); 
    return 1;
}

int TM_modify_task_start(TaskManager* tm, int task_order_id, Time start)
{
    char msg[MSG_MAXLEN];
    Task* task = tm->task_list[task_order_id];
    Time fallback = task->start;
    if(task == NULL) {
        snprintf(msg, sizeof(msg), "[task %0d] task doesnt exist", 
                task_order_id);
        goto error_handling;
    }

    task->start = start;
    if(validate_task_time(task) != 0) {
        task->start = fallback;
        snprintf(msg, sizeof(msg), "[task %0d] invalid starttime", 
                task_order_id);
        goto error_handling;
    }    
    task->duration_h = calculate_task_duration(task);

    if(g_verbose) {
        snprintf(msg, sizeof(msg), "[task %02d] task modified\n",
                task_order_id);
        printf(GREEN "[%-20s] SUCCESS: %s\n" RESET, __func__, msg);
    }

    return 0;
    
error_handling:
    fprintf(stderr, RED "[%-20s] FAIL: %s\n" RESET, __func__, msg); 
    return 1;
}

int TM_modify_task_end(TaskManager* tm, int task_order_id, Time end)
{
    char msg[MSG_MAXLEN];
    Task* task = tm->task_list[task_order_id];
    Time fallback = task->end;
    if(task == NULL) {
        snprintf(msg, sizeof(msg), "[task %0d] task doesnt exist", 
                task_order_id);
        goto error_handling;
    }

    task->end = end;
    if(validate_task_time(task) != 0) {
        task->end = fallback;
        snprintf(msg, sizeof(msg), "[task %0d] invalid endtime", 
                task_order_id);
        goto error_handling;
    }    
    task->duration_h = calculate_task_duration(task);

    if(g_verbose) {
        snprintf(msg, sizeof(msg), "[task %02d] task modified\n",
                task_order_id);
        printf(GREEN "[%-20s] SUCCESS: %s\n" RESET, __func__, msg);
    }

    return 0;
    
error_handling:
    fprintf(stderr, RED "[%-20s] FAIL: %s\n" RESET, __func__, msg); 
    return 1;
}

int TM_move_task_start(TaskManager* tm, int task_order_id, Time start)
{
    char msg[MSG_MAXLEN];
    Task* task = tm->task_list[task_order_id];

    if(task == NULL) {
        snprintf(msg, sizeof(msg), "[task %0d] task doesnt exist", 
                task_order_id);
        goto error_handling;
    }

    Time end = calculate_end_time(start, task->duration_h);
    int priority = compare_time(&task->start, &start);

    if(priority == 0) return 0;
    else if(priority>0){ // new start time is earlier
        TM_modify_task_start(tm, task_order_id, start);
        TM_modify_task_end(tm, task_order_id, end);
    } else if(priority<0){ // new start time is later
        TM_modify_task_end(tm, task_order_id, end);
        TM_modify_task_start(tm, task_order_id, start);
    }
    return 0;

error_handling:
    fprintf(stderr, RED "[%-20s] FAIL: %s\n" RESET, __func__, msg); 
    return 1;
}

int TM_save_state(TaskManager* tm)
{
    TM_state tms;
    char msg[MSG_MAXLEN];

    FILE *fp = fopen(STATE_FILE, "w");
    if(!fp) {
        snprintf(msg, sizeof(msg), "fopen: %s", strerror(errno));
        goto error_handling;
    }

    memcpy(&tms.tm, tm, sizeof(TaskManager));
    tms.tm.n_active_tasks = 0;
    int j = 0;
    for(int i = 0; i < tm->n_created_tasks; i++){
        if(tm->task_list[i] != NULL){
            memcpy(&tms.tm_task[j], tm->task_list[i], sizeof(Task));
            tms.tm_task[j].order_id = j;
            tms.tm.n_active_tasks = ++j;
        }
    }
    assert(tms.tm.n_active_tasks == tm->n_active_tasks);

    size_t bytes = fwrite(&tms, 1, sizeof(TM_state), fp);
    if(bytes < sizeof(TM_state)){
        snprintf(msg, sizeof(msg), "error on fwrite: %ld Bytes written"
                "(expect=%ld)\n", bytes, sizeof(TM_state));
        goto error_handling;
    }

    fclose(fp);
    fp = NULL;

    if(g_verbose) {
        snprintf(msg, sizeof(msg), "current state saved\n");
        printf(GREEN "[%-20s] SUCCESS: %s\n" RESET, __func__, msg);
    }

    return 0;

error_handling:
    if(fp != NULL) {
        fclose(fp);
    }
    fprintf(stderr, RED "[%-20s] FAIL: %s\n" RESET, __func__, msg); 
    return 1;
}

int TM_restore_state(TaskManager *tm)
{
    TM_state tms;
    char msg[MSG_MAXLEN];

    FILE *fp = fopen(STATE_FILE, "r");
    if(!fp) {
        snprintf(msg, sizeof(msg), "fopen: %s", strerror(errno));
        if(g_verbose) {
            snprintf(msg, sizeof(msg), "no state to restore\n");
            printf(YELLOW "[%-20s] WARNING: %s\n" RESET, __func__, msg);
        }
        return 1;
    }

    size_t bytes = fread(&tms, 1, sizeof(TM_state), fp);
    if(bytes < sizeof(TM_state)){
        snprintf(msg, sizeof(msg), "error on fwrite: %ld Bytes written"
                "(expect=%ld)\n", bytes, sizeof(TM_state));
        goto error_handling;
    }
    memcpy(tm, &tms.tm, sizeof(TaskManager));

    Task* t;
    for(int i = 0; i < tms.tm.n_active_tasks; i++){
        t = malloc(sizeof(Task));    

        int order_id = tms.tm_task[i].order_id;

        if(t == NULL){
            snprintf(msg, sizeof(msg), "malloc() failed");
            goto error_handling;
        } 

        t->id = tms.tm_task[i].id;
        t->order_id = tms.tm_task[i].order_id;
        t->start = tms.tm_task[i].start;
        t->end = tms.tm_task[i].end;
        t->duration_h = tms.tm_task[i].duration_h;
        strcpy(t->name, tms.tm_task[i].name);

        tm->task_list[order_id] = t;
    }

    if(g_verbose) {
        snprintf(msg, sizeof(msg), "last state restored");
        printf(GREEN "[%-20s] SUCCESS: %s\n" RESET, __func__, msg);
    }

    return 0;

error_handling:
    if(fp != NULL) {
        fclose(fp);
    }
    fprintf(stderr, RED "[%-20s] FAIL: %s\n" RESET, __func__, msg); 
    return 1;

}

int TM_sort_tasks(TaskManager* tm)
{
    qsort(&tm->task_list[0], tm->n_active_tasks, sizeof(Task*),
            compare_and_reorder_tasks);
    return 0;
}

void TM_export_to_ICS(TaskManager* tm)
{
    icaltimetype today = icaltime_today();

    icalcomponent* c = icalcomponent_new(ICAL_VCALENDAR_COMPONENT);

    icalcomponent_add_property(c, icalproperty_new_version("2.0"));
    icalcomponent_add_property(c, icalproperty_new_prodid("-//reverie//tmcli//EN"));

    for(int i = 0; i < tm->n_active_tasks; i++)
    {
        Task* task = tm->task_list[i];
        icalcomponent* event = icalcomponent_new(ICAL_VEVENT_COMPONENT);
        icalcomponent_add_property(event, icalproperty_new_summary(task->name));
        icaltimetype event_start = today;
        event_start.is_date = 0;
        event_start.hour = task->start.hour;
        event_start.minute = task->start.min;

        icaltimetype event_end = today;
        event_end.is_date = 0;
        event_end.hour = task->end.hour;
        event_end.minute = task->end.min;

        icalcomponent_add_property(event, icalproperty_new_dtstart(event_start));
        icalcomponent_add_property(event, icalproperty_new_dtend(event_end));

        icalcomponent_add_component(c, event);
    }

    char *ical_string = icalcomponent_as_ical_string(c);

    FILE *file = fopen("tmcli_export.ics", "w");
    fprintf(file, "%s", ical_string);
    fclose(file);
    
    icalcomponent_free(c);
}

/************************ TaskManager Methods END *****************************/


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include <libical/ical.h>
#include <libical/icalcalendar.h>

#include "colors.h"
#include "tmcli.h"
#include "utils.h"

bool g_verbose = 1;

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
    tm->task_date = get_date_today();
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
        printf("%-12s: %02d.%02d.%04d\n", "date", tm->task_date.day,
                tm->task_date.month, tm->task_date.year);
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
    char buf[LINE_MAXLEN-4];
    char title[32];
    Task* t;

    switch(mode){
        default:
        case 0: // compact mode
            goto mode_0; 
            break;
        case 1: // verbose mode 
            goto mode_1;
            break;
    }

mode_0:
    // header
    snprintf(title, sizeof(title), " %02d.%02d.%04d ", 
            tm->task_date.day, tm->task_date.month, tm->task_date.year);
    printf(CYAN);
    for(int i = 0; i < LINE_MAXLEN/2 - (int)strlen(title)/2; i++) printf("-");
    printf("%s", title);
    for(int i = 0; i < LINE_MAXLEN/2 - (int)strlen(title)/2; i++) printf("-");
    printf("\n" RESET);

    // body
    for(int i=0; i < tm->n_active_tasks; i++)
    {
        t = tm->task_list[i];
        if(t != NULL){
            if(strlen(t->name)<=32) snprintf(buf, sizeof(buf), 
                    "%02d:%02d - %02d:%02d    %.32s", 
                    t->start.hour, t->start.min,
                    t->end.hour, t->end.min,
                    t->name);
            else snprintf(buf, sizeof(buf), 
                    "%02d:%02d - %02d:%02d    %.32s...", 
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
        TM_print_task(tm, i);
    }
    return;
}

void TM_print_self(TaskManager* tm)
{
    printf("# Task Manager\n");
    printf("task_date = %02d.%02d.%04d\n", tm->task_date.day,
            tm->task_date.month, tm->task_date.year);
    printf("n_active_tasks = %d\n", tm->n_active_tasks);
    printf("n_created_tasks = %d\n\n", tm->n_created_tasks);
    printf("initialized = %d\n\n", tm->initialized);
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

int TM_modify_task_name(TaskManager* tm, int task_order_id, const char* name)
{
    char msg[MSG_MAXLEN];
    Task* task = tm->task_list[task_order_id];

    if(task == NULL) {
        snprintf(msg, sizeof(msg), "[task %0d] task doesnt exist", 
                task_order_id);
        goto error_handling;
    }

    snprintf(task->name, TASKNAME_MAXLEN, "%s", name);

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

int TM_export_to_ICS(TaskManager* tm)
{
    char msg[MSG_MAXLEN];
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

    FILE *fp = fopen(EXPORT_FILE, "w");

    if(!fp) {
        snprintf(msg, sizeof(msg), "fopen: %s", strerror(errno));
        goto error_handling;
    }
    fprintf(fp, "%s", ical_string);
    fclose(fp);
    
    icalcomponent_free(c);

    if(g_verbose){
        snprintf(msg, sizeof(msg), "icalcomponent freed");
        printf(GREEN "[%-20s] SUCCESS: %s\n" RESET, __func__, msg);
    }
    return 0;

error_handling:
    if(fp != NULL) {
        fclose(fp);
    }
    fprintf(stderr, RED "[%-20s] FAIL: %s\n" RESET, __func__, msg); 
    return -1;
}

int TM_get_curr_taskid(TaskManager* tm)
{
   struct tm* curr_time = localtime(&(time_t){time(NULL)}) ;
   Time now = { .hour = curr_time->tm_hour, .min = curr_time->tm_min};

   for(int i = 0; i < tm->n_active_tasks; i++){
        Task* task = tm->task_list[i];
        int after_start = compare_time(&now, &task->start); 
        int before_end = compare_time(&now, &task->end); 
        if(after_start >= 0 && before_end < 0) return task->id;
   }
   return -1;
}



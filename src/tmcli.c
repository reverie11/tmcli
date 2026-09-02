#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>

#include <libical/ical.h>
#include <libical/icalcalendar.h>

#include "colors.h"
#include "tmcli.h"
#include "types.h"
#include "utils.h"
#include "log.h"

bool g_verbose = 1;

int TM_init(TaskManager* tm)
{
    
    if(!tm) {
        log_error("Initializing a nullpointer");
        goto error_handling;
    }

    if(tm->initialized){
        log_warn("Reinitialization detected");
    }

    for(int i = 0; i < NTASKS_MAX; i++){
        tm->task_list[i] = NULL;
    }
    tm->task_date = get_date_today();
    tm->n_active_tasks = 0;
    tm->n_created_tasks = 0;
    tm->initialized = 1;

    log_ok("TaskManager initialized");

    return 0;

error_handling:
    return 1;
}

int TM_create_task(TaskManager* tm, const Timestamp start, const Timestamp end, const char* name)
{
    if(!tm->initialized){
        TM_init(tm);
    }

    Task* t = malloc(sizeof(Task));
    if(t == NULL){
        log_error("malloc() failed");
        goto error_handling;
    }

    t->id = tm->n_created_tasks;
    t->order_id = tm->n_active_tasks;
    t->start = start;
    t->end = end;
    
    int diff = compare_time(&end.time, &start.time);
    if(diff <= 0) t->end.date = shift_date_by_days(&start.date, 1); 

    t->duration_h = calculate_task_duration(t);
    snprintf(t->name, sizeof(t->name), "%s", name);

    if(validate_task_time(t) != 0){
        free(t);
        log_error( "task time invalid");
        goto error_handling;
    }

    tm->task_list[t->order_id] = t;
    tm->n_created_tasks++;
    tm->n_active_tasks++;

    log_ok("task created with id=%d", t->id);
    return t->id;

error_handling:
    return -1;
}

void TM_print_task(TaskManager*tm, int task_order_id)
{
    Task* t = tm->task_list[task_order_id];
    if( t != NULL){
        printf("## Task %2d\n", t->id);
        printf("%-12s: %s\n", "name", t->name);
        printf("%-12s: %02d.%02d.%04d\n", "start.date", t->start.date.day, t->start.date.month, t->start.date.year);
        printf("%-12s: %02d:%02d\n", "start.time", t->start.time.hour, t->start.time.min);
        printf("%-12s: %02d.%02d.%04d\n", "end.date", t->end.date.day, t->end.date.month, t->end.date.year);
        printf("%-12s: %02d:%02d\n", "end.time", t->end.time.hour, t->end.time.min);
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
    const char* label_ytd = "  (YTD)";
    const char* label_tmr = "  (TMR)";
    const char* label_dmy = "%02d.%02d.%04d";
    const char* label_nul = "          ";
    char end_label[11]    = "          ", 
         start_label[11]  = "          "; 
    Date today = get_date_today();
    Task* t;
    int diff_date = compare_date(&tm->task_date, &today);
    int diff_start = 0, diff_end =  0;

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
    if(diff_date == 0){
        snprintf(title, sizeof(title), " %02d.%02d.%04d (Today) ", 
                 tm->task_date.day, tm->task_date.month, tm->task_date.year);
    } else if(diff_date == -1){
        snprintf(title, sizeof(title), " %02d.%02d.%04d (Yesterday) ", 
                 tm->task_date.day, tm->task_date.month, tm->task_date.year);
    } else if(diff_date == 1){
        snprintf(title, sizeof(title), " %02d.%02d.%04d (Tomorrow) ", 
                 tm->task_date.day, tm->task_date.month, tm->task_date.year);
    } else {
        snprintf(title, sizeof(title), " %02d.%02d.%04d ", 
                 tm->task_date.day, tm->task_date.month, tm->task_date.year);
    }
    printf(COLOR_CYAN);
    for(int i = 0; i < LINE_MAXLEN/2 - (int)strlen(title)/2; i++) printf("-");
    printf("%s", title);
    for(int i = 0; i < LINE_MAXLEN/2 - (int)strlen(title)/2; i++) printf("-");
    printf("\n" COLOR_RESET);

    // body
    for(int i=0; i < tm->n_active_tasks; i++)
    {
        t = tm->task_list[i];
        if(t != NULL){
            diff_start = compare_date(&t->start.date, &tm->task_date); 
            diff_end = compare_date(&t->end.date, &tm->task_date); 

            if(diff_start != 0 && diff_end != 0) continue;

            if (diff_start == -1){
                snprintf(start_label, sizeof(start_label), "%s", label_ytd);
            } else if (diff_start == 1){
                snprintf(start_label, sizeof(start_label), "%s", label_tmr);
            } else if (diff_start != 0){
                const char* label_dmy = "%02d.%02d.%04d";
                snprintf(start_label, sizeof(start_label), label_dmy,
                         t->start.date.day, t->start.date.month, t->start.date.year);
            } 

            if (diff_end == -1){
                snprintf(end_label, sizeof(end_label), "%s", label_ytd);
            } else if (diff_end == 1){
                snprintf(end_label, sizeof(end_label), "%s", label_tmr);
            } else if (diff_end != 0){
                snprintf(end_label, sizeof(end_label), label_dmy, t->end.date.day,
                         t->end.date.month, t->end.date.year);
            } 
            if(strlen(t->name)<=32){
                snprintf(buf, sizeof(buf), "    %02d:%02d   -   %02d:%02d     %.25s", 
                         t->start.time.hour, t->start.time.min,
                         t->end.time.hour, t->end.time.min,
                         t->name);
            } else {
                snprintf(buf, sizeof(buf), "    %02d:%02d   -   %02d:%02d     %.25s...", 
                         t->start.time.hour, t->start.time.min,
                         t->end.time.hour, t->end.time.min,
                         t->name);
            }
            if(highlight_id == -1 || t->id != highlight_id) printf(COLOR_BLUE);
            else if(t->id == highlight_id) printf(COLOR_PURPLE);
            printf("%-*s [%d]   \n", (int)sizeof(buf)-3, buf, t->order_id);
            if(diff_start || diff_end) printf("  %-10s  %-10s\n", start_label, end_label);
            printf(COLOR_RESET);
            diff_start = 0; diff_end = 0;
            snprintf(start_label, 10, "%s", label_nul);
            snprintf(end_label, 10, "%s", label_nul);
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
    printf("task_date = %02d.%02d.%04d\n", tm->task_date.day, tm->task_date.month, tm->task_date.year);
    printf("n_active_tasks = %d\n", tm->n_active_tasks);
    printf("n_created_tasks = %d\n\n", tm->n_created_tasks);
    printf("initialized = %d\n\n", tm->initialized);
}

int TM_delete_task(TaskManager* tm, int task_order_id)
{
    if(tm->task_list[task_order_id] == NULL){
        log_error("[task %02d] task doesnt exist", task_order_id);
        goto error_handling;
    } 

    int task_id = tm->task_list[task_order_id]->id;
    free(tm->task_list[task_order_id]);
    tm->task_list[task_order_id] = NULL;
    tm->n_active_tasks--;

    log_ok("[task %02d] task deleted (id=%d)", task_order_id, task_id);
    return 0;

error_handling:
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
        log_error("%s", msg);
        return 1;
    }

    return 0;
}

int TM_modify_task_start(TaskManager* tm, int task_order_id, Timestamp start)
{
    Task* task = tm->task_list[task_order_id];
    if(task == NULL) {
        log_error("[task %0d] task doesnt exist", task_order_id);
        return 1;
    }

    int status = 0;
    int diff = compare_time(&task->start.time, &task->end.time);
    if(diff <= 0){
        status |= TM_modify_task_start_date(tm, task_order_id, start.date);
        status |= TM_modify_task_start_time(tm, task_order_id, start.time);
    }else {
        status |= TM_modify_task_start_time(tm, task_order_id, start.time);
        status |= TM_modify_task_start_date(tm, task_order_id, start.date);
    }

    if(status != 0) log_error("fail");
    return status;
}

int TM_modify_task_end(TaskManager* tm, int task_order_id, Timestamp end)
{
    Task* task = tm->task_list[task_order_id];
    if(task == NULL) {
        log_error("[task %0d] task doesnt exist", task_order_id);
        return 1;
    }

    int status = 0;

    int diff = compare_time(&end.time, &task->start.time);
    if(diff <= 0){
        status |= TM_modify_task_end_date(tm, task_order_id, end.date);
        status |= TM_modify_task_end_time(tm, task_order_id, end.time);
    }else {
        status |= TM_modify_task_end_time(tm, task_order_id, end.time);
        status |= TM_modify_task_end_date(tm, task_order_id, end.date);
    }

    if(status != 0) log_error("fail");
    return status;
}

int TM_modify_task_start_time(TaskManager* tm, int task_order_id, Time start)
{
    Task* task = tm->task_list[task_order_id];
    Time fallback = task->start.time;
    if(task == NULL) {
        log_error("[task %0d] task doesnt exist", task_order_id);
        return 1;
    }

    task->start.time = start;
    if(validate_task_time(task) != 0) {
        task->start.time= fallback;
        log_error("[task %0d] invalid start.time", task_order_id);
        return 1;
    }    
    task->duration_h = calculate_task_duration(task);

    log_ok("[task %02d] task modified\n", task_order_id);
    return 0;
}

int TM_modify_task_end_time(TaskManager* tm, int task_order_id, Time end)
{
    Task* task = tm->task_list[task_order_id];
    Time fallback = task->end.time;
    if(task == NULL) {
        log_error("[task %0d] task doesnt exist", task_order_id);
        return  1;
    }

    task->end.time = end;
    if(validate_task_time(task) != 0) {
        task->end.time = fallback;
        log_error("[task %0d] invalid end.time", task_order_id);
        return 1;
    }
    task->duration_h = calculate_task_duration(task);

    log_ok("[task %02d] task modified\n", task_order_id);
    return 0;
}

int TM_modify_task_start_date(TaskManager* tm, int task_order_id, Date start)
{
    Task* task = tm->task_list[task_order_id];
    Date fallback = task->start.date;
    if(task == NULL) {
        log_error("[task %0d] task doesnt exist", task_order_id);
        return 1;
    }

    task->start.date = start;
    if(validate_task_time(task) != 0) {
        task->start.date= fallback;
        log_error("[task %0d] invalid start.date", task_order_id);
        return 1;
    }    
    task->duration_h = calculate_task_duration(task);

    log_ok("[task %02d] task modified\n", task_order_id);
    return 0;
}

int TM_modify_task_end_date(TaskManager* tm, int task_order_id, Date end)
{
    Task* task = tm->task_list[task_order_id];
    Date fallback = task->end.date;
    if(task == NULL) {
        log_error("[task %0d] task doesnt exist", task_order_id);
        return 1;
    }

    task->end.date = end;
    if(validate_task_time(task) != 0) {
        task->end.date = fallback;
        log_error("[task %0d] invalid end.date", task_order_id);
        return 1;
    }
    task->duration_h = calculate_task_duration(task);

    log_ok("[task %02d] task modified\n", task_order_id);

    return 0;
}

int TM_modify_task_name(TaskManager* tm, int task_order_id, const char* name)
{
    Task* task = tm->task_list[task_order_id];

    if(task == NULL) {
        log_error("[task %0d] task doesnt exist", task_order_id);
        return 1;
    }

    snprintf(task->name, TASKNAME_MAXLEN, "%s", name);

    log_ok("[task %02d] task modified\n", task_order_id);

    return 0;
    
}

int TM_move_task_start(TaskManager* tm, int task_order_id, Timestamp start)
{
    Task* task = tm->task_list[task_order_id];

    if(task == NULL) {
        log_error("[task %0d] task doesnt exist", task_order_id);
        return 1;
    }

    Timestamp end = calculate_end_timestamp(start, task->duration_h);
    int priority = compare_timestamp(&task->start, &start);
    if(priority == 0) return 0;
    else if(priority>0){ // new start time is earlier
        TM_modify_task_start(tm, task_order_id, start);
        TM_modify_task_end(tm, task_order_id, end);
    } else if(priority<0){ // new start time is later
        TM_modify_task_end(tm, task_order_id, end);
        TM_modify_task_start(tm, task_order_id, start);
    }
    return 0;
}

int TM_save_state(TaskManager* tm)
{
    return TM_save_state_to_date(tm, tm->task_date);
}

int TM_save_state_to_date(TaskManager* tm, const Date target_date)
{
    TM_state tms;
    char state_file[PATH_MAX];

    snprintf(state_file, PATH_MAX, STATE_DIR STATE_FILE);

    FILE *fp = fopen(state_file, "w");
    if(!fp) {
        log_error("fopen: %s", strerror(errno));
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

    // state saving happens here
    size_t bytes = fwrite(&tms, 1, sizeof(TM_state), fp);
    if(bytes < sizeof(TM_state)){
        log_error("error on fwrite: %ld Bytes written (expect=%ld)\n", bytes, sizeof(TM_state));
        goto error_handling;
    }

    fclose(fp);
    fp = NULL;

    if(g_verbose) {
        log_ok("current state saved");
    }

    return 0;

error_handling:
    if(fp != NULL) {
        fclose(fp);
    }
    return 1;
}

int TM_restore_state(TaskManager *tm)
{
    return TM_restore_state_from_date(tm, tm->task_date);
}

int TM_restore_state_from_date(TaskManager *tm, const Date target_date)
{
    TM_state tms;
    char state_file[PATH_MAX];

    snprintf(state_file, PATH_MAX, STATE_DIR STATE_FILE);

    FILE *fp = fopen(state_file, "r");
    if(!fp) {
        if(g_verbose) {
            log_warn("no state to restore");
            log_warn( "fopen: %s", strerror(errno));
        }
        return 1;
    }

    size_t bytes = fread(&tms, 1, sizeof(TM_state), fp);
    if(bytes < sizeof(TM_state)){
        log_error("error on fread: %ld Bytes read"
                "(expect=%ld)\n", bytes, sizeof(TM_state));
        goto error_handling;
    }
    fclose(fp);
    memcpy(tm, &tms.tm, sizeof(TaskManager));

    Task* t;
    for(int i = 0; i < tms.tm.n_active_tasks; i++){
        t = malloc(sizeof(Task));    

        int order_id = tms.tm_task[i].order_id;

        if(t == NULL){
            log_error("malloc() failed");
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
        log_ok("last state restored");
    }

    return 0;

error_handling:
    if(fp != NULL) {
        fclose(fp);
    }
    return 1;

}

int TM_refresh_state(TaskManager* tm)
{
    TM_state tms;
    char state_file[PATH_MAX];

    snprintf(state_file, PATH_MAX, STATE_DIR STATE_FILE);

    FILE *fp = fopen(state_file, "r");
    if(!fp) {
        if(g_verbose) {
            log_warn("no state to restore");
            log_warn("fopen: %s", strerror(errno));
        }
        return 1;
    }

    size_t bytes = fread(&tms, 1, sizeof(TM_state), fp);
    if(bytes < sizeof(TM_state)){
        log_error("error on fread: %ld Bytes read (expect=%ld)\n", bytes, sizeof(TM_state));
        goto error_handling;
    }
    fclose(fp);

    assert(tms.tm.n_active_tasks == tm->n_active_tasks);
    assert(compare_date(&tms.tm.task_date, &tm->task_date) == 0);
    Task* t;
    for(int i = 0; i < tms.tm.n_active_tasks; i++){
        t = tm->task_list[i];

        int order_id = tms.tm_task[i].order_id;

        if(t == NULL){
            log_error("malloc() failed");
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
        log_ok("last state refreshed");
    }
    return 0;

error_handling:
    if(fp != NULL) {
        fclose(fp);
    }
    return 1;

}

int TM_reset_state(TaskManager* tm)
{
    char state_file[PATH_MAX];
    snprintf(state_file, PATH_MAX, STATE_DIR STATE_FILE);
    if(unlink(state_file) != 0){
        log_warn("unlink: %s", strerror(errno));
        return 1;
    }
    return 0;
}

int TM_sort_tasks(TaskManager* tm)
{
    qsort(&tm->task_list[0], tm->n_active_tasks, sizeof(Task*),
            compare_and_reorder_tasks);
    return 0;
}

int TM_export_to_ICS(TaskManager* tm)
{
    icaltimetype today = {
        .year = tm->task_date.year,
        .month = tm->task_date.month,
        .day = tm->task_date.day,
        .is_date = false,
    };

    icalcomponent* c = icalcomponent_new(ICAL_VCALENDAR_COMPONENT);

    icalcomponent_add_property(c, icalproperty_new_version("2.0"));
    icalcomponent_add_property(c, icalproperty_new_prodid("-//reverie//tmcli//EN"));

    for(int i = 0; i < tm->n_active_tasks; i++)
    {
        Task* task = tm->task_list[i];
        icalcomponent* event = icalcomponent_new(ICAL_VEVENT_COMPONENT);
        icalcomponent_add_property(event, icalproperty_new_summary(task->name));
        icaltimetype event_start = today;
        event_start.hour = task->start.time.hour;
        event_start.minute = task->start.time.min;

        icaltimetype event_end = today;
        event_end.hour = task->end.time.hour;
        event_end.minute = task->end.time.min;

        icalcomponent_add_property(event, icalproperty_new_dtstart(event_start));
        icalcomponent_add_property(event, icalproperty_new_dtend(event_end));

        icalcomponent_add_component(c, event);
    }

    char *ical_string = icalcomponent_as_ical_string(c);

    char filename[32];
    snprintf(filename, sizeof(filename), EXPORT_FILE, tm->task_date.day,
           tm->task_date.month, tm->task_date.year);
    FILE *fp = fopen(filename, "w");
    if(!fp) {
        log_error("fopen: %s", strerror(errno));
        goto error_handling;
    }

    fprintf(fp, "%s", ical_string);

    // cleanup
    icalcomponent_free(c);
    icaltimezone_free_builtin_timezones();
    icalmemory_free_ring();

    if(fclose(fp) == EOF) {
        log_error("fclose: %s", strerror(errno));
        goto error_handling;
    }

    log_ok("tasks exported to %s", filename);
    return 0;

error_handling:
    if(fp != NULL) fclose(fp);
    if(c != NULL) icalcomponent_free(c);
    return -1;
}

int TM_get_curr_taskid(TaskManager* tm)
{
   struct tm* curr_time = localtime(&(time_t){time(NULL)}) ;
   Timestamp now = { 
        .time.hour = curr_time->tm_hour, .time.min = curr_time->tm_min, 
        .date.day = curr_time->tm_mday, .date.month = curr_time->tm_mon+1, .date.year = (curr_time->tm_year+1900)
   };

   for(int i = 0; i < tm->n_active_tasks; i++){
        Task* task = tm->task_list[i];
        long after_start = compare_timestamp(&now, &task->start); 
        long before_end = compare_timestamp(&now, &task->end); 
        if(after_start >= 0 && before_end < 0) return task->id;
   }
   return -1;
}



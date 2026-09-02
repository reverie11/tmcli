#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <limits.h>

#include "log.h"
#include "tmcli.h"
#include "types.h"
#include "utils.h"
#include "cmd.h"

#ifndef VERSION
#define VERSION "unknown"
#endif

#ifndef AUTHOR
#define AUTHOR "unknown"
#endif

void print_help() {
    printf(
        "Usage: tmcli [OPTIONS] [PRE-COMMAND] COMMAND [ARGS...]\n"
        "\n"
        "OPTIONS\n"
        "  -v, --verbose              Enable verbose output\n"
        "  -h, --help                 Show this help message\n"
        "\n"
        "PRE-COMMAND\n"
        "  on       DATE              Temporarily set the working date, default is today\n"
        "\n"
        "COMMAND\n"
        "  add      STRT ENDT NAME    Add a new task with start-, endtime, and name\n"
        "  modify   T_ID OBJT VALUE   Modify the object value of an existing task of specified id\n"
        "  move     T_ID      TIME    Move anexisting task of a specified id to specified time\n"
        "  delete   T_ID              Delete an existing task of a specified id\n"
        "  show                       Show all exisiting tasks\n"
        "  export                     export all existiing tasks to ICS-Format (.ics)\n"
        "  reset                      Reset task-list\n"
        "\n"
        "OBJECTS\n"
        "  start[.time]               task's start.time\n"
        "  start.date                 task's start.date\n"
        "  end[.time]                 task's end.time\n"
        "  end.date                   task's end.date\n"
        "  name                       task's name\n"
        "\n"
        "FORMAT\n"
        "  DATE                       D[D.MM.YYYY]\n"
        "  TIME                       H[H:MM]\n"
        "\n"
        "Version  :  %s\n"
        "Author   :  %s\n",
        VERSION,
        AUTHOR
    );
}

void print_completion(int argc, char** argv ) {

    TM_state tms = {.tm.n_active_tasks = 0};

    char state_file[PATH_MAX];
    Date target_date = get_date_today();
    Time now = get_time_now();

    const Date today = get_date_today();
    const Date tmr = shift_date_by_days(&today, 1);
    const Date ytd = shift_date_by_days(&today, -1);

    int ind = 0;
    
    if(argc >= 1 && strcmp(argv[0], CMD_STR[_ON]) == 0){
        if(argc == 1) {
            printf("%s ", date_to_str(&target_date));
            for(int i=0; i < N_VALS; i++) printf("%s ", VAL_STR[i]);
        } else if (argc >= 2){
            if(strcmp(argv[1], VAL_STR[TDY]) == 0){
                target_date = today;
            } else if(strcmp(argv[1], VAL_STR[TMR]) == 0){
                target_date = tmr;
            } else if(strcmp(argv[1], VAL_STR[YTD]) == 0){
                target_date = ytd;
            } else {
                target_date = str_to_date(argv[1]);
            }

            if(validate_date(target_date ) == 0 ) {
                ind+=2;
            }
        }

    }

    snprintf(state_file, PATH_MAX, STATE_DIR STATE_FILE);

    // quick fetch state
    FILE *fp = fopen(state_file, "r");

    if(fp != NULL){
        fread(&tms, 1, sizeof(TM_state), fp);
        fclose(fp);
    }

    if(argc == ind){
        // tmcli 
       for(int i = (ind==0)?0:1; i < N_CMDS; i++) printf("%s ", CMD_STR[i]);
    } else if(argc == ind+1){
        // tmcli CMD
        if( strcmp(argv[ind], CMD_STR[MOD]) == 0 ||
            strcmp(argv[ind], CMD_STR[MOV]) == 0 ||
            strcmp(argv[ind], CMD_STR[DEL]) == 0 )
        {
            for(int i = 0; i < tms.tm.n_active_tasks; i++) printf("%d ", i);
        } else if ( strcmp(argv[ind], CMD_STR[ADD]) == 0 ){
            printf("%02d:%02d ", now.hour, now.min);
        }
    } else if(argc == ind+2){ 
        // tmcli CMD ARG1 
        if( strcmp(argv[ind], CMD_STR[MOD]) == 0 )
            for(int i = 0; i < N_OBJS; i++) printf("%s ", OBJ_STR[i]);
        else if ( strcmp(argv[ind], CMD_STR[ADD]) == 0 )
            printf("%02d:%02d ", now.hour+1, now.min);
        else if (strcmp(argv[ind], CMD_STR[MOV]) == 0) 
            printf("%02d:%02d ", now.hour+1, now.min);
    } else if(argc == ind+3){
        // tmcli CMD ARG1 ARG2
        if ( strcmp(argv[ind], CMD_STR[ADD]) == 0 ||
             (strcmp(argv[ind], CMD_STR[MOD]) == 0 && strcmp(argv[ind+2], OBJ_STR[NAME]) == 0)
        ){
            printf("taskNameA taskNameB taskNameC");
        } else if ( strcmp(argv[ind], CMD_STR[MOD]) == 0){

            if( strcmp(argv[ind+2], OBJ_STR[START])         == 0 ||
                strcmp(argv[ind+2], OBJ_STR[START_TIME])    == 0 ||
                strcmp(argv[ind+2], OBJ_STR[END])           == 0 ||
                strcmp(argv[ind+2], OBJ_STR[END_TIME])      == 0
            ){
                printf("%02d:%02d ", now.hour, now.min);
            } else if( strcmp(argv[ind+2], OBJ_STR[START_DATE]) == 0 ||
                       strcmp(argv[ind+2], OBJ_STR[END_DATE])   == 0
            ){
                printf("%02d.%02d.%04d ", target_date.day, target_date.month, target_date.year);
                for(int i=0; i < N_VALS; i++) printf("%s ", VAL_STR[i]);
            }
        } 
    }   

}

int main(int argc, char** argv)
{
    g_verbose = 0;

    char msg[MSG_MAXLEN];
    bzero(msg, sizeof(msg));

    // strictly meant for bash completion
    if(argc > 1 && (strcmp(argv[1], "--complete-args") == 0)){
        //if(argc > 2) print_completion(argc-3, argv+3, str_to_date(argv[2]));
        print_completion(argc-2, argv+2);
        return 0;
    }

    static struct option long_options[] = {
        {"verbose",         no_argument,       0,   'v'},
        {"help",            no_argument,       0,   'h'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, ":vh", long_options, NULL)) != -1) {
        switch (opt) {
        case 'v':
            g_verbose = 1;
            break;
        case 'h':
            print_help();
            return 0;
        case '?':
        default:
            snprintf(msg, sizeof(msg), "unknown options: -%c", optopt);
            log_error("%s\n", msg);
            fprintf(stderr, "try passing --help instead\n");
            return 1;
        }
    }

    TaskManager tm;
    bzero(&tm, sizeof(TaskManager));
    TM_init(&tm);
    TM_restore_state(&tm);

    const Date today = get_date_today();
    const Date tmr = shift_date_by_days(&today, 1);
    const Date ytd = shift_date_by_days(&today, -1);

    const char* cmd = CMD_STR[SHW];
    const char* value;
    const char* object;
    const char* start;
    const char* end;   
    const char* name;  
    Timestamp tstamp;
    
    int status = 0;
    int n_args = argc - optind;

    if(optind < argc) {
        cmd = argv[optind++];
        n_args = argc - optind;
    }

    if(strcmp(cmd, CMD_STR[_ON]) == 0){
        if(n_args < 1) {
            snprintf(msg, sizeof(msg), "missing args");
            goto error_handling;
        }

        value = argv[optind++];

        if(strcmp(value, VAL_STR[TDY]) == 0){
            value = date_to_str(&today);
        } else if(strcmp(value, VAL_STR[TMR]) == 0){
            value = date_to_str(&tmr);
        } else if(strcmp(value, VAL_STR[YTD]) == 0){
            value = date_to_str(&ytd);
        }
        Date d = str_to_date(value);

        if (d.day == -1){
            snprintf(msg, sizeof(msg), "str_do_date fail");
            goto error_handling;
        }

        tm.task_date = d;
        TM_save_state(&tm);
        cmd = argv[optind++];
        n_args = argc - optind;
        if(cmd == NULL) cmd = CMD_STR[SHW];
     }


    // add      STRT ENDT NAME
    if(strcmp(cmd, CMD_STR[ADD]) == 0){
        if(n_args < 3) {
            snprintf(msg, sizeof(msg), "missing args");
            goto error_handling;
        }

        start = argv[optind++];
        end   = argv[optind++];
        name  = argv[optind++];
        int id = TM_create_task(&tm, str_to_time(start), str_to_time(end), name);
        TM_sort_tasks(&tm);
        TM_save_state(&tm);
        TM_refresh_state(&tm);

        TM_print_all_tasks_highlight(&tm, 0, id);
        TM_save_state(&tm);

    // delete   T_ID 
    } else if(strcmp(cmd, CMD_STR[DEL]) == 0){
        if(n_args < 1) {
            snprintf(msg, sizeof(msg), "missing args");
            goto error_handling;
        }

        int order_id = str_to_uint(argv[optind++]);
        if(order_id == -1) {
            snprintf(msg, sizeof(msg), "conversion error");
            goto error_handling;
        }
        if(order_id >= tm.n_active_tasks) {
            snprintf(msg, sizeof(msg), "ID is out of range");
            goto error_handling;
        }
        TM_delete_task(&tm, order_id);
        TM_save_state(&tm);

    // modify   T_ID OBJT TIME
    } else if(strcmp(cmd, CMD_STR[MOD]) == 0){
        if(n_args < 3) {
            snprintf(msg, sizeof(msg), "missing args");
            goto error_handling;
        }

        int order_id = str_to_uint(argv[optind++]);
        if(order_id == -1){
            snprintf(msg, sizeof(msg), "conversion error");
            goto error_handling;
        }
        if(order_id >= tm.n_active_tasks) {
            snprintf(msg, sizeof(msg), "ID is out of range");
            goto error_handling;
        }
        int id = tm.task_list[order_id]->id;
        object = argv[optind++];
        value = argv[optind++];

        if(strcmp(value, VAL_STR[TDY]) == 0){
            value = date_to_str(&today);
        } else if(strcmp(value, VAL_STR[TMR]) == 0){
            value = date_to_str(&tmr);
        } else if(strcmp(value, VAL_STR[YTD]) == 0){
            value = date_to_str(&ytd);
        }

        if(strcmp(object, OBJ_STR[NAME]) != 0){
            tstamp = str_to_timestamp(value);
        }
        
        if(strcmp(object, OBJ_STR[NAME]) == 0){
            status = TM_modify_task_name(&tm, order_id, value);
        } else if(strcmp(object, OBJ_STR[START]) == 0 ||
                  strcmp(object, OBJ_STR[START_TIME]) == 0){
            status = TM_modify_task_start(&tm, order_id, tstamp);
        } else if(strcmp(object, OBJ_STR[END]) == 0 || 
                  strcmp(object, OBJ_STR[END_TIME]) == 0){
            status = TM_modify_task_end(&tm, order_id, tstamp);
        } else if(strcmp(object, OBJ_STR[START_DATE]) == 0){
            status = TM_modify_task_start_date(&tm, order_id, tstamp.date);
        } else if(strcmp(object, OBJ_STR[END_DATE]) == 0){
            status = TM_modify_task_end_date(&tm, order_id, tstamp.date);
        } 

        if(status != 0) goto error_handling;

        TM_sort_tasks(&tm);
        TM_save_state(&tm);
        TM_refresh_state(&tm);
        TM_print_all_tasks_highlight(&tm, 0, id);
    
    // move     T_ID      TIME
    } else if(strcmp(cmd, CMD_STR[MOV]) == 0){
        if(n_args < 2) {
            snprintf(msg, sizeof(msg), "missing args");
            goto error_handling;
        }

        int order_id = str_to_uint(argv[optind++]);
        if(order_id == -1){
            snprintf(msg, sizeof(msg), "conversion error");
            goto error_handling;
        }
        if(order_id >= tm.n_active_tasks) {
            snprintf(msg, sizeof(msg), "ID is out of range");
            goto error_handling;
        }

        int id = tm.task_list[order_id]->id;
        tstamp = str_to_timestamp(argv[optind++]);
        TM_move_task_start(&tm,order_id, tstamp);

        TM_sort_tasks(&tm);
        TM_save_state(&tm);
        TM_refresh_state(&tm);

        TM_print_all_tasks_highlight(&tm, 0, id);

    // show
    } else if(strcmp(cmd, CMD_STR[SHW]) == 0){
        if(g_verbose) TM_print_self(&tm);
        if(n_args > 0){
            int mode = str_to_uint(argv[optind++]);
            TM_print_all_tasks(&tm, mode);
        } else{
            int curr_taskid = TM_get_curr_taskid(&tm);
            TM_print_all_tasks_highlight(&tm, 0, curr_taskid);
        }

    // export
    } else if(strcmp(cmd, CMD_STR[EXP]) == 0){
        if(TM_export_to_ICS(&tm) != 0){
            snprintf(msg, sizeof(msg), "export failed");
            goto error_handling;
        } 
        snprintf(msg, sizeof(msg), "tasks exported to ./" EXPORT_FILE,
            tm.task_date.day, tm.task_date.month, tm.task_date.year);
        log_info("%s\n", msg);

    // reset
    } else if(strcmp(cmd, CMD_STR[RST]) == 0){
        if (TM_reset_state(&tm) != 0){
            snprintf(msg, sizeof(msg), "reset failed");
        }
        snprintf(msg, sizeof(msg), "task list reseted");
        log_info("%s\n", msg);

    } else {
        snprintf(msg, sizeof(msg), "unknown command: %s", cmd);
        goto error_handling;
    }

    TM_delete_all_tasks(&tm);
    return 0;

error_handling:
    log_error("%s\n", msg);
    fprintf(stderr, "try passing --help instead\n");
    TM_delete_all_tasks(&tm);
    return 1;
}


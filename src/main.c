#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>

#include "colors.h"
#include "commands.h"
#include "tmcli.h"

void print_help() {
    printf(
        "Usage: tmcli [OPTIONS] COMMAND [ARGS...]\n\n"
        "COMMANDS\n"
        "  add      START END NAME     Add a new task with starttime START, \n"
        "                                 endtime END, and name NAME\n"
        "  modify   ID    OBJ VAL      Modify the start- or endtime of an existing task\n"
        "  move     ID    VAL          Move existing task to different time\n"
        "  delete   ID                 Delete an existing task\n"
        "  show                        Show all tasks\n"
        "  export                      export to ICS-Format (.ics)\n"
        "  reset                       Reset TMCLI's state\n"
        "\n"
        "OBJECTS\n"
        " start                        task's starttime\n"
        " end                          task's endtime\n"
        "\n"
        "OPTIONS\n"
        "  -v, --verbose               Enable verbose output\n"
        "  -h, --help                  Show this help message\n"
        "\n"
        "Version  :  1.0.0\n"
        "Author   :  reverie\n"
    );
}

void print_completion(int argc, char** argv) {

    TM_state tms = {.tm.n_active_tasks = 0};

    // quick fetch state
    FILE *fp = fopen(STATE_FILE, "r");
    if(fp != NULL){
        size_t bytes = fread(&tms, 1, sizeof(TM_state), fp);
        fclose(fp);
        if(bytes < sizeof(TM_state)) return;
    }

    if(argc == 0){
       for(int i = 0; i < N_CMDS; i++) printf("%s ", CMD_STR[i]);
    } else if(argc == 1){
        if(strcmp(argv[0], CMD_STR[MOD]) == 0 ||
           strcmp(argv[0], CMD_STR[MOV]) == 0 ||
           strcmp(argv[0], CMD_STR[DEL]) == 0 ){
            for(int i = 0; i < tms.tm.n_active_tasks; i++) printf("%d ", i);
        }
    } else if(argc == 2){
        if( strcmp(argv[0], CMD_STR[MOD]) == 0 ){
            for(int i = 0; i < N_OBJS; i++) printf("%s ", OBJ_STR[i]);
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
        print_completion(argc-2, argv+2);
        goto cleanup;
    }

    static struct option long_options[] = {
        {"verbose",         no_argument,       0,   'v'},
        {"help",            no_argument,       0,   'h'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "vh", long_options, NULL)) != -1) {
        switch (opt) {
        case 'v':
            g_verbose = 1;
            break;
        case 'h':
            print_help();
            return 0;
        default:
            print_help();
            return 1;
        }
    }

    TaskManager tm;
    TM_init(&tm);
    TM_restore_state(&tm);

    char* cmd = argv[optind++];
    int n_args = argc - optind;

    if(cmd == NULL){
        goto error_handling;
    }

    if(strcmp(cmd, CMD_STR[ADD]) == 0){
        if(n_args < 3) goto error_handling;
        char* start = argv[optind++];
        char* end = argv[optind++];
        char* name = argv[optind++];
        int id = TM_create_task(&tm, str_to_time(start), str_to_time(end),
                name);
        TM_sort_tasks(&tm);
        TM_print_all_tasks_highlight(&tm, 0, id);
        TM_save_state(&tm);

    } else if(strcmp(cmd, CMD_STR[DEL]) == 0){
        if(n_args < 1) goto error_handling;
        int order_id = str_to_uint(argv[optind++]);
        if(order_id == -1) goto error_handling;
        TM_delete_task(&tm, order_id);
        TM_save_state(&tm);

    } else if(strcmp(cmd, CMD_STR[MOD]) == 0){
        if(n_args < 3) goto error_handling;

        int order_id = str_to_uint(argv[optind++]);
        if(order_id == -1){
            snprintf(msg, sizeof(msg), "conversion error");
            goto error_handling;
        }
        if(tm.task_list[order_id] == NULL) {
            snprintf(msg, sizeof(msg), "ID is out of range");
            goto error_handling;
        }
        int id = tm.task_list[order_id]->id;
        char* object = argv[optind++];
        Time value = str_to_time(argv[optind++]);

        if(strcmp(object, "start") == 0){
            TM_modify_task_start(&tm, order_id, value);
        } else if(strcmp(object, "end") == 0){
            TM_modify_task_end(&tm, order_id, value);
        }
        TM_sort_tasks(&tm);
        TM_print_all_tasks_highlight(&tm, 0, id);
        TM_save_state(&tm);

    } else if(strcmp(cmd, CMD_STR[MOV]) == 0){
        if(n_args < 2) {
            goto error_handling;
        }

        int order_id = str_to_uint(argv[optind++]);
        if(order_id == -1) goto error_handling;
        if(tm.task_list[order_id] == NULL) {
            snprintf(msg, sizeof(msg), "ID is out of range");
            goto error_handling;
        }

        int id = tm.task_list[order_id]->id;
        Time value = str_to_time(argv[optind++]);
        TM_move_task_start(&tm,order_id, value);

        TM_sort_tasks(&tm);
        TM_print_all_tasks_highlight(&tm, 0, id);
        TM_save_state(&tm);

    } else if(strcmp(cmd, CMD_STR[SHW]) == 0){
        if(g_verbose) TM_print_self(&tm);
        if(n_args > 0){
            int mode = str_to_uint(argv[optind++]);
            TM_print_all_tasks(&tm, mode);
        } else{
            int curr_taskid = TM_get_curr_taskid(&tm);
            TM_print_all_tasks_highlight(&tm, 0, curr_taskid);
        }

    } else if(strcmp(cmd, CMD_STR[EXP]) == 0){
        int status = TM_export_to_ICS(&tm);
        if(status == -1){
            snprintf(msg, sizeof(msg), "export failed");
        } else snprintf(msg, sizeof(msg), "tasks exported to ./%s", EXPORT_FILE);
        fprintf(stdout, CYAN "INFO: " RESET "%s\n", msg);

    } else if(strcmp(cmd, "reset") == 0){
        unlink(STATE_FILE);
        snprintf(msg, sizeof(msg), "task list reseted");
        fprintf(stdout, CYAN "INFO: " RESET "%s\n", msg);

    } else {
        goto error_handling;
    }

cleanup:
    TM_delete_all_tasks(&tm);
    return 0;

error_handling:
    print_help();
    TM_delete_all_tasks(&tm);
    return 1;
}


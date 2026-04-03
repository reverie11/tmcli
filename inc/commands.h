#ifndef COMMANDS_H
#define COMMANDS_H

typedef enum {
    ADD, 
    DEL,
    MOD,
    MOV,
    SHW,
    EXP,
    RST,
    N_CMDS,
} Cmd_list;

static const char *CMD_STR[N_CMDS] = {
    [ADD] = "add", 
    [DEL] = "delete",
    [MOD] = "modify",
    [MOV] = "move",
    [SHW] = "show", 
    [EXP] = "export", 
    [RST] = "reset",
};

typedef enum{
    START, 
    END,
    NAME,
    N_OBJS,
} Obj_list;

static const char *OBJ_STR[N_CMDS] = {
    [START] = "start",
    [END]   = "end",
    [NAME]  = "name",
};

#endif

#ifndef CMD_H
#define CMD_H

typedef enum {
    _ON, // pre-command
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
    [_ON] = "on",
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
    START_TIME, 
    START_DATE, 
    END,
    END_TIME,
    END_DATE,
    NAME,
    N_OBJS,
} Obj_list;

static const char *OBJ_STR[N_OBJS] = {
    [START]         = "start",
    [START_TIME]    = "start.time",
    [START_DATE]    = "start.date",
    [END]           = "end",
    [END_TIME]      = "end.time",
    [END_DATE]      = "end.date",
    [NAME]          = "name",
};

typedef enum{
    TDY,
    YTD,
    TMR,
    ALL,
    N_VALS
} Val_list;

static const char *VAL_STR[N_VALS] = {
    [TDY] = "today",
    [YTD] = "yesterday",
    [TMR] = "tomorrow",
    [ALL] = "all",
};

#endif //CMD_H

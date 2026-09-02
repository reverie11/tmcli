#ifndef CMD_H
#define CMD_H

typedef enum {
    // PRE-COMMANDS
    CMD_ON, 
    // COMMANDS
    CMD_ADD, 
    CMD_DEL,
    CMD_MOD,
    CMD_MOV,
    CMD_SHW,
    CMD_EXP,
    CMD_RST,
    N_CMD,
} CmdList;

static const char *STR_CMD[N_CMD] = {
    [CMD_ON] = "on",
    [CMD_ADD] = "add", 
    [CMD_DEL] = "delete",
    [CMD_MOD] = "modify",
    [CMD_MOV] = "move",
    [CMD_SHW] = "show", 
    [CMD_EXP] = "export", 
    [CMD_RST] = "reset",
};

typedef enum{
    OBJ_START, 
    OBJ_START_TIME, 
    OBJ_START_DATE, 
    OBJ_END,
    OBJ_END_TIME,
    OBJ_END_DATE,
    OBJ_NAME,
    N_OBJ,
} ObjList;

static const char *STR_OBJ[N_OBJ] = {
    [OBJ_START]         = "start",
    [OBJ_START_TIME]    = "start.time",
    [OBJ_START_DATE]    = "start.date",
    [OBJ_END]           = "end",
    [OBJ_END_TIME]      = "end.time",
    [OBJ_END_DATE]      = "end.date",
    [OBJ_NAME]          = "name",
};

typedef enum{
    VAL_DATE_TDY,
    VAL_DATE_YTD,
    VAL_DATE_TMR,
    N_VAL_DATE
} ValDateList;

static const char *STR_VAL_DATE[N_VAL_DATE] = {
    [VAL_DATE_TDY] = "today",
    [VAL_DATE_YTD] = "yesterday",
    [VAL_DATE_TMR] = "tomorrow",
};

typedef enum{
    MODE_ALL,
    N_MODES
} ModeList;

#endif //CMD_H

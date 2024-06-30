#ifndef GAME_CONTROL_H
#define GAME_CONTROL_H

#include "puzzle.h"

enum commands 
{
    CMD_AUTO_XMARK,
    CMD_DELETE_TEMP_MARKS,
    CMD_CLEAR,
    CMD_CAPTURE,
    CMD_RESTORE_CAPTURE,
    CMD_SAVE,
    CMD_QUIT,
    CMD_N
};

enum game_return_code
{
    GAME_RET_SUCCESS = 0,
    GAME_RET_QUIT,

    GAME_RET_ERROR_LOAD = -1,
    GAME_RET_ERROR_MEMORY = -2,

    GAME_RET_ERROR_INTERNAL = -99
};

extern char *command_mode_choices[CMD_N];
extern char *command_mode_desc[CMD_N];

enum game_return_code new_game(void);
enum game_return_code continue_game(void);

#endif // GAME_CONTROL_H

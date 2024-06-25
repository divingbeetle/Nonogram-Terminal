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

extern char *command_mode_choices[CMD_N];
extern char *command_mode_desc[CMD_N];

int play(const struct puzzle *pz);
int load_and_play(void);

#endif // GAME_CONTROL_H

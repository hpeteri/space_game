#ifndef GAME_H
#define GAME_H

typedef enum PROGRAM_STATE{
  PROGRAM_STATE_GAME = 0,
  PROGRAM_STATE_MENU,
  PROGRAM_STATE_EDITOR,
} PROGRAM_STATE;

typedef struct ProgramState{
  PROGRAM_STATE state;
  
} ProgramState;

PROGRAM_STATE current_program_state();
void do_program_state_input();
void update_and_draw();
#endif

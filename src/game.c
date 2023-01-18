#include "game.h"

#include "console/console.h"
static ProgramState program_state;

PROGRAM_STATE current_program_state(){
  return program_state.state;
}

void do_program_state_input(){

  if(input_is_key_up(KC_F3)){
    toggle_console();
  }
}

void update_and_draw(){
  do_program_state_input();

  do_editor_camera();
    
  draw_scene();
  //draw_scene_simple();
  update_and_draw_console();
  draw_editor();
}

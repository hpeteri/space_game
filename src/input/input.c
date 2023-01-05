#include "input.h"

static Input input_;

void input_new_frame(){
  input_.mouse_delta = vec2_zero();

  for(int i = 0; i < 256; i++){
    input_.keyboard[i].was_down = input_.keyboard[i].is_down;
  }
  
  for(int i = 0; i < 3; i++){
    input_.mouse_buttons[i].was_down = input_.mouse_buttons[i].is_down;
  }
}

// === mouse move ===
void input_set_mouse_pos(float x, float y, int width, int height){
  input_.mouse_delta = input_.mouse_pos;
  
  input_.mouse_pos = vec2_make((x - width / 2.0) / (float)width,
                               (y - height / 2.0) / -(float)height);

  input_.mouse_delta = vec2_subv(input_.mouse_pos, input_.mouse_delta);
}

vec2 input_mouse_pos(){
  return input_.mouse_pos;
}

vec2 input_mouse_delta(){
  return input_.mouse_delta;
}

// === mouse buttons ===
void input_set_mouse_button(int button, int is_down){
  
  input_.mouse_buttons[button - 1].is_down = is_down;
}

int input_is_left_mouse_down(){
  
  return input_.left_mouse.is_down && !input_.left_mouse.was_down;
}

int input_is_left_mouse_held(){
  return input_.left_mouse.is_down;
}

int input_is_left_mouse_up(){
  return !input_.left_mouse.is_down && input_.left_mouse.was_down;
}

int input_is_middle_mouse_down(){
  return input_.middle_mouse.is_down && !input_.middle_mouse.was_down;
}
int input_is_middle_mouse_held(){
  return input_.middle_mouse.is_down;
}
int input_is_middle_mouse_up(){
  return !input_.middle_mouse.is_down && input_.middle_mouse.was_down;
}

int input_is_right_mouse_down(){
  return input_.right_mouse.is_down && !input_.right_mouse.was_down;
}

int input_is_right_mouse_held(){
  return input_.right_mouse.is_down;
}

int input_is_right_mouse_up(){
  return !input_.right_mouse.is_down && input_.right_mouse.was_down;
}

// === keyboard ===

void input_update_key(uint32_t key, int is_down){
  if(key < 256 && key >= 0)
    input_.keyboard[key].is_down = is_down;
}

int input_is_key_down(uint32_t key){
  if(key < 256)
    return input_.keyboard[key].is_down && !input_.keyboard[key].was_down;
  return 0;
}

int input_is_key_held(uint32_t key){
  if(key < 256){
    return input_.keyboard[key].is_down;
  }
  return 0;
}

int input_is_key_up(uint32_t key){
  if(key < 256){
    return !input_.keyboard[key].is_down && input_.keyboard[key].was_down;
  }
  return 0;
}

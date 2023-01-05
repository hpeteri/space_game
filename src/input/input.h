#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>
#include "cmath/n1_cmath.h"

struct InputKey{
  union{
    struct{
      int is_down:  1;
      int was_down: 1;
    };
    char state;
  };
};

typedef struct Input{
  struct InputKey keyboard[256];
  vec2 mouse_pos; //(-1 to 1)
  vec2 mouse_delta;
  union{
    struct {
      struct InputKey left_mouse;
      struct InputKey middle_mouse;
      struct InputKey right_mouse;
    };
    struct InputKey mouse_buttons[3];
  };
} Input;


void input_new_frame();
void input_set_mouse_pos(float x, float y, int width, int height);
void input_set_mouse_button(int button, int is_down);
vec2 input_mouse_pos();
vec2 input_mouse_delta();



int input_is_left_mouse_down();
int input_is_left_mouse_held();
int input_is_left_mouse_up();

int input_is_middle_mouse_down();
int input_is_middle_mouse_held();
int input_is_middle_mouse_up();

int input_is_right_mouse_down();
int input_is_right_mouse_held();
int input_is_right_mouse_up();

void input_update_key(uint32_t key, int is_down);

int input_is_key_down(uint32_t key);
int input_is_key_held(uint32_t key);
int input_is_key_up(uint32_t key);

#endif

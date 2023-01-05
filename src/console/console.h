#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>
#include "n1_cmath.h"
#include "c_allocator/n1_allocator.h"

typedef struct ConsoleString{
  char* string;
  vec2 size;
  
} ConsoleString;

typedef struct Console{
  n1_Allocator allocator;

  int is_inited : 1;
  int is_open   : 1;
  
  float openness;
  float blink;
  
  //history 
  int32_t       history_at;
  ConsoleString history[256];

  // input line
  int32_t input_line_max;
  int32_t input_line_len;
  int32_t input_line_at;
  char*   input_line;

  
} Console;

void make_console();
void free_console();
int  is_console_open();
void toggle_console();
void update_and_draw_console();

void console_push_control_keycode(uint32_t keycode);
void console_push_keycode(uint32_t keycode);


#endif

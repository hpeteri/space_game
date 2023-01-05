#ifndef CORE_H
#define CORE_H

#include "platform/src/n1_platform.h"
#include "sglr.h"

struct TimeStats{

  float    dt;
  float    game_dt;
  uint64_t frame_count;
  int      fps;
  uint64_t gpu_time;
  uint64_t cpu_time;
};

typedef struct Core{
  n1_Window* window;
  sglr_Context* renderer;
  
  struct TimeStats time;
} Core;

int make_core();
void free_core();

n1_Window*       get_main_window();
float            get_dt();
struct TimeStats get_time_stats();
void             next_frame();
void             end_frame();
void             update_time_stats_pre_swap();

#endif

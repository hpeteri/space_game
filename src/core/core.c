#include "sglr.h"
#include "core.h"

#include "../editor/editor.h"
#include "../tweak/tweak.h"

static Core core;

// internal time tracking
static uint64_t elapsed_time;
static uint64_t frame_start;
static uint64_t frame_end;
static int      fps;

int make_core(){
  
  Tweaks tweaks = get_tweaks();

  int window_width  = tweaks.startup.window_width;
  int window_height = tweaks.startup.window_height;
  
  // == window ===
  
  n1_Window* window = platform_create_window("simple window", window_width, window_height);
  core.window = window;

  
#if defined(DEBUG_BUILD)
  int is_debug = 1;
#elif defined(RELEASE_BUILD)
  int is_debug = 0;
#endif
  
  if(!platform_create_glcontext(window, 4, 6, is_debug)){
    perror("failed to create glContext\n");
    return 1;
  }
  
  platform_gl_swap_interval(tweaks.graphics.vsync);    
  
  platform_init_time();
  
  // == renderer ===

  sglr_Context* renderer = sglr_make_context(default_allocator());
  core.renderer = renderer;
  
  if(!renderer){
    perror("failed to create renderer context");
    return 1;
  }
  
  sglr_set_context(renderer);
  
  sglr_make_main_render_target(window_width, window_height, 0, GL_RGBA16F, GL_DEPTH_COMPONENT32F);
  sglr_set_render_target_debug_name(sglr_main_render_target(), "main_rt");
  
  glEnable(GL_MULTISAMPLE);


  // === editor ===
  make_editor();
  
  frame_start = platform_get_time_ns();
  return 0;


  
}

void free_core(){
  sglr_free_debug_logger();
  sglr_free_context(sglr_current_context());

  n1_Window* window = core.window;
  platform_free_glcontext(window);  
  platform_free_window(window);
}

n1_Window* get_main_window(){
  return core.window;
}

float get_dt(){
  return core.time.dt;
}

struct TimeStats get_time_stats(){
  return core.time;
}

void next_frame(){
  sglr_begin_query_time_elapsed_ns();
  sglr_stats_reset();

  fps ++;
  
  input_new_frame();
}

void end_frame(){

  frame_end = platform_get_time_ns();
  core.time.dt = platform_convert_ns_to_seconds(frame_end - frame_start);
  core.time.game_dt = core.time.dt;
  elapsed_time += frame_end - frame_start;

  frame_start = frame_end;
  
  if(elapsed_time >= second_to_ns){
    elapsed_time %= second_to_ns;

    core.time.fps = fps;
    fps = 0;
  } 
}


void update_time_stats_pre_swap(){
  core.time.gpu_time = sglr_end_query_time_elapsed_ns();
  core.time.cpu_time = platform_get_time_ns() - frame_start;
}

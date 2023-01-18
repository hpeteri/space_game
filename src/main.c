
#include <stdio.h>

#define N1_ALLOCATOR_IMPLEMENTATION
#include "c_allocator/n1_allocator.h"

#define N1_CMATH_IMPLEMENTATION
#include "n1_cmath.h"

#include "input/input.h"

#include "core/core.c"
#include "sglr.h"
#include "scene.c"
#include "console/console.h"

#include "tweak/tweak.h"

#include "game.c"

sglr_Texture missing_texture;
sglr_Texture white_texture;


#include "temp_shader_loading.c"

void create_temp_textures(){
  
  {
    uint32_t magenta = 0xff8a00f6;
    uint32_t black = 0xff000000;

    int size = 32;
    uint32_t bytes[4096];

    int frac = size / 32;
    int w = size / 4;
    for(int y = 0; y < size; y++){
      for(int x = 0; x < size; x++){

        
        const uint32_t color = ((x / w) + (y / w)) % 2 ? magenta : black;
        
        bytes[y * size + x] = color;
      }
    }
    missing_texture = sglr_make_texture_2d(size, size,
                                           GL_SRGB_ALPHA,
                                           GL_RGBA, GL_UNSIGNED_BYTE,
                                           bytes);
    sglr_set_texture(missing_texture);
    sglr_make_texture_mipmap(missing_texture);
    sglr_set_texture_min_filter(missing_texture, GL_NEAREST_MIPMAP_LINEAR);
    sglr_set_texture_wrap(missing_texture, GL_CLAMP_TO_EDGE);
    
    sglr_set_texture_debug_name(missing_texture, "missing_texture");
  }
  {
    uint32_t white = 0xffffffff;                      

    //white_texture = sglr_make_texture_2d_rgba(1, 1, &white);
    
    white_texture = sglr_make_texture_2d(1, 1,
                                         GL_SRGB_ALPHA,
                                         GL_RGBA, GL_UNSIGNED_BYTE,
                                         &white);
    
    sglr_set_texture(white_texture);
    sglr_set_texture_wrap(white_texture, GL_CLAMP_TO_EDGE);
    sglr_set_texture_debug_name(white_texture, "white_texture");
  }
}

void get_gl_extensions(){
 int extension_count;
 glGetIntegerv(GL_NUM_EXTENSIONS, &extension_count);

 #if 0
 for(int i = 0; i < extension_count; i++){
  const char* extension_name = glGetStringi(GL_EXTENSIONS, i);

  printf("[%d]: '%s'\n", i, extension_name);
 }
#endif

}

void perf_mon_amd(){
  int num_groups;
  uint32_t* groups;
  
  glGetPerfMonitorGroupsAMD(&num_groups, 0, NULL);
  sglr_check_error();
  printf("num groups: %d\n", num_groups);

  groups = malloc(sizeof(uint32_t) * num_groups);

  glGetPerfMonitorGroupsAMD(NULL, num_groups, groups);
  sglr_check_error();
  
  
  for(int i = 0; i < num_groups; i++){
    char group_name[256];
    group_name[0] = 0;
    glGetPerfMonitorGroupStringAMD(groups[i], 256, NULL, group_name);
    sglr_check_error();
    printf("[%d]: '%s'\n", i, group_name);


    int num_counters;
    int num_max_active;
    glGetPerfMonitorCountersAMD(groups[i],
                                &num_counters,
                                &num_max_active,
                                0,
                                NULL);
    printf("counters: %d (max active: %d)\n", num_counters, num_max_active);
    
    GLuint* counters = (GLuint*) malloc(num_counters * sizeof(GLuint));
    glGetPerfMonitorCountersAMD(groups[i], NULL, NULL,
                                num_counters,
                                counters);

    for(int ii = 0; ii < num_counters; ii++){
      GLuint counter = counters[ii];
      char counter_name[256];
      
      glGetPerfMonitorCounterStringAMD(groups[i],
                                       counter,
                                       256, NULL, counter_name);
      printf("-> [%d]: '%s'\n", ii, counter_name); 
      sglr_check_error();
    }

    free(counters);
  }
  
  free(groups);
}

#include <stdio.h>

int main(int argc, const char* argv[]){

  set_cwd(get_exe_dir());
  set_cwd("..");
  
  load_tweak_file();
  
  int err = make_core();
  if(err){
    return err;
  }
  
  n1_Window* window = get_main_window();

  create_temp_textures();
  
  printf("gpu:       %s\n", sglr_gpu_name());
  printf("vendor:    %s\n", sglr_gpu_vendor());
  printf("gl ver.:   %s\n", sglr_gl_version());
  printf("glsl ver.: %s\n", sglr_glsl_version());

  int is_running = 1;

  get_gl_extensions();
  
  while(is_running){
    
    next_frame();
    
    platform_window_get_events(window);
    n1_WindowEvent e = platform_window_get_next_event(window);
    while(e.type){
      if(e.type == EVENT_QUIT){
        printf("quit\n");
        is_running = 0;
      }
      else if(e.type == EVENT_MOUSE_MOVE){
        int mouse_x = e.mouse.x;
        int mouse_y = e.mouse.y;

        input_set_mouse_pos(mouse_x, mouse_y, window->width, window->height);
        
      }
      else if(e.type == EVENT_SIZE){
        sglr_resize_main_render_target(window->width,
                                       window->height);
        update_editor_camera(window->width, window->height);
        
      }else if(e.type == EVENT_KEY_DOWN){
        input_update_key(e.key.keycode, 1);
        console_push_control_keycode(e.key.keycode);
      }else if(e.type == EVENT_KEY_UP){
        input_update_key(e.key.keycode, 0);

      }else if(e.type == EVENT_MOUSE_DOWN){
        input_set_mouse_button(e.key.keycode, 1);
      }else if(e.type == EVENT_MOUSE_UP){
        input_set_mouse_button(e.key.keycode, 0);

      }else if(e.type == EVENT_CHAR){
        console_push_keycode(e.key.keycode);
      }
      
      e = platform_window_get_next_event(window);
    }
    
    
    sglr_set_render_target(sglr_main_render_target());
    sglr_set_clear_color_u32_rgba(0xff000000);
    sglr_set_clear_depth(1.0f);
    sglr_clear_render_target_depth();
    sglr_clear_render_target_color();
        
    update_and_draw();

    // compute thing
    
    
    sglr_flush();

    sglr_blit_main_render_target(window->width, window->height, GL_NEAREST);

    //update gpu and cpu time
    update_time_stats_pre_swap();
    
    //swap buffers
    platform_window_swap_buffers(window);
    
    end_frame();
  }

  free_core();
  return 0;
}

#include <stdio.h>

#define N1_ALLOCATOR_IMPLEMENTATION
#include "c_allocator/n1_allocator.h"

#define N1_CMATH_IMPLEMENTATION
#include "n1_cmath.h"

#include "n1_platform_main.c"

#include "input.c"

//renderer
#include "sglr.c"

sglr_Context* renderer;


uint32_t alloc_count = 0;
void* my_malloc(size_t size){
  alloc_count ++;
  printf("malloc (%d)\n",alloc_count);
  return malloc(size);
}

void* my_realloc(void* data, size_t size){
  printf("realloc, %p (%d)\n", data, alloc_count);
  return realloc(data, size);
}

void my_free(void* data){
  alloc_count --;
  printf("freed: %p (%d)\n", data, alloc_count);
  free(data);
}

int main(int argc, const char* argv[]){
  //Create window
  n1_Window* window = platform_create_window("simple window");
  perror("created window");

  if(!platform_create_glcontext(window, 4, 6, 1)){
    perror("failed to create glContext\n");
    return 1;
  }
  perror("created gl context");
  
  n1_Allocator renderer_allocator = default_allocator();
  
#if 0
  renderer_allocator.alloc        = my_malloc;
  renderer_allocator.realloc      = my_realloc;
  renderer_allocator.free         = my_free;
#endif
    
  renderer = sglr_make_context(renderer_allocator);
  
  sglr_set_context(renderer);
  sglr_make_shader_builtin_simple();

  sglr_make_main_render_target(256, 256, 4, GL_RGBA16F, GL_DEPTH24_STENCIL8);
  glEnable(GL_MULTISAMPLE);
  
  
  if(!renderer){
    perror("failed to create renderer context");
    return 1;
  }
  
  platform_init_time();
  
  char fps_buffer[50] = {0};
  char stat_buffer[1024] = {0};
  
  uint64_t ns_start = platform_get_time_ns();
  uint64_t ns_frame_start = ns_start;
    
  int ns_fps      = 0;
  int frame_count = 0;
  float dt = 0;

  int64_t gpu_time = 0;
  int64_t cpu_time = 0;
  
  int is_running = 1;
      
  int window_width = 0;
  int window_height = 0;

  sglr_Camera cam = sglr_make_camera();
  
  sglr_Texture missing_texture;
  sglr_Texture white_texture;
    
  {
    uint32_t magenta = 0xff8a00f6;
    uint32_t black = 0xff000000;
    uint32_t bytes[16] = {
      magenta, black, magenta, black,
      black, magenta, black, magenta,
      magenta, black, magenta, black,
      black, magenta, black, magenta,
    };
                      
    missing_texture = sglr_make_texture_2d_rgba(4, 4, bytes);
    sglr_set_texture_debug_name(missing_texture, "missing_texture");
  }
  {
    
    uint32_t white = 0xffffffff;                      
    white_texture = sglr_make_texture_2d_rgba(1, 1, &white);
    sglr_set_texture_debug_name(missing_texture, "white_texture");
  }

  printf("gpu:       %s\n", sglr_gpu_name());
  printf("vendor:    %s\n", sglr_gpu_vendor());
  printf("gl ver.:   %s\n", sglr_gl_version());
  printf("glsl ver.: %s\n", sglr_glsl_version());
 
  while(is_running){
    sglr_begin_query_time_elapsed_ns();
    sglr_stats_reset();
    
    //handle window events
    input_new_frame();
    
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

        input_set_mouse_pos(mouse_x, mouse_y, window_width, window_height);
        
      }
      else if(e.type == EVENT_SIZE){
        window_width = e.size.width;
        window_height = e.size.height;
        
        sglr_resize_main_render_target(window_width,
                                       window_height);

#if 0
        sglr_camera_set_ortho(&cam,
                              mat4_ortho_rh(-window_width / 2.0, window_width/ 2.0,
                                            window_height/ 2.0, -window_height/ 2.0,
                                            0.01, 10000)
                              );
#else
        sglr_camera_set_perspective(&cam,
                                    mat4_perspective_rh(90,
                                                        window_width / (float)window_height,
                                                        0.01,
                                                        100000));
          
#endif
            
      }else if(e.type == EVENT_KEY_DOWN){
        input_update_key(e.key.keycode, 1);

      }else if(e.type == EVENT_KEY_UP){
        input_update_key(e.key.keycode, 0);

      }else if(e.type == EVENT_MOUSE_DOWN){
        input_set_mouse_button(e.key.keycode, 1);

      }else if(e.type == EVENT_MOUSE_UP){
        input_set_mouse_button(e.key.keycode, 0);
      }
      
      
      e = platform_window_get_next_event(window);
    }

    ns_fps ++;
    frame_count ++;

    sglr_set_render_target(sglr_main_render_target());
    sglr_set_clear_color_i32_rgba(0xff323232);
    
    sglr_clear_render_target_depth();
    sglr_clear_render_target_color();
    
    //update
    {
      vec3 delta = vec3_zero();
      float speed = 500 * dt;
      
      if(input_is_key_down('W')){
        delta.z -= speed;
      }
      if(input_is_key_down('S')){
        delta.z += speed;
      }

      if(!input_is_key_down(' ')){
        
        if(input_is_key_down('A')){
          delta.x -= speed;
        }
        if(input_is_key_down('D')){
          delta.x += speed;
        }
        if(input_is_key_down('Q')){
          delta.y -= speed;
        }
        if(input_is_key_down('E')){
          delta.y += speed;
        }
      }
      
      sglr_camera_move(&cam, quat_mulv(sglr_camera_rot(cam), delta));
      
      if(input_is_right_mouse_down()){
        vec3 euler = cam.euler;
        
        float rot_speed = PI;

        vec3 mouse_delta = vec3_make(input_mouse_delta().x, input_mouse_delta().y, 0);
        
        //mouse_delta = quat_mulv(sglr_camera_rot(cam), mouse_delta); 
        euler.x += mouse_delta.y * rot_speed; 
        euler.y -= mouse_delta.x * rot_speed; 
        sglr_camera_set_euler(&cam, euler);
      }
      
    }
    
    { 
      sglr_CommandBuffer* cb = sglr_make_command_buffer();
      sglr_command_buffer_set_render_target(cb,
                                            sglr_main_render_target());

      {
        // triangles
        sglr_Material simple = sglr_make_material(sglr_make_shader_builtin_simple());
        sglr_set_material_texture_0(&simple, missing_texture.id);


        
        sglr_GraphicsPipeline pipeline = sglr_make_graphics_pipeline_default(simple,
                                                                             GL_TRIANGLES);
        
        sglr_CommandBuffer2* scb = sglr_make_command_buffer2_im(pipeline);
        
        sglr_command_buffer2_set_cam(scb, cam);
      
      
        sglr_immediate_triangle(scb,
                                vec3_make(0, 0, -1),
                                vec3_make(0,0,0),
                                ~0,
                                vec3_make(100, 0, -1),
                                vec3_make(1,0,0),
                                ~0,
                                vec3_make(0, 100, -1),
                                vec3_make(0,1,0),
                                ~0);
        
        sglr_command_buffer2_submit(scb, cb);
      }
      
      {
        //lines
        sglr_Material simple = sglr_make_material(sglr_make_shader_builtin_simple());
        sglr_set_material_texture_0(&simple, white_texture.id);
        
        sglr_GraphicsPipeline pipeline = sglr_make_graphics_pipeline_default(simple,
                                                                             GL_TRIANGLES);

        pipeline.renderer_state.flags ^= SGLR_CULL_FACE;
        
        sglr_CommandBuffer2* scb = sglr_make_command_buffer2_im(pipeline);
        sglr_command_buffer2_set_cam(scb, cam);
      

        float size = 100;

        //x
        sglr_immediate_line_2d(scb,
                               vec3_make(0, 1, 0), //normal
                               vec3_make(0,0, 0),
                               0xff0000ff,
                               vec3_make(size, 0, 0),
                               0xff0000ff,
                               10);
        
        //y
        sglr_immediate_line_2d(scb,
                               vec3_make(0, 0, 1), //normal
                               vec3_make(0, 0, 0),
                               0xff00ff00,
                               vec3_make(0, size, 0),
                               0xff00ff00,
                               10);

        //z
        sglr_immediate_line_2d(scb,
                               vec3_make(0, 1, 0), //normal
                               vec3_make(0, 0, 0),
                               0xffff0000,
                               vec3_make(0, 0, size),
                               0xffff0000,
                               10);

        sglr_immediate_aabb_outline(scb,
                                    vec3_make(size * -10, size * -10, size * -10),
                                    vec3_make(size * 10, size * 10, size * 10),
                                    ~0,
                                    10);
                                    
        sglr_command_buffer2_submit(scb, cb);
      }
      {
        //draw fps
        sglr_Material text_mat = sglr_make_material(sglr_make_shader_builtin_text());
        
        sglr_set_material_texture_0(&text_mat, sglr_make_bitmap_font_builtin().texture.id);
        
        sglr_GraphicsPipeline pipeline = sglr_make_graphics_pipeline_default(text_mat,
                                                                             GL_TRIANGLES);

        pipeline.renderer_state.flags ^= SGLR_CULL_FACE;
        pipeline.renderer_state.flags |= SGLR_BLEND;
                
        sglr_CommandBuffer2* scb = sglr_make_command_buffer2_im(pipeline);

        sglr_Camera cam2 = sglr_make_camera();
        
        
        sglr_camera_set_ortho(&cam2,
                              mat4_ortho_rh(0, window_width,
                                            window_height, 0,
                                            0.01, 10000)
                              );
        sglr_command_buffer2_set_cam(scb, cam2);
        
        sglr_immediate_text(scb,
                            fps_buffer,
                            vec3_make(10, window_height - 10 - 8 * 2, -1),
                            2,
                            ~0);

        {
          sprintf(stat_buffer,
                  "frame time: %f ms\n"
                  "gpu time:   %f ms\n"
                  "cpu time:   %f ms\n"
                  "\n"
                  "trianlges: %d",
                  dt * 1000,
                  gpu_time / 1000000.0f,
                  cpu_time / 1000000.0f,
                  sglr_stats_triangle_count());

          
          sglr_immediate_text(scb,
                              stat_buffer,
                              vec3_make(10, window_height - 10 - 8 * 2  - (2.5f * 8), -1),
                              1,
                              ~0);
          
          //printf("frame time :: %f ms\n", dt * 1000);
          //printf("gpu time   :: %f ms \n", gpu_time / (float)1000000);
          //printf("cpu time   :: %f ms \n", cpu_time / (float)1000000);
          //printf("\n");
        }
                
        sglr_command_buffer2_submit(scb, cb);
      }
      
      sglr_command_buffer_submit(cb);
    }

    
    //draw
    sglr_flush();

        
    sglr_blit_main_render_target(window_width, window_height, GL_NEAREST);
    
    gpu_time = sglr_end_query_time_elapsed_ns();
    cpu_time = platform_get_time_ns() - ns_frame_start;
    
    sglr_flush();
    platform_window_swap_buffers(window);

    // dt
    uint64_t ns_end = platform_get_time_ns();
    dt = platform_convert_ns_to_seconds(ns_end - ns_frame_start);
    ns_frame_start = ns_end;
    
    
    if(platform_convert_ns_to_seconds(ns_end - ns_start) >= 1){
      ns_start += second_to_ns;
      sprintf(fps_buffer, "%d", ns_fps);
      // printf("fps: %d\n", ns_fps);
      ns_fps = 0;          
    }
  }
  
  sglr_free_debug_logger();
  sglr_free_context(sglr_current_context());
  platform_free_glcontext(window);  
  platform_free_window(window);
    
  return 0;
}

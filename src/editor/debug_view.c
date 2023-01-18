#include "debug_view.h"

#include <stdio.h>

#include "sglr/src/sglr.h"
#include "n1_platform.h"

#define GPU_HISTORY_SAMPLE_COUNT (256)
int gpu_sample_at;
static float gpu_samples[GPU_HISTORY_SAMPLE_COUNT];

#define CPU_HISTORY_SAMPLE_COUNT (256)
int cpu_sample_at;
static float cpu_samples[CPU_HISTORY_SAMPLE_COUNT];

void update_samples(){
  gpu_samples[gpu_sample_at] = min((get_time_stats().gpu_time / 1000000.0f) / (1000.0 / 60.0), 1);
  gpu_sample_at = (gpu_sample_at + 1) % GPU_HISTORY_SAMPLE_COUNT;

  cpu_samples[cpu_sample_at] = min((get_time_stats().cpu_time / 1000000.0f) / (1000.0 / 60.0), 1);
  cpu_sample_at = (cpu_sample_at + 1) % CPU_HISTORY_SAMPLE_COUNT;
}

vec3 draw_graph(const char* title,
                vec3 cursor,
                float scale,
                sglr_ImmediateModeCmd* text_cb,
                sglr_ImmediateModeCmd* flat_cb,
                float graph_height,
                int sample_count,
                float samples[],
                int sample_offset){

  cursor.y -= graph_height;
  
  const uint32_t color      = 0xff323232;
  const float    line_width = 2 * scale;
  const vec3     n          = vec3_make(0, 0, 1);
  const float    text_scale = 1 * scale;
  const uint32_t text_color = ~0;//0xff64ccaa;
  const float    text_pad   = 2 * text_scale;
  const uint32_t thic  = 2 * scale;
  
  const vec3 bl = vec3_addv(cursor, vec3_make(-line_width, -line_width, 0));
  const vec3 tl = vec3_addv(cursor, vec3_make(-line_width, graph_height + line_width * 2, 0));
  const vec3 br = vec3_addv(cursor, vec3_make(256 + line_width, - line_width, 0));
  const vec3 tr = vec3_addv(cursor, vec3_make(256 + line_width, graph_height + line_width * 2, 0));
  
  sglr_immediate_line_2d(flat_cb, n, bl, color, tl, color, line_width); 
  sglr_immediate_line_2d(flat_cb, n, br, color, tr, color, line_width);
  sglr_immediate_line_2d(flat_cb, n, bl, color, br, color, line_width); 
  sglr_immediate_line_2d(flat_cb, n, tl, color, tr, color, line_width);
  
  sglr_immediate_text(text_cb,
                      title,
                      vec3_addv(cursor, vec3_make(text_scale * 1, graph_height - text_pad - text_scale * 8 - text_scale, 0)),
                      text_scale,
                      0xff000000);

  sglr_immediate_text(text_cb,
                      title,
                      vec3_addv(cursor, vec3_make(text_pad, graph_height - text_pad - text_scale * 8, 0)),
                      text_scale,
                      text_color);
  

  uint32_t color_0 = 0xffffffff;
  float    y_0     = 0;
  for(int i = 0; i < sample_count; i++){

    float    y_1     = samples[(sample_offset + i) % sample_count];
    uint32_t color_1 = 0xffffffff;


    if(y_1 <= 0.33f){
      color_1 = 0xff00ff00;
    }else if(y_1 <= 0.66f){
      color_1 = 0xff00ffff;
    }else if(y_1 <= 1.0f){
      color_1 = 0xff0000ff;
    }    
      
    
    y_1 *= graph_height;

    vec3 min = vec3_make(i , y_0, 0);
    vec3 max = vec3_make(i + 1, y_1 + thic , 0);
    uint32_t min_color = color_0;
    uint32_t max_color = color_1;


    
    sglr_immediate_quad_min_max(flat_cb,
                                vec3_addv(cursor, min), vec3_zero(), min_color,
                                vec3_addv(cursor, max), vec3_zero(), max_color);

    y_0     = y_1;
    color_0 = color_1;
  }

  
  return cursor;
}

void draw_debug_view(){
  update_samples();
  char buffer[1024] = {0};

  const int window_width = get_main_window()->width;
  const int window_height = get_main_window()->height;

  
  const float    dt       = get_dt();
  const int      fps      = get_time_stats().fps;
  const uint64_t gpu_time = get_time_stats().gpu_time;
  const uint64_t cpu_time = get_time_stats().cpu_time;
  
  // material for bitmap font
  sglr_Material text_mat = sglr_make_material(sglr_make_shader_builtin_text());
  sglr_set_material_sampler_i(&text_mat, 0, sglr_make_bitmap_font_builtin().texture);

  sglr_Camera cam   = sglr_make_camera();
  sglr_camera_set_ortho_rh(&cam,
                           0, window_width,
                           window_height, 0,
                           0.01, 10000);
               
  
  
  // draw to main render target
  sglr_CommandBuffer* cb = sglr_make_command_buffer();
  sglr_command_buffer_set_render_target(cb, sglr_main_render_target());

  // pipeline for drawing text
  sglr_GraphicsPipeline pipeline = sglr_make_graphics_pipeline_default(text_mat, GL_TRIANGLES);
  pipeline.renderer_state.flags = 0;
  
  // set camera
  sglr_CommandBuffer2* scb = sglr_make_command_buffer2();
  sglr_command_buffer2_add_cam(scb, cam);

  sglr_ImmediateModeCmd* cmd = sglr_immediate_begin(pipeline);
      

  float          scale       = 2;
  const float    pad_x       = 4 * scale;
  const float    pad_y       = 4 * scale;
  const float    text_height = sglr_text_height(scale);
  const uint32_t color       = 0xff64ccaa;
  vec3           cursor      = vec3_make(pad_x, window_height - pad_y - text_height, -1);

  //fps counter
  sprintf(buffer, "%d", fps);
  sglr_immediate_text(cmd, buffer,
                      cursor,
                      scale,
                      sglr_color_yellow);

  scale /= 2; // draw other text smaller
  
  // time info
  cursor.y -= text_height;
  
  sprintf(buffer,
          "frame time: %.2f ms\n"
          "gpu time:   %.2f ms\n"
          "cpu time:   %.2f ms\n"
          "draw calls: %d\n"
          "triangles:  %d",
          dt * 1000,
          gpu_time / 1000000.0f,
          cpu_time / 1000000.0f,
          sglr_stats_draw_call_count(),
          sglr_stats_triangle_count());

  
  vec2 text_size = sglr_immediate_text(cmd,
                                       buffer,
                                       cursor,
                                       scale,
                                       color);

  cursor.y += text_size.y;
  
  int32_t total = sglr_vram_total();
  int32_t avail = sglr_vram_avail();

  sglr_check_error();


#if defined (_MSC_VER)
  sprintf(buffer,
          "vram: %.0f%% (%d MB)\n"
          "ram:  %zd KB",
          (1 - avail / (float)total) * 100, total / 1024,
          get_process_mem_use());
#elif defined (__GNUC__)
  sprintf(buffer,
          "vram: %.0f%% (%d MB)\n"
          "ram:  %ld KB",
          (1 - avail / (float)total) * 100, total / 1024,
          get_process_mem_use());
#endif
  
  text_size = sglr_immediate_text(cmd,
                                  buffer,
                                  cursor,
                                  scale,
                                  color);

  cursor.y += text_size.y;


  extern sglr_Texture white_texture;
    
  sglr_Material mat = sglr_make_material(sglr_make_shader_builtin_flat());
  sglr_set_material_sampler_i(&mat, 0, white_texture);
    
  sglr_GraphicsPipeline pipeline2 = sglr_make_graphics_pipeline_default(mat, GL_TRIANGLES);
  pipeline2.renderer_state.flags = 0;

    
  sglr_CommandBuffer2* scb_2 = sglr_make_command_buffer2();
  sglr_command_buffer2_add_cam(scb_2, cam);
  
  sglr_ImmediateModeCmd* cmd_2 = sglr_immediate_begin(pipeline2);
  
  sglr_immediate_color(cmd_2, 0xff0000ff);

  const int graph_height = 20 * scale;
  const float offset = 2 * scale;

  cursor.y -= pad_y;
  cursor = draw_graph("gpu",
                      cursor,
                      scale,
                      cmd,
                      cmd_2,
                      graph_height,
                      GPU_HISTORY_SAMPLE_COUNT,
                      gpu_samples,
                      gpu_sample_at);

  cursor.y -= graph_height * 0.5 + pad_y;
  cursor = draw_graph("cpu",
                      cursor,
                      scale,
                      cmd,
                      cmd_2,
                      graph_height,
                      CPU_HISTORY_SAMPLE_COUNT,
                      cpu_samples,
                      cpu_sample_at);


  sglr_cmd_immediate_draw(scb_2, cmd_2);
  sglr_cmd_immediate_draw(scb, cmd);                         
  
  sglr_command_buffer2_submit(scb_2, cb);
  sglr_command_buffer2_submit(scb, cb);
  
  sglr_command_buffer_submit(cb);
}

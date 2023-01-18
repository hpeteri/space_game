#include "console.h"

#include "../core/core.h" //dt
#include "../input/input.h"

#include "sglr.h"

#include <stdio.h>

static Console console;
void make_console(){
  console.allocator = default_allocator();
  
  console.is_inited = 1;
  
  console.input_line_max = 256;
  console.input_line = (char*)console.allocator.alloc(console.input_line_max);

  console.input_line[0] = 0;
}

void free_console(){
  console.allocator.free(console.input_line);
  for(int i = 0; i < 256; i++){
    ConsoleString it = console.history[i];
    if(it.string){
      console.allocator.free(it.string);
    }
  }
}

void toggle_console(){

  if(!console.is_inited){
    make_console();
  }
  
  console.is_open = !console.is_open;
}

int is_console_open(){
  return console.is_open;
}

static void reset_console_blink(){
  console.blink = 0;
}
void update_and_draw_console(){

  if(!console.is_inited){
    make_console();
  }

  // === update ===
  const float dt = get_dt();
  const float speed = 1 * dt;

  if(!console.is_open){
    console.openness -= speed * 2;
    console.openness = max(console.openness, 0);
    
  }else{
    console.openness += speed * 5;
    console.openness = min(console.openness, 0.3);
  }
  
  console.blink += dt * PI;
  
  // === draw ===

  if(console.openness == 0){
    return;
  }
  
  //colors
  const uint32_t color_bg       = 0xaa323232;
  const uint32_t color_input_bg = 0xaa101010;
  const uint32_t color_text     = ~0;
  const uint32_t color_cursor  = sign(sin(console.blink)) == 1 ? 0xaaff75ae : 0x55000000;
  
  const float font_scale = 2.0f;
  const float font_height = abs(sglr_text_size("", font_scale).y);
  const float text_pad = font_scale;
  
  const int window_width  = get_main_window()->width;
  const int window_height = get_main_window()->height;

  
  // orthographic camera in pixel space
  sglr_Camera cam   = sglr_make_camera();
  sglr_camera_set_ortho_rh(&cam, 0, window_width, window_height, 0, 0.01, 10000);
  
  extern sglr_Texture white_texture;
  sglr_Material flat_mat = sglr_make_material(sglr_make_shader_builtin_flat());
  sglr_set_material_sampler_i(&flat_mat, 0, white_texture);
  
  sglr_Material text_mat = sglr_make_material(sglr_make_shader_builtin_text());
  sglr_set_material_sampler_i(&text_mat, 0, sglr_make_bitmap_font_builtin().texture);
  
  {
    
    // draw to main render target
    sglr_CommandBuffer* cb = sglr_make_command_buffer();
    sglr_command_buffer_set_render_target(cb, sglr_main_render_target());

    {
      // im pipeline 
      sglr_GraphicsPipeline pipeline = sglr_make_graphics_pipeline_default(flat_mat, GL_TRIANGLES);
      pipeline.renderer_state.flags = SGLR_CULL_FACE | SGLR_BLEND;
      
      sglr_CommandBuffer2* scb = sglr_make_command_buffer2();
      sglr_command_buffer2_add_cam(scb, cam);

      sglr_ImmediateModeCmd* cmd = sglr_immediate_begin(pipeline);
      
      
      vec3 p0 = vec3_make(0, (1 - console.openness) * window_height, -1);
      vec3 p1 = vec3_make(window_width, window_height, -1);

      // draw bg
      sglr_immediate_quad_min_max(cmd,
                                  p0, vec3_zero(), color_bg,
                                  p1, vec3_zero(), color_bg);
      // draw input bar bg
      
      p1.y = p0.y;
      p0.y = p1.y - font_height - text_pad * 2;
      
      sglr_immediate_quad_min_max(cmd,
                                  p0, vec3_zero(), color_input_bg,
                                  p1, vec3_zero(), color_input_bg);
      

      sglr_cmd_immediate_draw(scb, cmd);
      sglr_command_buffer2_submit(scb, cb);
    }
    {
      sglr_GraphicsPipeline pipeline = sglr_make_graphics_pipeline_default(text_mat, GL_TRIANGLES);
      pipeline.renderer_state.flags = 0;
      
      sglr_CommandBuffer2* scb = sglr_make_command_buffer2();
      sglr_command_buffer2_add_cam(scb, cam);

      sglr_ImmediateModeCmd* cmd = sglr_immediate_begin(pipeline);

      vec3 p0 = vec3_make(0, (1 - console.openness) * window_height - font_height, -1);
      
      p0.x += text_pad;
      p0.y -= text_pad;
      
      sglr_immediate_text(cmd,
                          console.input_line,
                          p0,
                          font_scale,
                          color_text);


      p0.y += text_pad;

      //draw history
      int i = (console.history_at + 255) % 256;
      for(;;){
        ConsoleString* it = console.history + i;
        if(!it->string){
          break;
        }
        
        p0.y -= it->size.y - text_pad * 2;

        sglr_immediate_text(cmd,
                            it->string,
                            p0,
                            font_scale,
                            color_text);

        
        i = (i  + 255) % 256;

        if(p0.y >= window_height)
          break;
      }


      sglr_cmd_immediate_draw(scb, cmd);
      sglr_command_buffer2_submit(scb, cb);
    }
    {
      //draw cursor blink
      // im pipeline 
      sglr_GraphicsPipeline pipeline = sglr_make_graphics_pipeline_default(flat_mat, GL_TRIANGLES);
      pipeline.renderer_state.flags = SGLR_CULL_FACE | SGLR_BLEND;
      
      sglr_CommandBuffer2* scb = sglr_make_command_buffer2();
      sglr_command_buffer2_add_cam(scb, cam);

      sglr_ImmediateModeCmd* cmd = sglr_immediate_begin(pipeline);

      
      vec3 p0 = vec3_make(0, (1 - console.openness) * window_height - font_height, -1);
      
      p0.x += text_pad;
      p0.y -= text_pad;
      p0.x += sglr_text_size_n(console.input_line, console.input_line_at, 2).x;

      vec3 p1 = vec3_make(p0.x + font_height, p0.y + font_height, p0.y);
      
      // draw cursor
      sglr_immediate_quad_min_max(cmd,
                                  p0, vec3_zero(), color_cursor,
                                  p1, vec3_zero(), color_cursor);

      sglr_cmd_immediate_draw(scb, cmd);
      sglr_command_buffer2_submit(scb, cb);      
    }
    sglr_command_buffer_submit(cb);
  }
  
}

static int32_t console_get_codepoint_size(uint32_t keycode){
  int len = 0;

  char* at = (char*)&keycode;
  char character = *at;
  
  // utf-8 to codepoint
  if((character & 0b11110000) == 0b11110000){
    len = 4;
  }else if((character & 0b11100000) == 0b11100000){
    len = 3;
  }else if((character & 0b110000000) == 0b110000000){
    len = 2;
  }else if((character & 0b000000000) == 0b000000000){
    len = 1;
  }
  
  return len;
}

void console_push_control_keycode(uint32_t keycode){
  if(!console.is_inited){
    make_console();
  }

  if(!console.is_open){
    return;
  }

  
  switch(keycode){
  case KC_LEFT:
    // left arrow
    // move cursor to left
    while(console.input_line_at){
      
      char* at = console.input_line + console.input_line_at - 1;
      const uint8_t c = *(uint8_t*)at;

      int is_continue_code = (c & 0b10000000) == 0b10000000;
      
      console.input_line_at --;
      
      if(!is_continue_code){
        break;
      }else if(c & 0b01000000){
        break;
      }      
    }
    break;
  case KC_RIGHT:
    {
      // right arrow
      // move cursor to right
      uint8_t c = *(console.input_line + console.input_line_at);
      if(!c){
        break;
      }
      
      int len = console_get_codepoint_size(c);
      console.input_line_at += len;

      break;
    }
  case KC_BACK:
    {
      // backspace
      // remove character from input line
      int len = 0;
      while(console.input_line_at){
        char* at = console.input_line + console.input_line_at - 1;
      
        const uint8_t c = *(uint8_t*)at;

        int is_continue_code = (c & 0b10000000) == 0b10000000;
        *at = 0;
        console.input_line_at --;
        len ++;
        if(!is_continue_code){
          break;
        }else if(c & 0b01000000){
          break;
        }      
      }
    
      memmove(console.input_line + console.input_line_at,
              console.input_line + console.input_line_at + len,
              console.input_line_len - console.input_line_at + len + 1);

      console.input_line_len -= len;
      break;
    }
  case KC_RETURN:
    {
      // enter
      // push new string to history
      if(console.input_line_at){
        ConsoleString* it = console.history + console.history_at;
      
        //free old string
        if(it->string){
          console.allocator.free(it->string);
        }

        //copy string
        it->string = (char*)console.allocator.alloc(console.input_line_at + 1);
        memcpy(it->string, console.input_line, console.input_line_at + 1);

        it->size = sglr_text_size(it->string, 2);
      
        console.history_at = (console.history_at + 1) % 256;
    
        console.input_line[0] = 0;
        console.input_line_at = 0;
    
        int i = (console.history_at + 255) % 256;
        for(;;){
          ConsoleString* it = console.history + i;
          if(!it->string){
            break;
          }
      
          i = (i  + 255) % 256;
        }
      }
      break;
    }
  default:
    // just so we don't reset blink for unrecogniced input
    return;
  }

  reset_console_blink();
  
}

void console_push_keycode(uint32_t keycode){
  if(!console.is_inited){
    make_console();
  }

  if(!console.is_open){
    return;
  }
  
  if(keycode < 32){
    return;
  }

  uint32_t s = 0;
  int len = 0;
  if(keycode < 128){
    s = keycode;
    len = 1;
  }else{
    
    if(keycode < 0xff){
      uint8_t* dst = (uint8_t*)&s;
      dst[0] = 0b11000000 | (keycode >> 6);
      dst[1] = 0b10000000 | (keycode & 0b00111111);

      len = 2;
    }else{
      //larger codepoint, should add at some point
      return;
    }
  }  

  if(len == 0){
    return;
  }

  reset_console_blink();
  
  //ensure enough space for char
  if(console.input_line_len >= console.input_line_max - 1 - len){
    console.input_line_max = max(console.input_line_max, len);
    console.input_line_max *= 2;
    console.input_line = (char*)console.allocator.realloc(console.input_line, console.input_line_max);
  }

  memmove(console.input_line + console.input_line_at + len,
          console.input_line + console.input_line_at,
          console.input_line_len - console.input_line_at + 1);
  
  memcpy(console.input_line + console.input_line_at,
         &s,
         len);

  console.input_line_at += len;
  console.input_line_len += len;
}

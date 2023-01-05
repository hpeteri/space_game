#include "editor.h"

#include "../input/input.h"
#include "../core/core.h"
#include "../console/console.h"

#include "debug_view.h"

static Editor editor;

void make_editor(){
  editor.cam = sglr_make_camera();
  editor.secondary_cam = sglr_make_camera();

  
  editor.zoom = 50;
}

void do_editor_camera(){
  
  if(is_console_open()){
    return;
  }

  if(input_is_key_up(KC_TAB)){
    
    sglr_Camera temp = editor.cam;
    editor.cam = editor.secondary_cam;
    editor.secondary_cam = temp;
    
    editor.is_ortho = !editor.is_ortho;
    update_editor_camera(get_main_window()->width,
                         get_main_window()->height);
    
  }
  
  const float dt = get_dt();

  const float speed = 5 * dt;
  const float rot_speed = PI;
  const float zoom_speed = 20 * dt;
  

  // move
  
  vec3 delta = vec3_zero();

  if(editor.cam.is_ortho){
    if(input_is_key_held('W')){ delta.y += speed * 50 / editor.zoom; }
    if(input_is_key_held('S')){ delta.y -= speed * 50 / editor.zoom; }
    if(input_is_key_held('A')){ delta.x -= speed * 50 / editor.zoom; }
    if(input_is_key_held('D')){ delta.x += speed * 50 / editor.zoom; }

    int update_camera = 0;
    if(input_is_key_held('Q')){
      editor.zoom -= zoom_speed;
      update_camera = 1;
    }
    if(input_is_key_held('E')){
      editor.zoom += zoom_speed;
      update_camera = 1;      
    }
    if(update_camera){
      editor.zoom = max(editor.zoom, 1);
    }
  }else{
    if(input_is_key_held('W')){ delta.z -= speed; }
    if(input_is_key_held('S')){ delta.z += speed; }
    if(input_is_key_held('A')){ delta.x -= speed; }
    if(input_is_key_held('D')){ delta.x += speed; }
    if(input_is_key_held('Q')){ delta.y -= speed; }
    if(input_is_key_held('E')){ delta.y += speed; }
    
  }
  
  sglr_camera_move(&editor.cam, quat_mulv(sglr_camera_rot(editor.cam), delta));
  update_editor_camera(get_main_window()->width,
                       get_main_window()->height);
  
  // rotate

  if(input_is_right_mouse_held()){

    vec3 euler = editor.cam.euler;

    vec3 mouse_delta = vec3_make(input_mouse_delta().x, input_mouse_delta().y, 0);
        
    euler.x += mouse_delta.y * rot_speed; 
    euler.y -= mouse_delta.x * rot_speed; 

    sglr_camera_set_euler(&editor.cam, euler);
  }
  
}


void update_editor_camera(int width, int height){

  if(editor.is_ortho){
    
    sglr_camera_set_ortho_rh(&editor.cam,
                             -width / editor.zoom, width / editor.zoom,
                             height / editor.zoom, height / -editor.zoom,
                             1,
                             100);

  }else{
    sglr_camera_set_perspective_rh(&editor.cam,
                                   45,
                                   width / (float)height,
                                   1,
                                   100);
    
    sglr_camera_update_frustrum_aabb(&editor.cam);
  }
}
sglr_Camera editor_camera(){
  

  return editor.cam;
}
sglr_Camera editor_camera_perspective(){
  if(editor.cam.is_ortho){
    return editor.secondary_cam;
  }
  return editor.cam;
}
sglr_Camera editor_camera_ortho(){
  if(!editor.cam.is_ortho){
    return editor.secondary_cam;
  }
  return editor.cam;  
}

void draw_editor(){
  draw_debug_view();
}

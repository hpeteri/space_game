#ifndef EDITOR_H
#define EDITOR_H

#include "sglr.h"
typedef struct Editor{
  int is_ortho;
  
  sglr_Camera cam;
  sglr_Camera secondary_cam;

  float zoom; //orthographic scale
} Editor;

void        make_editor();
void        do_editor_camera();

sglr_Camera editor_camera();

sglr_Camera editor_camera_perspective();
sglr_Camera editor_camera_ortho();

void update_editor_camera(int width, int height);
void draw_editor();

#endif

#version 330

uniform mat4 cam_proj;

layout(location = 0) in vec3 vert_pos;
layout(location = 10) in mat4 model;

void main(){
  gl_Position = cam_proj * model * vec4(vert_pos, 1);
}

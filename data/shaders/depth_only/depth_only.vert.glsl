#version 460
   
layout(location = 0) in vec3 vert_pos;
layout(location = 10) in mat4 model;

void main(){
  gl_Position = model * vec4(vert_pos, 1);
}
  

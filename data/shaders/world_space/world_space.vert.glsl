#version 330

uniform mat4 cam_proj;
   
layout(location = 0) in vec3 vert_pos;
layout(location = 10) in mat4 model;
   
out vec4 pos_w;
void main(){
  pos_w = model * vec4(vert_pos, 1);
  gl_Position = cam_proj * pos_w;

  //store clipspace in w
  pos_w.w = gl_Position.z;
}

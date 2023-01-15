#version 330 

layout(location = 0) in vec3 vert_pos;
layout(location = 1) in vec3 vert_tc;

out vec2 frag_tc;

void main(){
  frag_tc   = vert_tc.xy;
  gl_Position = vec4(vert_pos, 1);
}

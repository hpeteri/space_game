#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 36) out;

layout(std140) uniform cam_info{
  int camera_count;
  
  ivec4 render_layers[12 / 4];
  mat4  cam_projections[12];
};

void main(){

  const int cam_count = 3;

  for(int i = 0; i < cam_count; i++){
    if(i >= camera_count)
      break;

    int y = i / 4;
    int x = i % 4;
    int layer_i = render_layers[y][x];
    
    gl_Layer    = layer_i;
    gl_Position = cam_projections[i] * gl_in[0].gl_Position;
    EmitVertex();

    gl_Layer    = layer_i;
    gl_Position = cam_projections[i] * gl_in[1].gl_Position;
    EmitVertex();
  
    gl_Layer    = layer_i;
    gl_Position = cam_projections[i] * gl_in[2].gl_Position;
    EmitVertex(); 

  }
  
}


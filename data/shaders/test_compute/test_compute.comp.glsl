#version 460

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D texture0;

void main(){

  vec4 pixel = vec4(gl_LocalInvocationID / 32.0, 1);
  ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
  imageStore(texture0, pixel_coords, pixel);
}

#include "platform/src/n1_platform.h"

sglr_Shader pbr_shader(){
  static sglr_Shader shader;

  if(!shader.id){
    FileContent vert = read_entire_file("data/shaders/pbr/pbr.vert.glsl", default_allocator());
    FileContent frag = read_entire_file("data/shaders/pbr/pbr.frag.glsl", default_allocator());
    
    shader = sglr_make_shader(vert.data,
                              NULL,
                              NULL,
                              NULL,
                              frag.data);
    
    default_allocator().free(vert.data);
    default_allocator().free(frag.data);
  }
  return shader;
  
}

sglr_Shader z_prepass_shader(){
  static sglr_Shader shader;

  if(!shader.id){
    FileContent vert = read_entire_file("data/shaders/z_prepass/z_prepass.vert.glsl", default_allocator());
    FileContent frag = read_entire_file("data/shaders/z_prepass/z_prepass.frag.glsl", default_allocator());
    
    shader = sglr_make_shader(vert.data,
                              NULL,
                              NULL,
                              NULL,
                              frag.data);
    default_allocator().free(vert.data);
    default_allocator().free(frag.data);
  }
  return shader;
  
}

sglr_Shader depth_only_shader(){
  static sglr_Shader shader;

  if(!shader.id){
    FileContent vert = read_entire_file("data/shaders/depth_only/depth_only.vert.glsl", default_allocator());
    FileContent frag = read_entire_file("data/shaders/depth_only/depth_only.frag.glsl", default_allocator());
    FileContent geom = read_entire_file("data/shaders/depth_only/depth_only.geom.glsl", default_allocator());
    
    shader = sglr_make_shader(vert.data,
                              NULL,
                              NULL,
                              geom.data,
                              frag.data);
    
    default_allocator().free(vert.data);
    default_allocator().free(geom.data);
    default_allocator().free(frag.data);
     
  }
  return shader;
  
}


sglr_Shader shadow_pass_shader(){
  static sglr_Shader shader;

  if(!shader.id){
    FileContent vert = read_entire_file("data/shaders/shadow_pass/shadow_pass.vert.glsl", default_allocator());
    FileContent frag = read_entire_file("data/shaders/shadow_pass/shadow_pass.frag.glsl", default_allocator());
    
    shader = sglr_make_shader(vert.data,
                              NULL,
                              NULL,
                              NULL,
                              frag.data);
    
    default_allocator().free(vert.data);
    default_allocator().free(frag.data);
     
  }
  return shader;
  
}

sglr_Shader world_space_shader(){
  static sglr_Shader shader;

  if(!shader.id){
    FileContent vert = read_entire_file("data/shaders/world_space/world_space.vert.glsl", default_allocator());
    FileContent frag = read_entire_file("data/shaders/world_space/world_space.frag.glsl", default_allocator());
    
    shader = sglr_make_shader(vert.data,
                              NULL,
                              NULL,
                              NULL,
                              frag.data);
    
    default_allocator().free(vert.data);
    default_allocator().free(frag.data);
     
  }
  return shader;
  
}

#version 330
   
uniform sampler2D      texture_0;
uniform sampler2DArray texture_1;

struct PointLight{
  vec3  pos;
  int   color;
  float intensity;
};

struct DirLight{
  mat4  projections[4];
  vec3  direction;
  int   color;
  float intensity;
};

uniform interface_block_0{
  float ambient;
  int point_light_count;
  int spot_light_count;
  int dir_light_count;
  PointLight point_lights[256];
  DirLight dir_lights[1];
};

in vec2 frag_tc;
out vec4 out_color;

void main(){
  vec4 tex_lookup_0 =  texture(texture_0, frag_tc);
  vec3 frag_pos_world = tex_lookup_0.xyz;

  
  float depth = tex_lookup_0.w / 100.0;

  int cascade_idx = 1;
  if(depth < 0.33){
    cascade_idx = 0;
    out_color.rgb = vec3(1, 0.2, 0.2);
  }else if(depth < 0.66){
    cascade_idx = 1;
    out_color.rgb = vec3(0.2, 1, 0.2);
  }else if(depth < 1){
    cascade_idx = 2;
    out_color.rgb = vec3(0.2 ,0.2,1);
  }else{
    cascade_idx = 3;
    out_color.rgb = vec3(0, 0, 0);
  }
  
  const float bias = 0.01;
  vec2 texel_size = 1.0 / textureSize(texture_1, 0).xy;

  int i = 0;
  DirLight dir_light = dir_lights[i];
    
  float shadow = 0;


#if 1

  vec4 frag_pos_light;
  if(cascade_idx == 0){   
    frag_pos_light = dir_light.projections[0] * vec4(frag_pos_world, 1);
  }else if(cascade_idx == 1){
    frag_pos_light = dir_light.projections[1] * vec4(frag_pos_world, 1);
  }else{
    frag_pos_light = dir_light.projections[2] * vec4(frag_pos_world, 1);
  }
  
#else

  vec4 frag_pos_light = dir_light.projections[cascade_idx] * vec4(frag_pos_world, 1);

#endif
  
  vec3 light_coords = frag_pos_light.xyz / frag_pos_light.w;

  #if 1
  if(light_coords.z <= 1.0f){
    light_coords = (light_coords + 1) / 2;
    
    float depth_1 = light_coords.z;
    
#if 1
    float depth_0 = 0;
    if(cascade_idx == 0){   
      frag_pos_light = dir_light.projections[0] * vec4(frag_pos_world, 1);
      depth_0 = texture(texture_1, vec3(light_coords.xy, 0)).r;
    }else if(cascade_idx == 1){
      frag_pos_light = dir_light.projections[1] * vec4(frag_pos_world, 1);
      depth_0 = texture(texture_1, vec3(light_coords.xy, 1)).r;
    }else{
      frag_pos_light = dir_light.projections[2] * vec4(frag_pos_world, 1);
      depth_0 = texture(texture_1, vec3(light_coords.xy, 2)).r;
    }
#else
    float depth_0 = texture(texture_1, vec3(light_coords.xy, cascade_idx)).r;
#endif
    
    if(depth_1 - bias > depth_0){
      shadow += 1;
    }
  }
  #endif

  out_color *= (1 - shadow);
}

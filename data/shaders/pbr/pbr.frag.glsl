#version 460

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
  DirLight dir_lights[256];
};
  
in vec3 frag_pos;
in vec3 frag_tc;
in vec4 frag_color;
in vec3 frag_norm;
in float clip_space_z;
  
in vec3 frag_pos_world;

out vec4 out_color;
  
void main(){
  
  float depth = clip_space_z / 100.0;

  out_color = texture(texture_0, frag_tc.xy) * frag_color;
  
  // calculate cascade index for shadow maps
  int cascade_idx = 1;
  
  if(depth < 0.33){
    cascade_idx = 0;
    out_color.rgb = mix(out_color.rgb, vec3(1, 0.2, 0.2), 0.75);
  }else if(depth < 0.66){
    cascade_idx = 1;
    out_color.rgb = mix(out_color.rgb, vec3(0.2, 1, 0.2), 0.75);
  }else{
    cascade_idx = 2;
    out_color.rgb = mix(out_color.rgb, vec3(0.2 ,0.2, 1), 0.75);
  }  
    
  vec3 diffuse = vec3(0);

  /* ==== point lights ====*/
#if 0
  
  for(int i = 0; i < point_light_count; i++){
    
    PointLight point_light = point_lights[i];
    vec3  light_pos = point_light.pos;
    vec3  light_dir = light_pos - frag_pos;
    float diff      = max(dot(frag_norm, normalize(light_dir)), 0.0);
    int   c         = point_light.color;
    float intensity = point_light.intensity / pow(length(light_dir), 2);

    diffuse += diff * vec3((c & 0xff) / 255.0f, ((c >> 8 & 0xff)) / 255.0f, ((c >> 16) & 0xff) / 255.0f) * intensity;

  }
  
#endif
  /* ==== dir lights ====*/
#if 1
  
  float bias = 0.01;
  vec2 texel_size = 1.0 / textureSize(texture_1, 0).xy;
  
  for(int i = 0; i < dir_light_count; i++){
    DirLight dir_light = dir_lights[i];
    
    float shadow = 0;
  
  
#if 1
           
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
    
    if(light_coords.z <= 1.0f){
      light_coords = (light_coords + 1) / 2;
  
      for(int y = -1; y <= 1; y++){
        for(int x = -1; x <= 1; x++){
  
          vec2 offset = vec2(x, y) * texel_size;
          
          float depth_1 = light_coords.z;

          
#if 0
    float depth_0 = 0;
    if(cascade_idx == 0){   
      frag_pos_light = dir_light.projections[0] * vec4(frag_pos_world, 1);
      depth_0 = texture(texture_1, vec3(light_coords.xy + offset, 0)).r;
    }else if(cascade_idx == 1){
      frag_pos_light = dir_light.projections[1] * vec4(frag_pos_world, 1);
      depth_0 = texture(texture_1, vec3(light_coords.xy + offset, 1)).r;
    }else{
      frag_pos_light = dir_light.projections[2] * vec4(frag_pos_world, 1);
      depth_0 = texture(texture_1, vec3(light_coords.xy + offset, 2)).r;
    }
#else
    float depth_0 = texture(texture_1, vec3(light_coords.xy + offset, cascade_idx)).r;
#endif
          
          if(depth_1 - bias > depth_0){
            shadow += 1 / 9.0f;
          }
        }
      }
    }

#endif
    
    float diff = max(dot(frag_norm, -dir_light.direction), 0.0) * (1.0 - shadow);
    int c = dir_light.color;
    float intensity = dir_light.intensity;
    diffuse += diff * vec3((c & 0xff) / 255.0f, ((c >> 8 & 0xff)) / 255.0f, ((c >> 16) & 0xff) / 255.0f) * intensity;
  }

  #endif
  
  //float ambient = 0.01;
  out_color.rgb = (ambient + diffuse) * out_color.rgb;
  
  float gamma = 2.2;
  out_color.rgb = pow(out_color.rgb, vec3(1.0 / gamma));
  
}
  

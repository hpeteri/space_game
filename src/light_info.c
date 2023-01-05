#include "light_info.h"

LightInfo make_light_info(){
  LightInfo info;
  N1_ZERO_MEMORY(&info);

  info.ambient = 0.01f;

  return info;
}

void add_point_light(LightInfo* info, PointLight light){
  if(info->point_light_count < MAX_POINT_LIGHT_COUNT){
    info->point_lights[info->point_light_count++] = light;
  }
}
void add_spot_light(LightInfo* info, SpotLight light){
 if(info->spot_light_count < MAX_SPOT_LIGHT_COUNT){
   info->spot_lights[info->spot_light_count++] = light;
 }
}
void add_dir_light(LightInfo* info, DirLight light){
 if(info->dir_light_count < MAX_DIR_LIGHT_COUNT){
   info->dir_lights[info->dir_light_count++] = light;
 }
}

PointLight make_point_light(vec3 pos, uint32_t color, float intensity){
  PointLight light =
    {
    .pos = pos,
    .color = color,
    .intensity = intensity
    };
  
  return light;
}

DirLight make_dir_light(vec3 dir, uint32_t color, float intensity){
  DirLight light =
    {
      .direction = vec3_normalize(dir),
      .color = color,
      .intensity = intensity
    };

  return light;
}

#ifndef LIGHT_INFO_H
#define LIGHT_INFO_H

#include <stdint.h>
#include "cmath/n1_cmath.h"

#define MAX_DIR_LIGHT_COUNT   (256)
#define MAX_POINT_LIGHT_COUNT (256)
#define MAX_SPOT_LIGHT_COUNT  (256)

typedef struct PointLight{
  vec3     pos;
  uint32_t color;
  float    intensity;
  int32_t  pad_0;
  int32_t  pad_1;
  int32_t  pad_2;
} PointLight;

typedef struct SpotLight{
  vec3 pos;
  vec3 direction;
  uint32_t color;
  
  float intensity;
  float fov;
  
} SpotLight;

typedef struct DirLight{
  mat4     projections[4];
  vec3     direction;
  uint32_t color;
  float    intensity;
  int32_t  pad_0;
  int32_t  pad_1;
  int32_t  pad_2;
} DirLight;

typedef struct LightInfo{
  float    ambient;

  uint32_t point_light_count;
  uint32_t spot_light_count;
  uint32_t dir_light_count;
  
  PointLight point_lights[MAX_POINT_LIGHT_COUNT];
  DirLight   dir_lights[MAX_DIR_LIGHT_COUNT];
  SpotLight  spot_lights[MAX_SPOT_LIGHT_COUNT];
} LightInfo;

LightInfo make_light_info();

void add_point_light(LightInfo* info, PointLight light);
void add_spot_light(LightInfo* info, SpotLight light);
void add_dir_light(LightInfo* info, DirLight light);

PointLight make_point_light(vec3 pos, uint32_t color, float intensity);
DirLight make_dir_light(vec3 dir, uint32_t color, float intensity);
#endif

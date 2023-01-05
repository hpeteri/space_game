#ifndef TWEAKS_H
#define TWEAKS_H

typedef struct StartupTweaks{

  int window_width;
  int window_height;
} StartupTweaks;

typedef struct GraphicsTweaks{
  
  int vsync;
  int shadow_quality;
} GraphicsTweaks;

typedef struct Tweaks{
  
  StartupTweaks  startup;
  GraphicsTweaks graphics;
} Tweaks;


int load_tweak_file();
Tweaks get_tweaks();

#endif

#define SL_STRING_IMPLEMENTATION
#include "sl_string.h"
#include "platform/src/n1_platform.h"
#include <stdio.h>

#include "tweak.h"

static int variable_names_inited;

static Tweaks tweaks_;

enum TWEAK_VARIABLE_TYPE{
  TWEAK_VARIABLE_INVALID = 0,
  TWEAK_VARIABLE_INT,
  TWEAK_VARIABLE_FLOAT,
};

struct TweakVariable{
  const char*              name;
  int                      offset;
  int                      size;
  enum TWEAK_VARIABLE_TYPE type;
};

struct TweakFolder{
  const char* name;
  void*       data;

  int           variable_count;
  struct TweakVariable variables[256];
};

static int tweak_folder_count;
static struct TweakFolder tweak_folders[256];

#define BEGIN_TWEAK_FOLDER(folder, ptr) struct TweakFolder tweak_folder_ ## folder = {sl_make_string(#folder), ptr, 0, NULL};
#define ATTACH_VARIABLE(folder, strct, variable, type) {struct TweakVariable variable_ ## variable = {sl_make_string(#variable), offsetof(strct, variable), sizeof(((strct*)0)->variable), type}; tweak_folder_ ## folder.variables[tweak_folder_##folder.variable_count++] = variable_ ## variable;}
#define END_TWEAK_FOLDER(folder) tweak_folders[tweak_folder_count ++] = tweak_folder_ ## folder;

static void bind_variable_names(){
  variable_names_inited = 1;

  // === startup ===
  
  BEGIN_TWEAK_FOLDER(startup, &tweaks_.startup);
  ATTACH_VARIABLE(startup, struct StartupTweaks, window_width, TWEAK_VARIABLE_INT);
  ATTACH_VARIABLE(startup, struct StartupTweaks, window_height, TWEAK_VARIABLE_INT);
  END_TWEAK_FOLDER(startup);

  // === graphics ===

  BEGIN_TWEAK_FOLDER(graphics, &tweaks_.graphics);
  ATTACH_VARIABLE(graphics, struct GraphicsTweaks, vsync, TWEAK_VARIABLE_INT);
  ATTACH_VARIABLE(graphics, struct GraphicsTweaks, shadow_quality, TWEAK_VARIABLE_INT);
  END_TWEAK_FOLDER(graphics);

  printf("%d folders total\n", tweak_folder_count);

  for(int i = 0; i < tweak_folder_count; i++){
    struct TweakFolder folder = tweak_folders[i];

    printf("folder: '%s' has %d variables\n", folder.name, folder.variable_count);
    for(int ii = 0; ii < folder.variable_count; ii++){
      struct TweakVariable var = folder.variables[ii];
      printf("[%d]: '%s' (offset: %d, size: %d)\n", ii, var.name, var.offset, var.size);
    }
  }
  printf("===========================\n");
}

int load_tweak_file(){
  if(!variable_names_inited){
    bind_variable_names();
  }
    
  FileContent file = read_entire_file("options.variables", default_allocator());

  struct TweakFolder* current_folder = NULL;
    
  if(!file.data){
    perror("failed to 'options.variables': ");
    return 1;
  }


  const char* start = file.data;
  const char* end   = sl_next_line(start);

  int linum = 1;

  while(*start){
    start = sl_eat_whitespace(start);
      
    int len = end - start - 1;
    if(len > 1){
      if(start[0] == '.'){
        //folder
        if(start[1] == '/'){
          int found = 0;
          for(int i = 0; i < tweak_folder_count; i++){
            struct TweakFolder* folder = tweak_folders + i;

            //check if folder is the same
            if(sl_strlen(folder->name) != (size_t)len  - 2){
              continue;
            }
            if(memcmp(folder->name, start + 2, len - 2)){
              continue;
            }

            found = 1;
            current_folder = folder;
            break;
          }
            
          // not found, set folder to NULL
          if(!found){

            printf("error at line %d: failed to find folder '%.*s'\n", linum, len - 2, start + 2);
            current_folder = NULL;
          }
          
        }else{

          printf("error at line %d: line starts with '.', but second symbol is not '/' (got '%c').\n", linum, start[1]);
        }
        
      }else{
        //variable
        
        if(!current_folder){
          printf("error at line %d: folder not set\n", linum);
        }
        
        const char* name_end = sl_next_token(start, ' ');
                
        int found = 0;
        int variable_name_len = name_end - start;
        
        struct TweakVariable* variable = NULL;
        for(int i = 0; i < current_folder->variable_count; i++){
          variable = current_folder->variables + i;

          //check if variable is the same
          if(sl_strlen(variable->name) != (size_t)variable_name_len){
            continue;
          }
          if(memcmp(variable->name, start, variable_name_len)){
            continue;
          }
          
          found = 1;
          break;
        }
            
        // not found, set variable to NULL
        if(!found){
          printf("error at line %d: failed to find variable '%.*s'\n", linum, variable_name_len, start);
        }else{
        
          const char* var_start = sl_eat_whitespace(name_end);
          const char* var_end = end;
          
          switch(variable->type){
          case TWEAK_VARIABLE_INT:
            {
              if(sl_is_digit(*var_start)){
                int64_t value = sl_atoi(var_start);
                memcpy((char*)current_folder->data + variable->offset, &value, variable->size);
                
              }else{
                printf("error at line %d: failed to parse '%.*s' as int\n", linum, (int)(var_end - var_start) - 1, var_start);
              }
              break;
            }
          case TWEAK_VARIABLE_FLOAT:
            {
              if(sl_is_digit(*var_start)){
                double value = sl_atof(var_start);
                memcpy((char*)current_folder->data + variable->offset, &value, variable->size);
              }else{
                printf("error at line %d: failed to parse '%.*s' as float\n", linum, (int)(var_end - var_start) - 1, var_start);
              }
              break;
            }
          default:
            printf("error at line %d: don't know how to parse type\n", linum);
          }
        }
      }
        
    }

    start = end;
    end   = sl_next_line(start);

    linum ++;
  }

  printf("=== startup ===\n");
  printf("width: %d\n", tweaks_.startup.window_width);
  printf("height: %d\n", tweaks_.startup.window_height);

  printf("=== graphics === \n");
  printf("vsync: %d\n", tweaks_.graphics.vsync);
  printf("shadow_quality: %d\n", tweaks_.graphics.shadow_quality);

  printf("=== end === \n\n");
  
  free(file.data);
  return 0;
}

Tweaks get_tweaks(){
  return tweaks_;
}

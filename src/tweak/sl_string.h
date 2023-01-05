#ifndef SL_STRING_H
#define SL_STRING_H

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

const char* sl_make_string(const char* c_str);
const char* sl_copy_string(const char* sl_str);
const char* sl_format_string(const char* format, ...);

void sl_free_string(const char* sl_str);

size_t sl_strlen(const char* sl_str);
int    sl_is_whitespace(char c);
int    sl_is_equal(const char* sl_str0, const char* sl_str1);
int    sl_is_digit(char c);

double  sl_atof(const char* str);
int32_t sl_atoi(const char* str);

const char* sl_next_token(const char* str, char token);
const char* sl_next_line(const char* str);
const char* sl_eat_whitespace(const char* str);

#if defined SL_STRING_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct sl_header{
  uint32_t len;
};

static struct sl_header* sl_get_header(const char* sl_str){
  struct sl_header* header = (struct sl_header*)sl_str;
  
  return header - 1;
}

const char* sl_make_string(const char* c_str){
  size_t len = strlen(c_str);
  
  struct sl_header* header = malloc(sizeof(struct sl_header) + len + 1);
  char* str = (char*)(header + 1);
  
  header->len = len;
  memcpy(str, c_str, len + 1);
    
  return str;
}

const char* sl_copy_string(const char* sl_str){
  
  struct sl_header* header = sl_get_header(sl_str);

  size_t size = sizeof(struct sl_header) + header->len + 1;

  struct sl_header* copy = malloc(size);
  memcpy(copy, header, size);

  const char* str = (char*)(copy + 1);
  return str;
}

static const char* sl_format_string2(const char* format, va_list args){

  //calculate string size
  va_list args_count;
  va_copy(args_count, args);
  
  size_t len = vsnprintf(NULL, 0, format, args_count);
  va_end(args_count);

  // malloc and create string
  struct sl_header* header = malloc(sizeof(struct sl_header) + len + 1);
  header->len = len;

  
  vsnprintf((char*)(header + 1) , len + 1, format, args);
  
  va_end(args);

  return (char*)(header + 1);
}

const char* sl_format_string(const char* format, ...){
  
  va_list args;
  va_start(args, format);
  
  const char* str = sl_format_string2(format, args);
  
  va_end(args);
  return str;
}

void sl_free_string(const char* sl_str){
  struct sl_header* header = sl_get_header(sl_str);
  free(header);
}

size_t sl_strlen(const char* sl_str){
  struct sl_header* header = sl_get_header(sl_str);
  return header->len;
}

int sl_is_whitespace(char c){
  if(c == ' '  ) return 1;
  if(c == '\t' ) return 1;
  if(c == '\n' ) return 1;
  if(c == '\v' ) return 1;
  if(c == '\f' ) return 1;
  if(c == '\r' ) return 1;
  
  return 0;
}

int sl_is_digit(char c){
  if(c >= '0' && c <= '9'){
    return 1;
  }
  return 0;
}
int sl_is_equal(const char* sl_str0, const char* sl_str1){
  struct sl_header* a = sl_get_header(sl_str0);
  struct sl_header* b = sl_get_header(sl_str1);

  if(a->len != b->len)
    return 0;
  
  for(uint32_t i = 0; i < a->len; i++){
    if(sl_str0[i] != sl_str1[i])
      return 0;
  }
  
  return 1;  
}

double sl_atof(const char* str){
  double value = 0;
  value = atof(str);

  return value;
}

int32_t sl_atoi(const char* str){
  int32_t value = 0;
  value = atoi(str);

  return value;
}

const char* sl_next_token(const char* str, char token){
  const char* at = str;

  if(!*at)
    return at;

  for(at++;*at && *at != token; at++);
  
  return at;
}

const char* sl_next_line(const char* str){
  const char* at = str;

  if(!*at)
    return at;

  for(; *at ; at++){
    if(*at == '\n'){
      at ++;
      break;
    }
  }  
  
  return at;
}

const char* sl_eat_whitespace(const char* str){
  const char* at = str;

  if(!*at)
    return at;
  
  for(at;*at && sl_is_whitespace(*at); at++);
  
  return at;

}

#endif
#endif

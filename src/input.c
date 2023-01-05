typedef struct Input{
  char keyboard[256];

  vec2 mouse_pos; //(-1 to 1)
  vec2 mouse_delta;
  union{
    struct {
      int left_mouse;
      int middle_mouse;
      int right_mouse;
    };
    int mouse_buttons[3];
  };
  
} Input;

static Input input_;

void input_new_frame(){
  input_.mouse_delta = vec2_zero();
}
void input_set_mouse_pos(float x, float y, int width, int height){
  input_.mouse_delta = input_.mouse_pos;
  
  input_.mouse_pos = vec2_make((x - width / 2.0) / (float)width,
                               (y - height / 2.0) / -(float)height);

  input_.mouse_delta = vec2_subv(input_.mouse_pos, input_.mouse_delta);
}
void input_set_mouse_button(int button, int is_down){
  input_.mouse_buttons[button - 1] = is_down;
}

vec2 input_mouse_pos(){
  return input_.mouse_pos;
}
vec2 input_mouse_delta(){
  return input_.mouse_delta;
}

int input_is_left_mouse_down(){
  return input_.left_mouse;
}

int input_is_middle_mouse_down(){
  return input_.middle_mouse;
}

int input_is_right_mouse_down(){
  return input_.right_mouse;
}


void input_update_key(char key, int is_down){
  if(key < 256 && key >= 0)
    input_.keyboard[key] = is_down;
}

int input_is_key_down(char key){
  if(key < 256)
    return input_.keyboard[key];

  return 0;
}

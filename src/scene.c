#include "editor/editor.h"

#include "light_info.h"
#include "light_info.c"

#include "temp_shader_loading.h"

float rot_z = HALF_PI / 2.0f;

static sglr_RenderTarget shadow_cascade_0;
static sglr_RenderTarget deferred_shadows;

vec3                     sun_pos;

sglr_Camera              light_cam_0;
sglr_Camera              light_cam_1;
sglr_Camera              light_cam_2;

static void fit_shadow_frustum(){
  
  sglr_Camera editor_camera = editor_camera_perspective();
    
  sglr_camera_set_pos(&light_cam_0, vec3_zero());
  sglr_camera_set_rot(&light_cam_0, quat_look_rot(sun_pos, vec3_make(0, 1, 0))); 
  
  float fov_x = editor_camera.perspective.fov / 180 * PI / 2;
  float fov_y = editor_camera.perspective.fov / editor_camera.aspect / 180 * PI / 2;
  
  float height_f = tan(fov_y) * editor_camera.fa;
  float width_f  = tan(fov_x) * editor_camera.fa;
  float height_n = tan(fov_y) * editor_camera.ne; 
  float width_n  = tan(fov_x) * editor_camera.ne;

  float aspect = editor_camera.aspect;

  // === fa ====
  vec3 points[8];
  
  points[0] = quat_mulv(quat_inverse(light_cam_0.rotation), vec3_addv(editor_camera.pos, quat_mulv(editor_camera.rotation, vec3_make(-width_f,  height_f, -editor_camera.fa ))));
  points[1] = quat_mulv(quat_inverse(light_cam_0.rotation), vec3_addv(editor_camera.pos, quat_mulv(editor_camera.rotation, vec3_make( width_f,  height_f, -editor_camera.fa ))));
  points[2] = quat_mulv(quat_inverse(light_cam_0.rotation), vec3_addv(editor_camera.pos, quat_mulv(editor_camera.rotation, vec3_make(-width_f, -height_f, -editor_camera.fa ))));
  points[3] = quat_mulv(quat_inverse(light_cam_0.rotation), vec3_addv(editor_camera.pos, quat_mulv(editor_camera.rotation, vec3_make( width_f, -height_f, -editor_camera.fa ))));
  points[4] = quat_mulv(quat_inverse(light_cam_0.rotation), vec3_addv(editor_camera.pos, quat_mulv(editor_camera.rotation, vec3_make(-width_n,  height_n, -editor_camera.ne))));
  points[5] = quat_mulv(quat_inverse(light_cam_0.rotation), vec3_addv(editor_camera.pos, quat_mulv(editor_camera.rotation, vec3_make( width_n,  height_n, -editor_camera.ne))));
  points[6] = quat_mulv(quat_inverse(light_cam_0.rotation), vec3_addv(editor_camera.pos, quat_mulv(editor_camera.rotation, vec3_make(-width_n, -height_n, -editor_camera.ne))));
  points[7] = quat_mulv(quat_inverse(light_cam_0.rotation), vec3_addv(editor_camera.pos, quat_mulv(editor_camera.rotation, vec3_make( width_n, -height_n, -editor_camera.ne))));

  vec3 min = points[0];
  vec3 max = points[0];

  for(int i = 1; i < 8; i++){
    min = vec3_minv(min, points[i]);
    max = vec3_maxv(max, points[i]);
  }
  
  min = vec3_make(floor(min.x), floor(min.y), floor(min.z));
  max = vec3_make(ceil(max.x), ceil(max.y), ceil(max.z));
    
  sglr_camera_set_ortho_rh(&light_cam_0,
                           min.x - 10, max.x + 10, max.y + 10, min.y - 10, min.z - 100, max.z + 100);
  
  light_cam_1 = light_cam_0;
  light_cam_2 = light_cam_0;
}

static sglr_Buffer test_create_lights(){

  sun_pos = vec3_make(cos(rot_z), -1, sin(rot_z));
  rot_z += get_dt() * PI * 0.1f;

  // ========================================

  LightInfo lights = make_light_info();
  lights.ambient = 0.01;
  
  fit_shadow_frustum();
  
  {
    for(int i = 0; i < 1; i++){
      add_dir_light(&lights, make_dir_light(sun_pos, sglr_inverse_gamma(0xffffff, 2.2), 0.5));
      
      lights.dir_lights[i].projections[0] =  sglr_camera_matrix(light_cam_0);
      lights.dir_lights[i].projections[1] =  sglr_camera_matrix(light_cam_1);
      lights.dir_lights[i].projections[2] =  sglr_camera_matrix(light_cam_2);
    }
  }
  
  static sglr_Buffer light_buffer;
  static sglr_Buffer transfer_buffer;
  if(!light_buffer.id){

    transfer_buffer = sglr_make_buffer_transfer(sizeof(lights), &lights);
    light_buffer = sglr_make_buffer_uniform(sizeof(lights), &lights);
  }else{

    sglr_fill_buffer(transfer_buffer, sizeof(lights), &lights);
    sglr_copy_buffer(transfer_buffer, light_buffer, 0, 0, sizeof(lights));
    //sglr_fill_buffer(light_buffer, sizeof(lights), &lights);
  }
  
  return light_buffer;  
}

static void do_compute_shadow_pass(sglr_CommandBuffer* command_buffer){
  static sglr_Shader compute;
  static sglr_Texture test_texture;
  
  if(!compute.id){
    FileContent file = read_entire_file("data/shaders/test_compute/test_compute.comp.glsl", default_allocator());

    compute = sglr_make_shader_compute(file.data);        
    default_allocator().free(file.data);

    test_texture = sglr_make_texture_2d(1024, 1024,
                                        GL_RGBA32F,
                                        GL_RGBA,
                                        GL_UNSIGNED_BYTE,
                                        NULL);
        
    sglr_set_texture_debug_name(test_texture, "compute_test_texture");
          
  }

  sglr_Material mat = sglr_make_material(compute);
  sglr_set_material_image_i(&mat, 0, test_texture);
                                  
  sglr_ComputePipeline pipeline = sglr_make_compute_pipeline(mat);
  sglr_CommandBuffer2* secondary_command_buffer = sglr_make_command_buffer2();

  sglr_cmd_set_compute_pipeline(secondary_command_buffer, pipeline);
  
  sglr_cmd_compute_dispatch(secondary_command_buffer,
                            32, 32, 1);
  
  sglr_command_buffer2_submit(secondary_command_buffer,
                              command_buffer);
  
}
void draw_scene(){
  if(!shadow_cascade_0.id){
    shadow_cascade_0 = sglr_make_render_target_layered(1024, 1024, 1,
                                                       GL_NONE,
                                                       GL_DEPTH_COMPONENT32);
    
  }

  
  sglr_set_render_target(shadow_cascade_0);
  sglr_set_clear_depth(1.0f);
  sglr_clear_render_target_depth(shadow_cascade_0);

    
  extern sglr_Texture missing_texture;
  sglr_Camera cam = editor_camera();

  sglr_Buffer light_buffer = test_create_lights();

  sglr_CommandBuffer* cmd_buf_prepass = sglr_make_command_buffer();
  sglr_command_buffer_set_render_target(cmd_buf_prepass, sglr_main_render_target());
  
  
  sglr_CommandBuffer* cb_0 = sglr_make_command_buffer();
  sglr_command_buffer_set_render_target(cb_0, sglr_main_render_target());

  
  sglr_CommandBuffer* cb_1 = sglr_make_command_buffer();
  sglr_command_buffer_set_render_target(cb_1, shadow_cascade_0);

  
  {
    // @NOTE
    // if we could submit same command buffer multible times, we would have less command buffer calls.
    
    sglr_CommandBuffer2* scb_shadow_cascades;
    sglr_CommandBuffer2* scb_prepass;
    sglr_CommandBuffer2* scb_main;

    sglr_ImmediateModeCmd* cmd_shadow_cascades;
    sglr_ImmediateModeCmd* cmd_prepass;
    sglr_ImmediateModeCmd* cmd_main;

    
    {
      // shadow cascade setup
      sglr_Material         mat_shadow      = sglr_make_material(depth_only_shader());
      sglr_GraphicsPipeline pipeline_shadow = sglr_make_graphics_pipeline_default(mat_shadow, GL_TRIANGLES);
      pipeline_shadow.renderer_state.flags  = SGLR_CULL_FACE |  SGLR_DEPTH_TEST;
      pipeline_shadow.cull_mode             = GL_BACK;
      pipeline_shadow.depth_mode.mask       = GL_TRUE;

      scb_shadow_cascades = sglr_make_command_buffer2();
      cmd_shadow_cascades = sglr_immediate_begin(pipeline_shadow);

      sglr_command_buffer2_add_cam_to_layer(scb_shadow_cascades, light_cam_0, 0);
      sglr_command_buffer2_add_cam_to_layer(scb_shadow_cascades, light_cam_1, 1);
      sglr_command_buffer2_add_cam_to_layer(scb_shadow_cascades, light_cam_2, 2);
      
      sglr_command_buffer2_set_draw_layer(scb_shadow_cascades, 0);      
    }
    {
      // prepass setup
      sglr_Material z_prepass = sglr_make_material(world_space_shader());
      sglr_GraphicsPipeline pipeline_1 = sglr_make_graphics_pipeline_default(z_prepass, GL_TRIANGLES);
      pipeline_1.renderer_state.flags = SGLR_CULL_FACE | SGLR_DEPTH_TEST;
            
      scb_prepass = sglr_make_command_buffer2();
      cmd_prepass = sglr_immediate_begin(pipeline_1);
            
      sglr_command_buffer2_add_cam(scb_prepass, cam);
      sglr_command_buffer2_set_draw_layer(scb_prepass, 1);
    }

    {
      //normal pass setup
      sglr_Material mat = sglr_make_material(pbr_shader());
      sglr_set_material_sampler_i(&mat, 0, missing_texture);
      sglr_set_material_sampler_i_2(&mat, 1, GL_TEXTURE_2D_ARRAY, shadow_cascade_0.depth_attachment.id);
      sglr_set_material_buffer_i(&mat, 0, light_buffer);
    
      sglr_GraphicsPipeline pipeline_0 = sglr_make_graphics_pipeline_default(mat, GL_TRIANGLES);
      pipeline_0.renderer_state.flags |= SGLR_CULL_FACE;
      pipeline_0.depth_mode.func = GL_EQUAL;
      pipeline_0.depth_mode.mask = GL_FALSE;
    
      scb_main = sglr_make_command_buffer2();
      cmd_main = sglr_immediate_begin(pipeline_0);

      
      sglr_command_buffer2_add_cam(scb_main, cam);
    }
        
    #if 1
    for(int z = -1; z < 5; z++){
      for(int y = -1; y < 5; y++){
        for(int x = -1; x < 5; x++){
          if(!x && !y && !z)
            continue;

          mat4 model = mat4_trs(vec3_make(x * 5, y * 5, z * 5),
                                quat_make_identity(),
                                vec3_make1(3));          

          
          sglr_immediate_mesh(cmd_shadow_cascades,
                              sglr_make_mesh_primitive_cube(),
                              model);

           sglr_immediate_mesh(cmd_prepass, 
                               sglr_make_mesh_primitive_cube(), 
                               model); 
          
           sglr_immediate_mesh(cmd_main,
                              sglr_make_mesh_primitive_cube(),
                              model);
        }
      }
    }
    #endif
    
    if(1){ //floor
      mat4 model = mat4_trs(vec3_make(0, -10, 0),
                            quat_make_identity(),
                            vec3_make(100, 1, 100));

      sglr_immediate_mesh(cmd_shadow_cascades,
                          sglr_make_mesh_primitive_cube(),
                          model);

      sglr_immediate_mesh(cmd_prepass, 
                          sglr_make_mesh_primitive_cube(), 
                          model); 
                    
      sglr_immediate_mesh(cmd_main,
                          sglr_make_mesh_primitive_cube(),
                          model);


    }

    // draw camera pos
    mat4 model = mat4_trs(editor_camera().pos,
                          quat_make_identity(),
                          vec3_make1(1));           

    if(editor_camera().is_ortho){
      sglr_immediate_mesh(cmd_main,
                          sglr_make_mesh_primitive_cube(),
                          model);
    }
    
    sglr_immediate_mesh(cmd_shadow_cascades,
                        sglr_make_mesh_primitive_cube(),
                        model);



    sglr_cmd_immediate_draw(scb_shadow_cascades, cmd_shadow_cascades);
    sglr_cmd_immediate_draw(scb_prepass, cmd_prepass);
    sglr_cmd_immediate_draw(scb_main, cmd_main);
    
    sglr_command_buffer2_submit(scb_prepass, cb_0);
    do_compute_shadow_pass(cb_0);
    sglr_command_buffer2_submit(scb_main, cb_0);
    sglr_command_buffer2_submit(scb_shadow_cascades, cb_1);
    
  }
  

  #if 0
  {
    //lines
    sglr_Material flat = sglr_make_material(sglr_make_shader_builtin_flat());
        
    sglr_GraphicsPipeline pipeline = sglr_make_graphics_pipeline_default(flat, GL_TRIANGLES);
    pipeline.renderer_state.flags ^= SGLR_CULL_FACE;
    
    sglr_CommandBuffer2* scb = sglr_make_command_buffer2_im(pipeline);
    
    sglr_command_buffer2_add_cam(scb, cam);
        
    float size = 1;

    //x
    sglr_immediate_line_2d(scb,
                           vec3_make(0, 1, 0), 
                           vec3_make(0,0, 0),
                           0xff0000ff,
                           vec3_make(size, 0, 0),
                           0xff0000ff,
                           size / 10.0f);
        
    //y
    sglr_immediate_line_2d(scb,
                           vec3_make(0, 0, 1), 
                           vec3_make(0, 0, 0),
                           0xff00ff00,
                           vec3_make(0, size, 0),
                           0xff00ff00,
                           size / 10.0f);

    //z
    sglr_immediate_line_2d(scb,
                           vec3_make(0, 1, 0), 
                           vec3_make(0, 0, 0),
                           0xffff0000,
                           vec3_make(0, 0, size),
                           0xffff0000,
                           size / 10.0f);
        
    { //draw camera frustrum;
      sglr_Camera ecam = editor_camera_perspective();

 
      if(editor_camera().is_ortho){
        sglr_immediate_aabb_outline(scb,
                                    ecam.perspective.aabb.min,
                                    ecam.perspective.aabb.max,
                                    0xff00ffff,
                                    size / 10.0f);
    
    
        vec3 forward = vec3_make(0, 0, -ecam.fa);
        sglr_immediate_line_3d(scb,
                               vec3_make(0, 1, 0), 
                               ecam.pos,
                               0xffffffff,
                               vec3_addv(ecam.pos, quat_mulv(ecam.rotation, forward)),
                               0xffffffff,
                               size / 10.0f);


        float height = tan(ecam.perspective.fov / ecam.aspect / 180 * PI / 2) * ecam.fa;
        float width = tan(ecam.perspective.fov / 180 * PI / 2) * ecam.fa;
        
        vec3 left   = vec3_make(-width, 0, -ecam.fa);
        vec3 right  = vec3_make(width, 0, -ecam.fa);
        vec3 top    = vec3_make(0, height, -ecam.fa);
        vec3 bottom = vec3_make(0, -height, -ecam.fa);

        vec3 up = vec3_make(0, 1, 0);
        float t = size / 10.0f;
        
        // === fa ====
        
        vec3 top_left_f     = vec3_addv(ecam.pos, quat_mulv(ecam.rotation, vec3_make(-width, height, -ecam.fa)));
        vec3 top_right_f    = vec3_addv(ecam.pos, quat_mulv(ecam.rotation, vec3_make(width, height, -ecam.fa)));
        vec3 bottom_left_f  = vec3_addv(ecam.pos, quat_mulv(ecam.rotation, vec3_make(-width, -height, -ecam.fa)));
        vec3 bottom_right_f = vec3_addv(ecam.pos, quat_mulv(ecam.rotation, vec3_make(width, -height, -ecam.fa)));

        height = tan(ecam.perspective.fov / ecam.aspect / 180 * PI / 2) * ecam.ne;
        width = tan(ecam.perspective.fov / 180 * PI / 2) * ecam.ne;
        
        vec3 top_left_n     = vec3_addv(ecam.pos, quat_mulv(ecam.rotation, vec3_make(-width, height, -ecam.ne)));
        vec3 top_right_n    = vec3_addv(ecam.pos, quat_mulv(ecam.rotation, vec3_make(width, height, -ecam.ne)));
        vec3 bottom_left_n  = vec3_addv(ecam.pos, quat_mulv(ecam.rotation, vec3_make(-width, -height, -ecam.ne)));
        vec3 bottom_right_n = vec3_addv(ecam.pos, quat_mulv(ecam.rotation, vec3_make(width, -height, -ecam.ne)));
        
        sglr_immediate_line_3d(scb, up, top_left_f,     ~0, top_right_f   , ~0, t);
        sglr_immediate_line_3d(scb, up, bottom_left_f,  ~0, bottom_right_f, ~0, t);
        sglr_immediate_line_3d(scb, up, bottom_left_f,  ~0, top_left_f    , ~0, t);
        sglr_immediate_line_3d(scb, up, bottom_right_f, ~0, top_right_f   , ~0, t);

        sglr_immediate_line_3d(scb, up, top_left_n,     ~0, top_right_n   , ~0, t);
        sglr_immediate_line_3d(scb, up, bottom_left_n,  ~0, bottom_right_n, ~0, t);
        sglr_immediate_line_3d(scb, up, bottom_left_n,  ~0, top_left_n    , ~0, t);
        sglr_immediate_line_3d(scb, up, bottom_right_n, ~0, top_right_n   , ~0, t);
        
        sglr_immediate_line_3d(scb, up, top_left_n,     ~0, top_left_f    , ~0, t);
        sglr_immediate_line_3d(scb, up, top_right_n,    ~0, top_right_f   , ~0, t);
        sglr_immediate_line_3d(scb, up, bottom_left_n,  ~0, bottom_left_f , ~0, t);
        sglr_immediate_line_3d(scb, up, bottom_right_n, ~0, bottom_right_f, ~0, t);
        
        
        sglr_immediate_color(scb, ~0);
        sglr_immediate_mesh(scb,
                            sglr_make_mesh_primitive_cube(),
                            mat4_translatev(ecam.pos));
        
        
      }
    }
    sglr_command_buffer2_submit(scb, cb_0);
  }

  #endif
  
  sglr_command_buffer_submit(cb_1);
  sglr_command_buffer_submit(cb_0);
  
}

void draw_scene_simple(){
  sglr_Camera cam = editor_camera();


  sglr_CommandBuffer* cb_0 = sglr_make_command_buffer();
  sglr_command_buffer_set_render_target(cb_0, sglr_main_render_target());
  
  sglr_CommandBuffer2* scb_main;
  sglr_ImmediateModeCmd* cmd_main;

  //normal pass setup
  sglr_Material mat = sglr_make_material(sglr_make_shader_builtin_flat());
  
  
  sglr_GraphicsPipeline pipeline_0 = sglr_make_graphics_pipeline_default(mat, GL_TRIANGLES);
  pipeline_0.renderer_state.flags |= SGLR_CULL_FACE;
  pipeline_0.depth_mode.func = GL_LESS;
  pipeline_0.depth_mode.mask = GL_FALSE;
    
  scb_main = sglr_make_command_buffer2();
  cmd_main = sglr_immediate_begin(pipeline_0);

  sglr_command_buffer2_add_cam(scb_main, cam);
  
  for(int z = -1; z < 5; z++){
    for(int y = -1; y < 5; y++){
      for(int x = -1; x < 5; x++){
        if(!x && !y && !z)
          continue;

        mat4 model = mat4_trs(vec3_make(x * 5, y * 5, z * 5),
                              quat_make_identity(),
                              vec3_make1(3));          
          
        sglr_immediate_mesh(cmd_main,
                            sglr_make_mesh_primitive_cube(),
                            model);
      }
    }
  }

  sglr_cmd_immediate_draw(scb_main, cmd_main);
  sglr_command_buffer2_submit(scb_main, cb_0);
  sglr_command_buffer_submit(cb_0);
  
}

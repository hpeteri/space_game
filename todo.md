* Graphics
  - Particle emitters
    - GPU :)
    - indirect draw
  
  - Normal maps, tesselation maps

  - Mesh Loading
    - obj parser
    - gltf parser

  - Lights and shadows
    - shadow map should be an array texture so we can use cascade index to sample from the texture directly
    - HOW DO WE USE ARRAY TEXTURE AS A RENDER TARGET

    - ShadowPass:
      For each light:
        render this command buffer
    
    - calculate cascaded frustum bounds    
    - add cascade_count into light info
    - add cascade_projections into light info
    - shadowmap clamp to edge
    - use different texture slots for cascade levels for now.
    - calculate cascades
    - add cascade steps into light info
    - add farZ into camera params
 
  - mesh aabb (RenderBounds)
  - mesh culling  
  - mesh lod
  - shader_define_variable() / #include directive
  - hotloader
  - set_texture and set_sampler should be different
  - command buffer sorting
  - compute barrier
  - remove light info from sglr, use buffer_0 or something similar to texture
  - add posibility to cache im buffers / use same buffer for other 
 
* command line arguments
  
*  Console:
  - engine settings
   + shadow_map_quality
    
* Input:
  + controller support into platform
  + mappings don't work, shift, '
    
* Logging
  - Debug log to file + console
  
* ECS / Scene
  - lights
  
* Physics

* Editor
  - Draw icons for lights
   
* Platform
  - glcontext doesn't pick visual. just selects the first one. Should study how this should be done.
  - controller support
  - audio 
  - when mapping input keys, ' gets mapped to right arrow

Shadow mapping in deferred

- immediate:
  - calculate if in shadow
    * world space to light space
    * texture lookups

- Deferred shadow + immediate light
  - single texture lookup if in shadow !
  - need surface normal in order to calculate color intensity
  - less branching in the shader
  - separate compute step or cpu task
  - not that much memory since shadows can be stored in r8 texture
  - filtering is also simpler
  - less uniform data passed to shaders, less bandwidth use




Z_Prepass:
  - depth
  - world position


Shadow compute:
  Z_Prepass > world position

  light info
  write to shadow texture


Main pass
  Shadow Compute
  
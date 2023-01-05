* AMD profiler

  
* Graphics
  - Particle emitters
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
    - light info should be part of sglr_, since there is basic lighting shader

       
  - z prepass
  - mesh aabb (RenderBounds)
  - mesh culling  
  - color macros
  - shader loading from file
  - shader_define_variable()
  - hotloader :)
  
* command line arguments
  
*  Console:
  - engine settings
   + shadow_map_quality
    
* Input:
  + controller support into platform
  + mappings don't work, shift, '
    
* Logging
  - Debug log

* Profiling
  - graph of cpu and gpu times
  - graph memory use

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
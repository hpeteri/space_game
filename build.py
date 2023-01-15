#!/usr/bin/env python3

import time
start_time = time.time()

project_name     = "space_game"
output_name      = f"{project_name}"
cc               = ""
preprocessor     = ""
warnings         = ""
include_folders  = ""
src              = ""
compiler_options = ""
libs             = ""
linker_options   = ""

import os
import sys

def set_src():

    global src

    
    src += """
    ./src/unity_build.c
    """
    
    #src+="""
    #./dependencies/platform/src/n1_platform_main.c 
    #./dependencies/sglr/src/sglr.c                 
    #./src/main.c              
    #./src/editor/editor_main.c 
    #./src/console/console.c    
    #./src/input/input.c        
    #./src/tweak/tweak.c        
    #"""
    
def set_command_line_options():
    set_src()
    
    global cc
    global output_name
    global preprocessor    
    global warnings        
    global include_folders 
    global compiler_options
    global libs           
    global linker_options

    
    if sys.platform == "linux":
        output_name = output_name + ".a"
        cc          = "gcc"
    elif sys.platform == "win32":
        output_name = output_name + ".exe"
        cc          = "cl"
        
    preprocessor     = ""
    warnings         = ""
    include_folders  = ""
    src              = ""
    compiler_options = ""
    libs             = ""
    linker_options   = ""


    
    if "release" in sys.argv:
        preprocessor +=  "-D RELEASE_BUILD "
    else:
        preprocessor += "-D DEBUG_BUILD "

    if cc == "gcc":
        if sys.platform == "linux":
            libs += """
            -lX11
            -pthread
            -lGL
            -lm
            ./dependencies/GLEW/lib/libGLEW.a
            """    
        elif sys.platform == "win32":
            libs += """
            -lGdi32 
            -lUser32 
            -lShlwapi 
            -lOpengl32  
            -lKernel32 
            ./dependencies/GLEW/lib/glew32s.lib 
            """

        if "release" in sys.argv:
            compiler_options += "-O3"
            preprocessor += "-s"
        else:
            compiler_options += "-O0"
            preprocessor += "-g"
            
        warnings += """
        -Wformat=2 
        -Wmain 
        -Wparentheses 
        -Wuninitialized
        -Wsign-compare 
        -Werror
        """
            
        include_folders += """
        -I ./src/
        -I ./dependencies/
        -I ./dependencies/GLEW/include/
        -I ./dependencies/platform/src/
        -I ./dependencies/cmath/
        -I ./dependencies/sglr/src/
        """

        linker_options += f"""-o ./build/{output_name}"""
            
    elif cc == "cl":
        if sys.platform == "win32":
            preprocessor     += ""

            warnings += """
            /WX
            """
            
            include_folders += """
            -I ./src/
            -I ./dependencies/
            -I ./dependencies/GLEW/include/
            -I ./dependencies/platform/src/
            -I ./dependencies/cmath/
            -I ./dependencies/sglr/src/
            """
            compiler_options += """ 
            /nologo 
            /Fo./build/obj/ 
            -O2 
            """
            
            libs += """
            Gdi32.lib
            User32.lib
            Shlwapi.lib
            Opengl32.lib
            ./dependencies/GLEW/lib/glew32s.lib
            """
            
            linker_options += f"""/link /OUT:./build/{output_name} /PDB:./build/vc140.pdb"""
    else:
        print("unknown cc");
    
def main():

    # === set working directory ===
    os.chdir(os.path.dirname(os.path.realpath(__file__)))
    cwd = os.getcwd()

    # === create build dirs ===
    if not os.path.isdir("build"):
        os.makedirs("build", 0o777)

    if not os.path.isdir("build/obj"):
        os.makedirs("build/obj/", 0o777)
    
    # === build and link ===
    command = f"{cc} {compiler_options} {warnings} {preprocessor} {include_folders} {src} {libs} {linker_options}".replace('\n', ' ');
    command = " ".join(command.split())
    
    print(command);
    os.system(command);
    
    print_exit_time()

def print_exit_time():
    global start_time
    end_time = time.time()
    print("time: {:.2f}s".format(end_time - start_time))

if __name__ == "__main__":
    set_command_line_options()
    main()


# Rae Ray
A small beginning of a ray tracer inspired by/copied from Peter Shirley's minibooks

![ScreenShot](https://cloud.githubusercontent.com/assets/6566641/16933706/ae8202a8-4d58-11e6-9008-33fd87df6dd8.jpeg)

# Features

- Ray tracing spheres
- Depth of field
- Cumulative rendering. Image gets less noisier over time.
- Interactive moveable camera (Second mouse button + WASDQE)
- Three material types (Lambertian, Metal, Dielectric)

Source code is found under "src/rae". 

# Build instructions

    # on Linux (use premake4.4beta, not 4.3 which comes with Ubuntu 16.04):
    premake4 gmake
    # I had to install the GL headers:
    sudo apt-get install libgl1-mesa-dev libglu1-mesa-dev libassimp-dev
    # Also had to install some X-libs:
    sudo apt-get install libxrandr-dev libxinerama-dev libxcursor-dev
    # I installed GLFW3 and GLEW from source, and copied the libs to the lib_linux dir,
    # and I also copied libassimp.so to the lib_linux dir, but you might not need to. 
    # And finally:
    make
    # cd into the bin directory and run:
    ./rae_ray

    # on OSX:
    premake4 xcode4
    # Open the project file and build it.
	
    # on Windows:
    premake4 vs2012
    # Open the project file and build it.
    # You may need to retarget it to vs2013 or vs2015 after opening it


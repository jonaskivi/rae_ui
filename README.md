# Rae UI
WIP on a component based UI system, which happens to use NanoVG.

For some reason currently contains also a raytracer...
![ScreenShot](https://cloud.githubusercontent.com/assets/6566641/16933706/ae8202a8-4d58-11e6-9008-33fd87df6dd8.jpeg)

Source code is found under "src/rae".

# Build instructions

    # Get the source code by cloning it to your computer. You also need to clone the submodules,
    # so you'll need to run something like this:
    git clone --recurse-submodules https://github.com/jonaskivi/rae_ui.git

    # If you already cloned the repository and you are missing the submodules, run:
    git submodule update --init --recursive
    # and only after that run premake.

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
    ./pihlaja

    # on OSX:
    premake4 xcode4
    # Open the project file and build it.

    # on Windows:
    premake4 vs2012
    # Open the project file and build it.
    # You may need to retarget it to vs2013 or vs2015 after opening it and sometimes need to restore (checkout)
    # the libs as they are built wrong.

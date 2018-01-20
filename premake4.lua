#!lua

-- XCode build note:
-- install Cuda SDK (tested with 8.0)
-- install OpenCV with Homebrew: (I guess also needs some call to enable science packages.)

-- To make sure that ffmpeg is built with LGPL licence
-- brew install ffmpeg --with-sdl2 --with-openh264 --without-x264 --without-xvid --without-lame

-- brew install opencv3 --with-contrib --with-tbb --with-ffmpeg --c++11
-- And maybe not --with-cuda
-- (contrib is the extra modules, which contains DeepFlow. tbb is presumably Thread Building Blocks library.)
-- And you still have to add to Xcode rpath (Runpath Search Paths) manually:
-- /usr/local/opt/opencv3/lib/
-- /usr/local/opt/ffmpeg/lib/

-- Needed to add to Runpath Search Paths manually in Xcode project settings:
-- @loader_path/../Libraries /usr/local/opt/opencv3/lib/
-- or
-- @loader_path/../Libraries @loader_path/../Libraries/opencv3

-- A solution contains projects, and defines the available configurations
solution "pihlaja"
   configurations { "Debug", "Release" }
   platforms {"native", "x64", "x32"}

   -- A project defines one build target
   project "pihlaja"
      kind "ConsoleApp"
      language "C++"
      targetdir "bin/"
      files
      {
         "src/pihlaja/**.hpp",
         "src/pihlaja/**.cpp",
         "src/rae/**.hpp",
         "src/rae/**.cpp",
         "src/rae_av/**.hpp",
         "src/rae_av/**.cpp",
         "src/rae_ray/**.hpp",
         "src/rae_ray/**.cpp"
      }
      includedirs
      {
         "external/glew/include",
         "external/glfw/include",
         "external/nanovg/src",
         "external/glm",
         "external/glm/glm",
         "src/",
         "src/rae",
         "src/rae_av",
         "src/rae_ray",
         "external/" }
      links
      {
         "glfw3", "glew", "nanovg", "assimp",
      }
      defines { "GLEW_STATIC", "NANOVG_GLEW" }

      configuration { "linux" }
         buildoptions { "-std=c++11" }
         links {"X11","Xrandr", "Xxf86vm", "Xinerama", "Xcursor", "rt", "GL", "GLU", "pthread"}
       
      configuration { "windows" }
         links {"glu32","opengl32", "gdi32", "winmm", "user32"}

      configuration { "macosx" }
         buildoptions { "-std=c++11 -stdlib=libc++" }
         includedirs {
                        "/usr/local/opt/opencv3/include/",
                        "/usr/local/opt/ffmpeg/include/",
                     }
         defines { "USE_RAE_AV" }
         links
         {
            --"opencv_calib3d",
            "opencv_core",
            "opencv_imgcodecs",
            "opencv_highgui",
            "opencv_imgproc",
            --"opencv_flann",
            "opencv_features2d",
            "opencv_video",
            --"opencv_nonfree",
            --"opencv_ml",
            ---- "opencv_cudaarithm",
            ---- "opencv_cudaoptflow",
            -- opencv contrib stuff:
            "opencv_optflow",
            -- ffmpeg stuff:
            "avcodec",
            "avformat",
            "swscale",
         }
         libdirs
         {
             --"@loader_path/../Libraries",
             --"@loader_path/../Libraries/opencv3",
             "../Libraries/",
             --"../Libraries/opencv3/",
             "/usr/local/opt/",
             "/usr/local/opt/opencv3/lib/",
             "/usr/local/opt/ffmpeg/lib/",
         }
         linkoptions { "-stdlib=libc++", "-framework OpenGL", "-framework Cocoa", "-framework IOKit", "-framework CoreVideo" }

      configuration "Debug"
         defines { "DEBUG" }
         flags { "Symbols" }
         debugdir "../bin/"

      configuration "Release"
         defines { "NDEBUG" }
         flags { "Optimize" }
         debugdir "../bin/"

   -- GLFW Library
   project "glfw3"
      kind "StaticLib"
      language "C"
      targetdir "lib"
      files { "external/glfw/lib/*.h", "external/glfw/lib/*.c", "external/glfw/include/GL/glfw.h" }
      includedirs { "external/glfw/lib", "external/glfw/include"}

      configuration {"linux"}
         files { "external/glfw/lib/x11/*.c", "external/glfw/x11/*.h" }
         includedirs { "external/glfw/lib/x11" }
         targetdir "lib"
         defines { "_GLFW_X11", "_GLFW_USE_LINUX_JOYSTICKS", "_GLFW_HAS_XRANDR", "_GLFW_HAS_PTHREAD" ,"_GLFW_HAS_SCHED_YIELD", "_GLFW_HAS_GLXGETPROCADDRESS" }
         buildoptions { "-pthread" }
       
      configuration {"windows"}
         files { "external/glfw/src/*.c", "external/glfw/src/*.h" }
         includedirs { "external/glfw/src" }
         defines { "_GLFW_EGL", "_GLFW_WIN32", "_GLFW_USE_OPENGL" }
       
      configuration {"Macosx"}
         files { "external/glfw/lib/cocoa/*.c", "external/glfw/lib/cocoa/*.h", "external/glfw/lib/cocoa/*.m" }
         includedirs { "external/glfw/lib/cocoa" }
         targetdir "Libraries"
         defines { "_GLFW_COCOA" }
         buildoptions { " -fno-common" }
         linkoptions { "-framework OpenGL", "-framework Cocoa", "-framework IOKit" }

      configuration "Debug"
         defines { "DEBUG" }
         flags { "Symbols", "ExtraWarnings" }

      configuration "Release"
         defines { "NDEBUG" }
         flags { "Optimize", "ExtraWarnings" }    

   -- GLEW Library         
   project "glew"
      kind "StaticLib"
      language "C"
      targetdir "lib"
      files {"external/glew/*.c", "external/glew/*.h"}
      defines { "GLEW_STATIC" }

      configuration {"linux"}
         targetdir "lib"

      configuration {"Macosx"}
         targetdir "Libraries"

      configuration "Debug"
         defines { "DEBUG" }
         flags { "Symbols", "ExtraWarnings" }

      configuration "Release"
         defines { "NDEBUG" }
         flags { "Optimize", "ExtraWarnings" }

   -- NanoVG Library
   project "nanovg"
      language "C"
      kind "StaticLib"
      includedirs { "external/nanovg/src" }
      files { "external/nanovg/src/*.c" }
      targetdir("lib")
      defines { "_CRT_SECURE_NO_WARNINGS", "GLEW_STATIC" } --,"FONS_USE_FREETYPE" } Uncomment to compile with FreeType support

      configuration {"linux"}
         targetdir "lib"

      configuration {"Macosx"}
         targetdir "Libraries"

      configuration "Debug"
         defines { "DEBUG" }
         flags { "Symbols", "ExtraWarnings"}

      configuration "Release"
         defines { "NDEBUG" }
         flags { "Optimize", "ExtraWarnings"}

   -- Assimp library
   project "assimp"
      kind "SharedLib"
      language "C"
      targetdir "lib"
      files {"external/assimp/*.c", "external/assimp/*.h"}
      
      configuration {"linux"}
         targetdir "lib"

      configuration {"Macosx"}
         targetdir "Libraries"

      configuration "Debug"
         defines { "DEBUG" }
         flags { "Symbols", "ExtraWarnings" }

      configuration "Release"
         defines { "NDEBUG" }
         flags { "Optimize", "ExtraWarnings" }


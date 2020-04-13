#!lua

-- XCode build notes:
-- Install dependencies with Homebrew:
-- brew install ffmpeg
-- brew install opencv3
-- brew install assimp
-- You might manually need to change the Deployment Target (macOS SDK) on Mojave at least.

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
         "src/rae_ray/**.cpp",
         "src/test/**.hpp",
         "src/test/**.cpp",
         "src/examples/**.hpp",
         "src/examples/**.cpp"
      }
      includedirs
      {
         "external/glew/include",
         "external/nanovg/src",
         "external/glm",
         "external/glm/glm",
         "src/",
         "src/rae",
         "src/rae_av",
         "src/rae_ray",
         "external/",
         "external/stb"
      }
      links
      {
         "glfw", "glew", "nanovg",
      }
      defines { "GLEW_STATIC", "NANOVG_GLEW" }

      configuration { "linux" }
         buildoptions { "-std=c++11" }
         defines { "USE_RAE_AV" }
         links
         {
            "X11","Xrandr", "Xxf86vm", "Xinerama", "Xcursor", "dl", "rt", "GL", "GLU", "pthread",
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
            "avutil",
            "avcodec",
            "avformat",
            "swscale",
         }

      configuration { "windows" }
         links {"glu32","opengl32", "gdi32", "winmm", "user32"}

      configuration { "macosx" }
         buildoptions { "-std=c++11 -stdlib=libc++" }
         defines
         {
            "USE_RAE_AV",
            "USE_ASSIMP"
         }
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

            "assimp",
         }
         includedirs
         {
            "/usr/local/opt/opencv/include/opencv4/",
            "/usr/local/opt/ffmpeg/include/",
            "/usr/local/opt/assimp/include/",
            "/usr/local/opt/glfw/include/",
         }
         libdirs
         {
             --"@loader_path/../Libraries",
             --"@loader_path/../Libraries/opencv3",
             "../Libraries/",
             --"../Libraries/opencv3/",
             "/usr/local/opt/",
             --"/usr/local/opt/opencv3/lib/",
             "/usr/local/opt/opencv/lib/",
             "/usr/local/opt/ffmpeg/lib/",
             "/usr/local/opt/assimp/lib/",
             "/usr/local/opt/glfw/lib/",
         }
         linkoptions
         {
            "-stdlib=libc++",
            "-framework OpenGL",
            "-framework Cocoa",
            "-framework IOKit",
            "-framework CoreVideo"
         }

      configuration "Debug"
         defines { "DEBUG" }
         flags { "Symbols" }
         debugdir "bin/"

      configuration "Release"
         defines { "NDEBUG" }
         flags { "Optimize" }
         debugdir "bin/"


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

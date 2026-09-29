cmake_minimum_required(VERSION 3.25)
project(MaineClient VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(MSVC)
  add_compile_options(/W4 /permissive- /EHsc)
endif()
add_executable(MaineClient WIN32
  src/main.cpp
  src/core/paths.cpp src/core/paths.hpp
  src/core/config.cpp src/core/config.hpp
  src/core/hardware.cpp src/core/hardware.hpp
  src/core/launcher.cpp src/core/launcher.hpp
  src/minecraft/version.cpp src/minecraft/version.hpp
  src/optimization/profile.cpp src/optimization/profile.hpp
  src/optimization/optimizer.cpp src/optimization/optimizer.hpp
  src/ui/main_window.cpp src/ui/main_window.hpp
)
target_compile_definitions(MaineClient PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
if(WIN32)
  target_link_libraries(MaineClient PRIVATE advapi32 ole32 shell32 version userenv dxgi)
endif()

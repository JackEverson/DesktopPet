cmake_minimum_required(VERSION 3.4...4.0)

project(DesktopPet VERSION 0.1
      DESCRIPTION "A small Win32 desktop pet"
      LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W3")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

# stb_image as an interface lib (header-only)
add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE external)

set(DesktopPet_src
  src/main.cpp
  src/PetWindow.cpp
  src/TrayIcon.cpp
  src/Pet.cpp
  src/Animation.cpp
  src/SpriteSheet.cpp
  src/Stats.cpp

  src/PetWindow.hpp
  src/TrayIcon.hpp
  src/Pet.hpp
  src/Animation.hpp
  src/SpriteSheet.hpp
  src/Stats.hpp
  src/Config.hpp
)

add_executable(${PROJECT_NAME} WIN32 ${DesktopPet_src} resources.rc)
target_link_libraries(${PROJECT_NAME} PRIVATE
  stb_image
  gdi32
  user32
  shell32
)

# MinGW needs an explicit link of -mwindows for the GUI subsystem too
if(NOT MSVC)
  target_link_options(${PROJECT_NAME} PRIVATE -mwindows -static -static-libgcc -static-libstdc++)
endif()

# Copy res/ next to the actual built binary (which lands under Release/ or
# Debug/ with multi-config generators).
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/res/"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/res/"
  COMMENT "Copying res/ next to ${PROJECT_NAME}"
)

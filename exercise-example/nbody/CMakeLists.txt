include(FetchContent)

# Fetch SFML 3.1.0 instead of relying on a system package (not available on EL9).
set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(SFML
  GIT_REPOSITORY https://github.com/SFML/SFML.git
  GIT_TAG 3.1.0
  GIT_SHALLOW ON)
FetchContent_MakeAvailable(SFML)

add_runnable(nbody main.cpp)
target_link_libraries(nbody PRIVATE SFML::Graphics SFML::Window SFML::System)

add_library(tests STATIC vec2d_test.cpp nbody_simulation_test.cpp)
target_link_libraries(tests PRIVATE SFML::Graphics SFML::Window SFML::System)

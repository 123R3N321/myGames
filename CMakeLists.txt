cmake_minimum_required(VERSION 3.27)
project(myGames)

# Find SDL2 and OpenGL
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
#find_package(SDL2_mixer REQUIRED)
#find_package(SDL2_image REQUIRED)


include(FindPkgConfig)
pkg_check_modules(SDL2_Mixer REQUIRED IMPORTED_TARGET SDL2_mixer)
#target_link_libraries(SDL_Test PkgConfig::SDL2_Mixer)


# Add your source files here
set(CMAKE_CXX_STANDARD 17)

# Create the executable
add_executable(myGames
#        testField.cpp
#        Entity.cpp
#        Entity.h
#        SmartEntity.cpp
        SmartEntity.h
        Map.cpp
        Map.h
        include/ShaderProgram.h
        include/stb_image.h
        include/ShaderProgram.cpp
#        exercise.cpp
#        pong.cpp
#        simpleGamesimpleGame.cpp
#        Menu.cpp
        Menu.h
        3d.cpp
)

# Include SDL2 headers for the executable
target_include_directories(myGames PRIVATE
        ${SDL2_INCLUDE_DIRS}
#        ${SDL2_MIXER_INCLUDE_DIRS}
#        ${SDL2_IMAGE_INCLUDE_DIRS}
)

# Link against SDL2 and OpenGL
target_link_libraries(myGames PRIVATE
        ${SDL2_LIBRARIES}
        PkgConfig::SDL2_Mixer
#        ${SDL2_MIXER_LIBRARIES}
#        ${SDL2_IMAGE_LIBRARIES}
        OpenGL::GL
)

# Print diagnostic messages
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")

# Add SDL2 flags manually
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SDL2_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SDL2_LINK_FLAGS}")

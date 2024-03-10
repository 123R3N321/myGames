cmake_minimum_required(VERSION 3.27)
project(myGames)

# Find SDL2 and OpenGL
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

# Add your source files here
set(CMAKE_CXX_STANDARD 14)

# Create the executable
add_executable(myGames
        testField.cpp
        Entity.cpp
        Entity.h
        include/ShaderProgram.h
        include/stb_image.h
        include/ShaderProgram.cpp
#        exercise.cpp
#        pong.cpp
)

# Include SDL2 headers for the executable
target_include_directories(myGames PRIVATE ${SDL2_INCLUDE_DIRS})

# Link against SDL2 and OpenGL
target_link_libraries(myGames PRIVATE
        ${SDL2_LIBRARIES}
        OpenGL::GL
)

# Print diagnostic messages
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")

# Add SDL2 flags manually
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SDL2_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SDL2_LINK_FLAGS}")

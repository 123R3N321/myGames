
#define LOG(argument) std::cout << argument << '\n'


#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 5

#define ENEMY_COUNT 3    //add enemies

#ifdef _WINDOWS
#include <GL/glew.h>
#endif


#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "include/glm/mat4x4.hpp"
#include "include/glm/gtc/matrix_transform.hpp"
#include "include/glm/gtc/type_ptr.hpp"
#include "include/ShaderProgram.h"
#include "include/stb_image.h"
#include <cmath>


const int WINDOW_WIDTH = 800;   //use a smaller window
const int WINDOW_HEIGHT = 600;

float rotationSpeed = 50.0f; //45 - 55 ok, global param for speed of diagonal rotation
int axisMode = 0;   //0, 1, 2   (different random number results)
float height = 5.0f;    //3 - 7 ok, global param for falling height (so also size of dice)

////////////////////////////////////////sprite loading//////////????????///////////////


ShaderProgram g_shader_program;
GLuint g_player_texture_id;

const char V_SHADER_PATH[] = "/home/ren/projects/myGames/include/shaders/vertex_textured.glsl",
        F_SHADER_PATH[] = "/home/ren/projects/myGames/include/shaders/fragment_textured.glsl",
        FONT_SPRITE_FILEPATH[]   = "/home/ren/projects/myGames/include/assets/font1.png",
        PLAYER_SPRITE_FILEPATH[] = "/home/ren/projects/myGames/include/assets/RachelsRocket.png";

const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL = 0;
const GLint TEXTURE_BORDER = 0;


const float BG_RED = 0.1922f,
        BG_BLUE = 0.149f,
        BG_GREEN = 0.1059f,
        BG_OPACITY = 1.0f;

// Define texture coordinates for each vertex of each quadrilateral
GLfloat texture_coordinates[] = {
        // Front face (Red)
        0.0f, 0.0f, // Bottom-left
        1.0f, 0.0f, // Bottom-right
        1.0f, 1.0f, // Top-right
        0.0f, 1.0f, // Top-left

        // Back face (Green)
        0.0f, 0.0f, // Bottom-left
        1.0f, 0.0f, // Bottom-right
        1.0f, 1.0f, // Top-right
        0.0f, 1.0f, // Top-left

        // Top face (Blue)
        0.0f, 0.0f, // Bottom-left
        1.0f, 0.0f, // Bottom-right
        1.0f, 1.0f, // Top-right
        0.0f, 1.0f, // Top-left

        // Bottom face (Yellow)
        0.0f, 0.0f, // Bottom-left
        1.0f, 0.0f, // Bottom-right
        1.0f, 1.0f, // Top-right
        0.0f, 1.0f, // Top-left

        // Right face (Magenta)
        0.0f, 0.0f, // Bottom-left
        1.0f, 0.0f, // Bottom-right
        1.0f, 1.0f, // Top-right
        0.0f, 1.0f, // Top-left

        // Left face (Cyan)
        0.0f, 0.0f, // Bottom-left
        1.0f, 0.0f, // Bottom-right
        1.0f, 1.0f, // Top-right
        0.0f, 1.0f, // Top-left
};


GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint texture_id;
    glGenTextures(NUMBER_OF_TEXTURES, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(image);

    return texture_id;
}

/////////////-----------------------------//////////////////

//// Enable vertex attribute array for texture coordinates
//glEnableVertexAttribArray(g_shader_program.texCoordAttribute);
//
//// Bind texture coordinates data to the vertex attribute
//glVertexAttribPointer(g_shader_program.texCoordAttribute, 2, GL_FLOAT, GL_FALSE, 0, texture_coordinates);
//
//// Bind texture
//glBindTexture(GL_TEXTURE_2D, g_player_texture_id);
//glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
//
//// We disable two attribute arrays now
//glDisableVertexAttribArray(g_shader_program.get_position_attribute());
//glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
//
//SDL_GL_SwapWindow(display_window);
//}

//////////////////////////////////////////////////////////////////////////////////////


void drawCube() {
    // Front face
    glBegin(GL_QUADS);
    glColor3f(0.80f, 0.0f, 0.0f); // Red
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glEnd();

    // Back face
    glBegin(GL_QUADS);
    glColor3f(0.0f, 0.80f, 0.0f); // Green
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    glEnd();

    // Top face
    glBegin(GL_QUADS);
    glColor3f(0.0f, 0.0f, 0.80f); // Blue
    glVertex3f(-0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glEnd();

    // Bottom face
    glBegin(GL_QUADS);
    glColor3f(0.80f, 0.80f, 0.0f); // Yellow
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glEnd();

    // Right face
    glBegin(GL_QUADS);
    glColor3f(0.80f, 0.0f, 0.80f); // Magenta
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glEnd();

    // Left face
    glBegin(GL_QUADS);
    glColor3f(0.0f, 0.80f, 0.80f); // Cyan
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glEnd();
}

float bounceFunction(float time, float height) {
    if(time<1.76){
        return -exp(time) + 1;}
    else if(time<sqrt( 9.81/8 * height)){  //hit gorund instance
        return exp(-time) * sin(17 * time) - height;
    }else{  //stabe stationary instance
        return -height;
    }
}

std::string callRes(int angle){
    angle /= 10;
    angle *= 10;    //a simple digit trim
//    LOG("angle rounded: "<<angle);
//    LOG("check again? "<<angle%180);
if(0 == axisMode){
    if(0 == angle%360){
        return "pink";
    }
    else if (0 == angle%180){   //somehow casting not working
        return "blue";
    }else{    //90 deg
        return "red";
    }
}
    else if(1 == axisMode){
        return "pink";  //unfinished
    }
    else{
        return "cyan";
    }

}


int main(int argc, char* argv[]) {

    float totalRotationAngle = 0.0f;    //non-user tracker param
    float rotationAngle = 0.0f;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("CubeExperiment", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    SDL_GLContext context = SDL_GL_CreateContext(window);


    //////////////////////////////////////////vvvvvvvvvvvvvvvvvvvvv/////
//todo: figure out why shaderprogram loading kills the object, binding to quadrilateral,
//todo: ask about it maybe

//todo: tentative conclusion: it is because all faces have the same color.

//    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

//    glUseProgram(g_shader_program.get_program_id());
//
//    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
//
//    g_player_texture_id = load_texture(PLAYER_SPRITE_FILEPATH);
    float Axis[] = {                        //overall 3by3 is enough
            1.0f,1.0f,1.0f,     //blue
            1.0f,-1.0f,1.0f,    //pink
            1.0f,1.0f,-1.0f,    //cyan
    };

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    ///////////////////////////////////////^^^^^^^^^^^^^^^^^^^^^^^^^////////////////
    bool stopFlag = false;

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    bool logFlag = true;

    bool running = true;
    while (running) {

//        LOG(bounceFunction((GLfloat)SDL_GetTicks() / 1000.0f,5.0f));

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Clear the screen
                            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                            // Set up the projection matrix
                            glMatrixMode(GL_PROJECTION);
                            glLoadIdentity();
                            glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
                            glLoadMatrixf(glm::value_ptr(projectionMatrix));

                            // Set up the modelview matrix
                            glMatrixMode(GL_MODELVIEW);
                            glLoadIdentity();
                            glTranslatef(0.0f, 0.0f, bounceFunction((GLfloat)SDL_GetTicks() / 1000.0f,height));
                            if(rotationSpeed>37.5f){rotationSpeed -= (GLfloat)SDL_GetTicks() / 1000.0f;}
                            if(rotationSpeed>7.5f){rotationSpeed -= (GLfloat)SDL_GetTicks() / 10000.0f;}
                            if(!stopFlag) rotationAngle = (GLfloat)SDL_GetTicks() / 100.0f * rotationSpeed; //problem is this line cannot be easily adjusted to other modes of control
                            glRotatef(rotationAngle, Axis[axisMode*3], Axis[axisMode*3 + 1], Axis[axisMode*3 + 2]); // Rotate the cube
                            totalRotationAngle += rotationAngle;
                            if(totalRotationAngle/100 >= 540){  //540, 720, 960 good
                                rotationSpeed=0;
                                stopFlag = true;
                                if(logFlag){
                                    LOG("end angle: "<<totalRotationAngle/100);
                                    LOG(callRes(std::round(totalRotationAngle/100)));
                                    logFlag = false;
                                }
                            } //stop rotation, only safe beyond 180 deg as 90 deg time window not enough
                            // Draw the cube
                            drawCube();

                            // Swap buffers
                            SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
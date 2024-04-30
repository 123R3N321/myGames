
#define LOG(argument) std::cout << argument << '\n'


#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION    //this is for sprite
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
GLuint DICE1ID;
GLuint DICE2ID;
GLuint DICE3ID;
GLuint DICE4ID;
GLuint DICE5ID;
GLuint DICE6ID;


const char V_SHADER_PATH[] = "/home/ren/projects/myGames/include/shaders/vertex_textured.glsl",
        F_SHADER_PATH[] = "/home/ren/projects/myGames/include/shaders/fragment_textured.glsl",
        FONT_SPRITE_FILEPATH[]   = "/home/ren/projects/myGames/include/assets/font1.png",
        DICE1[] = "/home/ren/projects/myGames/include/assets/Moon.jpeg",
        DICE2[] = "/home/ren/projects/myGames/include/assets/Moon.jpeg",
        DICE3[] = "/home/ren/projects/myGames/include/assets/Moon.jpeg",
        DICE4[] = "/home/ren/projects/myGames/include/assets/Moon.jpeg",
        DICE6[] = "/home/ren/projects/myGames/include/assets/Moon.jpeg",
DICE5[] = "/home/ren/projects/myGames/include/assets/simpleTile.png";

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



void drawCubeFace(GLfloat* vertices, GLfloat* texCoords, GLuint textureID) {
    // Enable vertex array
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    // Specify vertex data
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, texCoords);

    // Bind texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Draw the face
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Disable vertex array
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}


GLfloat frontVertices[] = {
        -0.5f, -0.5f, 0.5f,  // Vertex 1
        0.5f, -0.5f, 0.5f,   // Vertex 2
        -0.5f, 0.5f, 0.5f,   // Vertex 3
        0.5f, -0.5f, 0.5f,   // Vertex 4
        0.5f, 0.5f, 0.5f,    // Vertex 5
        -0.5f, 0.5f, 0.5f    // Vertex 6
};

GLfloat frontTexCoords[] = {
        0.0f, 0.0f,  // Texture coordinate for Vertex 1 (bottom-left corner)
        1.0f, 0.0f,  // Texture coordinate for Vertex 2 (bottom-right corner)
        0.0f, 1.0f,  // Texture coordinate for Vertex 3 (top-left corner)
        1.0f, 0.0f,  // Texture coordinate for Vertex 4 (bottom-right corner)
        1.0f, 1.0f,  // Texture coordinate for Vertex 5 (top-right corner)
        0.0f, 1.0f   // Texture coordinate for Vertex 6 (top-left corner)
};

// Back face
GLfloat backVertices[] = {
        -0.5f, -0.5f, -0.5f, // Vertex 1
        0.5f, -0.5f, -0.5f,  // Vertex 2
        -0.5f, 0.5f, -0.5f,  // Vertex 3
        0.5f, -0.5f, -0.5f,  // Vertex 4
        0.5f, 0.5f, -0.5f,   // Vertex 5
        -0.5f, 0.5f, -0.5f   // Vertex 6
};

GLfloat backTexCoords[] = {
        1.0f, 1.0f,  // Texture coordinate for Vertex 1 (top-right corner)
        0.0f, 1.0f,  // Texture coordinate for Vertex 2 (top-left corner)
        1.0f, 0.0f,  // Texture coordinate for Vertex 3 (bottom-right corner)
        0.0f, 1.0f,  // Texture coordinate for Vertex 4 (top-left corner)
        0.0f, 0.0f,  // Texture coordinate for Vertex 5 (bottom-left corner)
        1.0f, 0.0f   // Texture coordinate for Vertex 6 (bottom-right corner)
};

// Top face
GLfloat topVertices[] = {
        -0.5f, 0.5f, -0.5f,  // Vertex 1
        0.5f, 0.5f, -0.5f,   // Vertex 2
        -0.5f, 0.5f, 0.5f,   // Vertex 3
        0.5f, 0.5f, -0.5f,   // Vertex 4
        0.5f, 0.5f, 0.5f,    // Vertex 5
        -0.5f, 0.5f, 0.5f    // Vertex 6
};

GLfloat topTexCoords[] = {
        0.0f, 1.0f,  // Texture coordinate for Vertex 1 (bottom-left corner)
        1.0f, 1.0f,  // Texture coordinate for Vertex 2 (bottom-right corner)
        0.0f, 0.0f,  // Texture coordinate for Vertex 3 (top-left corner)
        1.0f, 1.0f,  // Texture coordinate for Vertex 4 (bottom-right corner)
        1.0f, 0.0f,  // Texture coordinate for Vertex 5 (top-right corner)
        0.0f, 0.0f   // Texture coordinate for Vertex 6 (top-left corner)
};


// Bottom face
GLfloat bottomVertices[] = {
        -0.5f, -0.5f, -0.5f, // Vertex 1
        0.5f, -0.5f, -0.5f,  // Vertex 2
        -0.5f, -0.5f, 0.5f,  // Vertex 3
        0.5f, -0.5f, -0.5f,  // Vertex 4
        0.5f, -0.5f, 0.5f,   // Vertex 5
        -0.5f, -0.5f, 0.5f   // Vertex 6
};

GLfloat bottomTexCoords[] = {
        0.0f, 0.0f,  // Texture coordinate for Vertex 1 (top-left corner)
        1.0f, 0.0f,  // Texture coordinate for Vertex 2 (top-right corner)
        0.0f, 1.0f,  // Texture coordinate for Vertex 3 (bottom-left corner)
        1.0f, 0.0f,  // Texture coordinate for Vertex 4 (top-right corner)
        1.0f, 1.0f,  // Texture coordinate for Vertex 5 (bottom-right corner)
        0.0f, 1.0f   // Texture coordinate for Vertex 6 (bottom-left corner)
};

// Right face
GLfloat rightVertices[] = {
        0.5f, -0.5f, -0.5f,  // Vertex 1
        0.5f, 0.5f, -0.5f,   // Vertex 2
        0.5f, -0.5f, 0.5f,   // Vertex 3
        0.5f, 0.5f, -0.5f,   // Vertex 4
        0.5f, 0.5f, 0.5f,    // Vertex 5
        0.5f, -0.5f, 0.5f    // Vertex 6
};

GLfloat rightTexCoords[] = {
        0.0f, 0.0f,  // Texture coordinate for Vertex 1 (bottom-left corner)
        1.0f, 0.0f,  // Texture coordinate for Vertex 2 (bottom-right corner)
        0.0f, 1.0f,  // Texture coordinate for Vertex 3 (top-left corner)
        1.0f, 0.0f,  // Texture coordinate for Vertex 4 (bottom-right corner)
        1.0f, 1.0f,  // Texture coordinate for Vertex 5 (top-right corner)
        0.0f, 1.0f   // Texture coordinate for Vertex 6 (top-left corner)
};

// Left face
GLfloat leftVertices[] = {
        -0.5f, -0.5f, -0.5f, // Vertex 1
        -0.5f, 0.5f, -0.5f,  // Vertex 2
        -0.5f, -0.5f, 0.5f,  // Vertex 3
        -0.5f, 0.5f, -0.5f,  // Vertex 4
        -0.5f, 0.5f, 0.5f,   // Vertex 5
        -0.5f, -0.5f, 0.5f   // Vertex 6
};

GLfloat leftTexCoords[] = {
        1.0f, 0.0f,  // Texture coordinate for Vertex 1 (bottom-right corner)
        0.0f, 0.0f,  // Texture coordinate for Vertex 2 (bottom-left corner)
        1.0f, 1.0f,  // Texture coordinate for Vertex 3 (top-right corner)
        0.0f, 0.0f,  // Texture coordinate for Vertex 4 (bottom-left corner)
        0.0f, 1.0f,  // Texture coordinate for Vertex 5 (top-left corner)
        1.0f, 1.0f   // Texture coordinate for Vertex 6 (top-right corner)
};


void standardDraw(GLuint DICE1ID, GLuint DICE2ID, GLuint DICE3ID, GLuint DICE4ID, GLuint DICE5ID, GLuint DICE6ID){

    drawCubeFace(frontVertices, frontTexCoords, DICE1ID);

    drawCubeFace(backVertices, backTexCoords, DICE2ID);

    drawCubeFace(topVertices, topTexCoords, DICE3ID);

    drawCubeFace(bottomVertices, bottomTexCoords, DICE4ID);

    drawCubeFace(rightVertices, rightTexCoords, DICE5ID);

    drawCubeFace(leftVertices, leftTexCoords, DICE6ID);

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

//    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);    //sprite
    DICE1ID = load_texture(DICE1); //sprite
    DICE2ID = load_texture(DICE2); //sprite
    DICE3ID = load_texture(DICE3); //sprite
    DICE4ID = load_texture(DICE4); //sprite
    DICE5ID = load_texture(DICE5); //sprite
    DICE6ID = load_texture(DICE6); //sprite

    //    LOG("front texture ID: "<< frontTextureID<<"\n");




    float Axis[] = {                        //overall 3by3 is enough
            1.0f,1.0f,1.0f,     //blue
            1.0f,-1.0f,1.0f,    //pink
            1.0f,1.0f,-1.0f,    //cyan
    };

    glEnable(GL_TEXTURE_2D);


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
//                            drawCube();
                            standardDraw(DICE1ID, DICE2ID,DICE3ID,DICE4ID,DICE5ID,DICE6ID);
                            // Swap buffers
                            SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
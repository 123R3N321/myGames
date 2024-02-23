/**
I will try to make this into a framework
 **/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

/**
 * below is a convenient debug tool
 */

#define LOG(argument) std::cout << argument << '\n'



/**
 File path adjusted to relative, only compatible with Debian-based linux
 because the packages for glm and sdl are under "SDL2" directory
 for all other OS, remove "include/" from headers
 **/


#define GL_GLEXT_PROTOTYPES 1
#include "SDL2/SDL.h"
#include "SDL2/SDL_opengl.h"
#include "include/glm/mat4x4.hpp"
#include "include/glm/gtc/matrix_transform.hpp"
#include "include/ShaderProgram.h"
#include "include/stb_image.h"

/**
 * in the future we could change shaders
 * note we also have two sprites used here
 * they all have to be absolute paths
 * same as other headers, for other OS/envrionments, remove all path components before "include" but keep "include/" itsef
 */
const char V_SHADER_PATH[] = "/home/ren/projects/myGames/include/shaders/vertex_textured.glsl",
        F_SHADER_PATH[] = "/home/ren/projects/myGames/include/shaders/fragment_textured.glsl";

const char PLAYER_SPRITE_FILEPATH[] = "/home/ren/projects/myGames/include/assets/Sun.png";
const char OTHER[] = "/home/ren/projects/myGames/include/assets/Earth.jpeg";

/**
 * each new sprite requires a new id, it is simply an ind pos from 1 to infini, note that 0 is not used.
 */
GLuint player_texture_id;
GLuint other_texture_id;

/**
 * similar to sprite id situation, each new object requires a new model matrix
 */
glm::mat4 g_view_matrix, g_projection_matrix,
            g_model_matrix, other_model_matrix; //here, add more model matrices for new objects


#define LOG(argument) std::cout << argument << '\n'


/**
Instead of putting all global variables together on top, I will
 only declare them immediately before they are needed for a function
 this is thus more readable and easier to trace
 **/
const char WINDOW_TITLE[] = "No Title";

const int WINDOW_WIDTH  = 1280,
        WINDOW_HEIGHT = 960;

const float  BG_RED = 0.0f,
        BG_BLUE = 0.0f,
        BG_GREEN = 0.0f,
        BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
        VIEWPORT_Y = 0,
        VIEWPORT_WIDTH  = WINDOW_WIDTH,
        VIEWPORT_HEIGHT = WINDOW_HEIGHT;


const float MINIMUM_COLLISION_DISTANCE = 1.0f;

const float RADIUS = 2.0f;      // radius of circle
const float ROT_SPEED = 0.01f;  // rotational speed
const float ROT_ANGLE = glm::radians(1.5f);
const float TRAN_VALUE = 0.01f;

float g_angle    = 0.0f;        // current angle accumulated
float g_x_coords = RADIUS;      // current x-coord
float g_y_coords = 0.0f;

const float GROWTH_FACTOR = 1.01f;  // grow by 1.0% / frame
const float SHRINK_FACTOR = 0.99f;  // grow by -1.0% / frame


SDL_Window* display_window;
bool g_game_is_running = true;
bool is_growing = true;

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL  = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER   = 0;   // this value MUST be zero


/**
 * x, y coordinate system used for tracking mouse cursor position
 * only used by process_input()
 */
enum Coordinate { x_coordinate, y_coordinate };
const Coordinate X_COORDINATE = x_coordinate;
const Coordinate Y_COORDINATE = y_coordinate;

/**
 * below variables only used for mouse hovering positioning adjustment
 * only used by get_screen_to_ortho() function
 */
const float ORTHO_WIDTH  = 7.5f,
        ORTHO_HEIGHT = 10.0f;

/**
 * below variables are used for controlling movement and indicate cursor position
 * mouseX, mouseY are absolute mouse cursor positions on-screen
 * orthoX, orthoY are adjusted positions from mouseX and mouseY, relative position of the game window.
 * only used by process_input()
 */
glm::vec3 g_player_movement = glm::vec3(0.0f, 0.0f, 0.0f);
int mouseX, mouseY;
float orthoX, orthoY;


ShaderProgram g_program;


/**
 * additional stuff for feb23 exercise
 *
 */
 int mode = 1;
int jump = 1;
/**
 * This function is used to load the sprites, no future modification needed
 *
 * @param Absolute filepath, in an array of char (char[])
 * @return id, which is positive int from 1 onwards
 * @ModificationInPlace function modifies underlying global variables provided by openGL
 */

GLuint load_texture(const char* filepath)
{

    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }


    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    stbi_image_free(image);

    return textureID;
}



/**
 * heavy modification needed for any future uses.
 * I will try to eliminate the need of modifying this function in the future
 * possibly need dynamic memory.(such as an array of texture inds on the heap)
 * @calledByMain
 */
void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    display_window = SDL_CreateWindow(WINDOW_TITLE,
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(display_window);
    SDL_GL_MakeCurrent(display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_program.load(V_SHADER_PATH, F_SHADER_PATH);



/**
 * Initial positioning of model matrices is actually optional as we reset in update()
 * but if I want to improve update() and get rid of that cumbersome reset every frame, i prolly need them here.
 */
    g_model_matrix = glm::mat4(1.0f);
    other_model_matrix = glm::mat4(1.0f);



    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_program.set_projection_matrix(g_projection_matrix);
    g_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    /**
     * generate multiple indeces and load multiple textures.
     * call additional load_texture() as long as you need more sprites
     */
    player_texture_id = load_texture(PLAYER_SPRITE_FILEPATH);
    other_texture_id = load_texture(OTHER);

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


float get_screen_to_ortho(float coordinate, Coordinate axis)
{
    switch(axis)
    {
        case x_coordinate: return ((coordinate / WINDOW_WIDTH) * ORTHO_WIDTH) - (ORTHO_WIDTH / 2.0);
        case y_coordinate: return (((WINDOW_HEIGHT - coordinate) / WINDOW_HEIGHT) * ORTHO_HEIGHT) - (ORTHO_HEIGHT / 2.0);
        default          : return 0.0f;
    }
}


void process_input()
{
    g_player_movement = glm::vec3(0.0f);

    SDL_GetMouseState(&mouseX, &mouseY);
    orthoX = get_screen_to_ortho(mouseX, X_COORDINATE);
    orthoY = get_screen_to_ortho(mouseY, Y_COORDINATE);


    //
    //
    // –––––––––––––––––––––––-----– KEYSTROKES ---------––––––––––––––––––– //
    //
    SDL_Event event;                                                         //
    while (SDL_PollEvent(&event))                                            //
    {                                                                        //
        switch (event.type)                                                  //
        {                                                                    //
            // End game                                                      //
            case SDL_QUIT:                                                   //
            case SDL_WINDOWEVENT_CLOSE:                                      //
                g_game_is_running = false;                                   //
                break;                                                       //
                //
            case SDL_KEYDOWN:                                                //
                switch (event.key.keysym.sym)                                //
                {

                    case SDLK_t:
                        jump *= -1;
                        break;
                    case SDLK_r:
                        mode*= -1;
                        break;

                    //                     //
                        //
                    case SDLK_q:                                             //
                        // Quit the game with a keystroke                    //
                        g_game_is_running = false;                           //
                        break;                                               //
                        //
                    default:                                                 //
                        break;                                               //
                }

                //
                //
            default:                                                         //
                break;                                                       //
        }                                                                    //
    }                                                                        //
    //
    // ––––––––––––––––––––––––––––––– KEY HOLD –––––––––––––––––––––––––––– //
    //
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);                     //
    //
    if (key_state[SDL_SCANCODE_LEFT])                                        //
    {                                                                        //
        g_player_movement.x = -1.0f;                                         //
    }                                                                        //
    else if (key_state[SDL_SCANCODE_RIGHT])                                  //
    {                                                                        //
        g_player_movement.x = 1.0f;                                          //
    }                                                                        //
    //
    if (key_state[SDL_SCANCODE_UP])                                          //
    {                                                                        //
        g_player_movement.y = 1.0f;                                          //
    }                                                                        //
    else if (key_state[SDL_SCANCODE_DOWN])                                   //
    {                                                                        //
        g_player_movement.y = -1.0f;                                         //
    }                                                                        //
    //
    // This makes sure that the player can't "cheat" their way into moving   //
    // faster, normalize direction vectors to always be unit vector.                                                                //
    if (glm::length(g_player_movement) > 1.0f)                               //
    {                                                                        //
        g_player_movement = glm::normalize(g_player_movement);               //
    }                                                                        //
    // ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––– //
}

/**
 Uses distance formula.
 */


float delta_time;
float g_previous_ticks  = 0.0f;
float speed = 2.0f;

const float radius = 2.0f;

glm::vec3 g_player_position = glm::vec3(0.0f, 0.0f, 0.0f);     //


void timer(){
    float ticks = (float) SDL_GetTicks() / 1000.0F; // get the current number of ticks
    delta_time = ticks - g_previous_ticks; // the delta time is the difference from the last frame
    g_previous_ticks = ticks;
}




void update()
{
    timer();    //makes sure delta time is flowing
    g_player_position += g_player_movement * speed * delta_time;   //

    glm::vec3 scale_vector;


    scale_vector = glm::vec3(is_growing ? GROWTH_FACTOR : SHRINK_FACTOR,
                             is_growing ? GROWTH_FACTOR : SHRINK_FACTOR,
                             1.0f);
    g_model_matrix =glm::scale(g_model_matrix, scale_vector);

    g_model_matrix = glm::mat4(1.0f);                                       //
    g_model_matrix = glm::translate(g_model_matrix, glm::vec3(orthoX,orthoY,0.0f));
    g_model_matrix = glm::translate(g_model_matrix, g_player_position);
//    g_model_matrix = glm::translate(g_model_matrix, glm::vec3(TRAN_VALUE, TRAN_VALUE, 0.0f));
    g_model_matrix = glm::rotate(g_model_matrix, ROT_ANGLE, glm::vec3(0.0f, 0.0f, 1.0f));

    if(1==mode){
        g_angle += ROT_SPEED;
    }else if(-1==mode){
        g_angle -= ROT_SPEED;

    }
    g_x_coords = RADIUS * glm::cos(g_angle);
    g_y_coords = RADIUS * glm::sin(g_angle);

    other_model_matrix = glm::translate(g_model_matrix, glm::vec3(jump * g_x_coords, jump * g_y_coords, 0.0f));







}

void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id)
{
    g_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);

}


void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Vertices
    float vertices[] = {
            -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
            -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates[] = {
            0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
            0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };

    glVertexAttribPointer(g_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_program.get_position_attribute());

    glVertexAttribPointer(g_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_program.get_tex_coordinate_attribute());

    // Bind texture
    draw_object(g_model_matrix, player_texture_id);
    draw_object(other_model_matrix, other_texture_id);

    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_program.get_position_attribute());
    glDisableVertexAttribArray(g_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(display_window);
}

void shutdown() { SDL_Quit(); }


int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
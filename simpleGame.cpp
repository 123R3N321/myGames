
/**
* Author: Ren
* Assignment: Rise of the AI
* Date due: 2024-03-23, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

/**
 * more note: x pos 13 means good.
 */

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

/**
 * 2 notes: I am really not sure if the mixer can work at all
 * And that once I remove this main file from cmake, it will fail to detect SDL, so don't panic
 * Just add it back to cmake and problem solved

oh, and, platform location x 4+, y-0.1+

 */

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "include/glm/mat4x4.hpp"
#include "include/glm/gtc/matrix_transform.hpp"
#include "include/ShaderProgram.h"
#include "include/stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "SmartEntity.h"
#include "Map.h"
#include "Menu.h"

GLuint  g_text_texture_id;
const int FONTBANK_SIZE        = 16;
volatile int enemyCount = ENEMY_COUNT;
volatile bool GameOver = false;

/**
 * Agenda:
 *  implement meta_init() to modify map and enemy
 *  --->next, implement life-count mechanism
 *  --->next, optionally handle shader
 *  --->next, optionally handle music
 */


//Set metaData which is global variable
int meta_player_life = 3;

int meta_lvl_count = 0;
std::vector<AIType> EnemyInfo = {ROCKET, ROCKET,ROCKET};
std::vector<float> SpawnPo = {2.5f,4.3f,12.3f};

// ————— GAME STATE ————— //
struct GameState
{
    Entity* player;
    Entity* enemies;

    Map* map;

    Mix_Music* bgm;
    Mix_Chunk* jump_sfx;
};

// ————— CONSTANTS ————— //
const int   WINDOW_WIDTH = 1280,
        WINDOW_HEIGHT = 960;

const float BG_RED = 0.1922f,
        BG_BLUE = 0.549f,
        BG_GREEN = 0.9059f,
        BG_OPACITY = 1.0f;

const int   VIEWPORT_X = 0,
        VIEWPORT_Y = 0,
        VIEWPORT_WIDTH = WINDOW_WIDTH,
        VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char GAME_WINDOW_NAME[] = "Hello, RandomAssets";

const char V_SHADER_PATH[] = "/home/ren/projects/myGames/include/shaders/vertex_textured.glsl",
        F_SHADER_PATH[] = "/home/ren/projects/myGames/include/shaders/fragment_textured.glsl",
        FONT_SPRITE_FILEPATH[]   = "/home/ren/projects/myGames/include/assets/font1.png";

const float MILLISECONDS_IN_SECOND = 1000.0;

const char
//SPRITESHEET_FILEPATH[]  = "/home/ren/projects/myGames/include/assets/george_0.png",
        SPRITESHEET_FILEPATH[]  = "/home/ren/projects/myGames/include/assets/Pikachu4x4.png",
        ENEMY0_SPRITE_PATH[] = "/home/ren/projects/myGames/include/assets/RachelsRocket.png",
        MAP_TILESET_FILEPATH[]  = "/home/ren/projects/myGames/include/assets/simpleTile.png",
        BGM_FILEPATH[]          = "/home/ren/projects/myGames/include/assets/LittlerootTownBGM.mp3",
        JUMP_SFX_FILEPATH[]     = "/home/ren/projects/myGames/include/assets/pika.wav";
//
//const char  SPRITESHEET_FILEPATH[]  = "/home/ren/projects/myGames/include/assets/george_0.png",
//        PLATFORM_FILEPATH[]     = "/home/ren/projects/myGames/include/assets/platformPack_tile027.png",
//        FONT_SPRITE_FILEPATH[]   = "/home/ren/projects/myGames/include/assets/font1.png";

const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL = 0;
const GLint TEXTURE_BORDER = 0;




unsigned int LEVEL_1_DATA[] =
        {
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
                1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
                2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
                2, 2, 2, 2, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2
        };

unsigned int LEVEL_2_DATA[] =
        {
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
                1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
                2, 0, 1, 1, 1, 1, 0, 1, 1, 2, 2, 2, 2, 2,
                2, 0, 2, 2, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2
        };

unsigned int LEVEL_3_DATA[] =
        {
                1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
                0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
                1, 1, 1, 1, 1, 0, 1, 1, 1, 2, 2, 2, 2, 2,
                0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, 2, 2, 2
        };


// ————— VARIABLES ————— //
GameState g_game_state;

SDL_Window* g_display_window;
bool g_game_is_running  = true;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float   g_previous_ticks = 0.0f,
        g_accumulator = 0.0f;


/////////////////////////add text engine//////////////////////////////////
void draw_text(ShaderProgram *program, GLuint font_texture_id, std::string text, float screen_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

    // Instead of having a single pair of arrays, we'll have a series of pairs—one for each character
    // Don't forget to include <vector>!
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their position
        //    relative to the whole sentence)
        int spritesheet_index = (int) text[i];  // ascii value of character
        float offset = (screen_size + spacing) * i;

        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float) (spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float) (spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
                offset + (-0.5f * screen_size), 0.5f * screen_size,
                offset + (-0.5f * screen_size), -0.5f * screen_size,
                offset + (0.5f * screen_size), 0.5f * screen_size,
                offset + (0.5f * screen_size), -0.5f * screen_size,
                offset + (0.5f * screen_size), 0.5f * screen_size,
                offset + (-0.5f * screen_size), -0.5f * screen_size,
        });

        texture_coordinates.insert(texture_coordinates.end(), {
                u_coordinate, v_coordinate,
                u_coordinate, v_coordinate + height,
                u_coordinate + width, v_coordinate,
                u_coordinate + width, v_coordinate + height,
                u_coordinate + width, v_coordinate,
                u_coordinate, v_coordinate + height,
        });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);

    program->set_model_matrix(model_matrix);
    glUseProgram(program->get_program_id());

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int) (text.size() * 6));

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}



// ————— GENERAL FUNCTIONS ————— //
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

/**
 * Level stores information of each level and is only directly accessed
 * by meta_init()
 *
 * Note that as for now we have very limited capabilities in it
 * we control enemy type, thats it.
 *
 * Leave map redulation to meta_init as we have array which is harder to deal with
 */

struct Level{   //, create with arugemts (AIType, AIType, AIType, posx1, posx2, posx3)
    std::vector<AIType> EnemyTypes;
    std::vector<float> SpawnPos;  //3 ints, we only care about x position
    Level(std::vector<AIType> EnemyInfo = {COWARD, COWARD,COWARD}, std::vector<float> SpawnPo = {2.5f,4.3f,12.3f}) : EnemyTypes(EnemyInfo), SpawnPos(SpawnPo){}
};
volatile int ind = 0;

void meta_init(){
    g_game_state.player->set_position(glm::vec3(0.0f, 5.0f, 0.0f));//always airdrop player lol
//    meta_lvl_count%=3;

    GLuint map_texture_id = load_texture(MAP_TILESET_FILEPATH);

    g_game_state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, (0==meta_lvl_count)?LEVEL_1_DATA:(1==meta_lvl_count)?LEVEL_2_DATA:LEVEL_3_DATA, map_texture_id, 1.0f, 4, 1);
    LOG("cur level: "<<meta_lvl_count<<std::endl);

    Level level1(EnemyInfo,SpawnPo);
    if(1==meta_lvl_count){
        level1.EnemyTypes[1] = CHARGER;
        level1.SpawnPos[0]+= 4.0f;
    }
    if(2==meta_lvl_count){
        level1.EnemyTypes[2] = CHARGER;
        level1.SpawnPos[2]+= 2.0f;
    }

    // ————— ENEMY SET-UP ————— //
    // Existing
    g_game_state.enemies = new Entity[ENEMY_COUNT];
    for (ind=0;ind<ENEMY_COUNT;++ind){
        g_game_state.enemies[ind].set_entity_type(ENEMY);
        g_game_state.enemies[ind].set_ai_type(level1.EnemyTypes[ind]);
        g_game_state.enemies[ind].set_ai_state(IDLE);
        g_game_state.enemies[ind].m_texture_id = load_texture(ENEMY0_SPRITE_PATH);
        g_game_state.enemies[ind].set_position(glm::vec3(level1.SpawnPos[ind], 5.0f, 0.0f));
        g_game_state.enemies[ind].set_movement(glm::vec3(0.0f));
        g_game_state.enemies[ind].set_speed(0.5f);
        g_game_state.enemies[ind].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
        g_game_state.enemies[ind].m_jumping_power = 0.0f;
    }

//
//
//    ++ind;
////
//    g_game_state.enemies[ind].set_entity_type(ENEMY);
//    g_game_state.enemies[ind].set_ai_type(COWARD);
//    g_game_state.enemies[ind].set_ai_state(IDLE);
//    g_game_state.enemies[ind].m_texture_id = load_texture(ENEMY0_SPRITE_PATH);
//    g_game_state.enemies[ind].set_position(glm::vec3(4.30f, 2.0f, 0.0f));
//    g_game_state.enemies[ind].set_movement(glm::vec3(0.0f));
//    g_game_state.enemies[ind].set_speed(0.5f);
//    g_game_state.enemies[ind].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
//    g_game_state.enemies[ind].m_jumping_power = 4.5f;
//
//    ++ind;
////
//    g_game_state.enemies[ind].set_entity_type(ENEMY);
//    g_game_state.enemies[ind].set_ai_type(CHARGER);
//    g_game_state.enemies[ind].set_ai_state(IDLE);
//    g_game_state.enemies[ind].m_texture_id = load_texture(ENEMY0_SPRITE_PATH);
//    g_game_state.enemies[ind].set_position(glm::vec3(12.30f, 2.0f, 0.0f));
//    g_game_state.enemies[ind].set_movement(glm::vec3(0.0f));
//    g_game_state.enemies[ind].set_speed(0.5f);
//    g_game_state.enemies[ind].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
//    g_game_state.enemies[ind].m_jumping_power = 0.0f;
}

void initialise()
{
    // ————— GENERAL ————— //
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow(GAME_WINDOW_NAME,
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        WINDOW_WIDTH, WINDOW_HEIGHT,
                                        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    // ————— VIDEO SETUP ————— //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    ////////////////text font setup//////////////////////////////////////////////
    g_text_texture_id = load_texture(FONT_SPRITE_FILEPATH);



    // ————— MAP SET-UP ————— //



//    if(0==meta_lvl_count){
//        g_game_state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, LEVEL_1_DATA, map_texture_id, 1.0f, 4, 1);
//
//    }
//
//    if(1==meta_lvl_count){
//        g_game_state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, LEVEL_2_DATA, map_texture_id, 1.0f, 4, 1);
//
//    }
//    if(2==meta_lvl_count){
//        g_game_state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, LEVEL_3_DATA, map_texture_id, 1.0f, 4, 1);
//
//    }

    // ————— GEORGE SET-UP ————— //
    // Existing
    g_game_state.player = new Entity();
    g_game_state.player->set_entity_type(PLAYER);
    g_game_state.player->set_position(glm::vec3(0.0f, 5.0f, 0.0f));
    g_game_state.player->set_movement(glm::vec3(0.0f));
    g_game_state.player->set_speed(2.5f);
    g_game_state.player->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    g_game_state.player->m_texture_id = load_texture(SPRITESHEET_FILEPATH);

    // Walking
    g_game_state.player->m_walking[g_game_state.player->DOWN] = new int[4] { 0, 1, 2, 3 };
    g_game_state.player->m_walking[g_game_state.player->LEFT] = new int[4] { 4, 5, 6, 7 };
    g_game_state.player->m_walking[g_game_state.player->RIGHT] = new int[4] { 8, 9, 10, 11 };
    g_game_state.player->m_walking[g_game_state.player->UP] = new int[4] { 12, 13, 14, 15 };

    g_game_state.player->m_animation_indices = g_game_state.player->m_walking[g_game_state.player->RIGHT];  // start George looking left
    g_game_state.player->m_animation_frames = 4;
    g_game_state.player->m_animation_index = 0;
    g_game_state.player->m_animation_time = 0.0f;
    g_game_state.player->m_animation_cols = 4;
    g_game_state.player->m_animation_rows = 4;
    g_game_state.player->set_height(0.8f);
    g_game_state.player->set_width(0.8f);

    // Jumping
    g_game_state.player->m_jumping_power = 6.0f;


    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    g_game_state.bgm = Mix_LoadMUS(BGM_FILEPATH);
    Mix_PlayMusic(g_game_state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME );

    /**
     * if you don want music then comment in the line below
     */
//    Mix_HaltMusic();

    g_game_state.jump_sfx = Mix_LoadWAV(JUMP_SFX_FILEPATH);



    ///////////////////////////below my new stuff//////////////////////////////////////////////////////////////////
        meta_init(); //initialize differently based on level


    // ————— BLENDING ————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void handleMem(){   //extremely dangerous func. Only called when switch scene
    delete[] g_game_state.enemies;
//    delete    g_game_state.player;
    delete    g_game_state.map;
//    Mix_FreeChunk(g_game_state.jump_sfx);
//    Mix_FreeMusic(g_game_state.bgm);
}

float lockLife = 0.0f;

bool keyLock = false;
void process_input()
{
    g_game_state.player->set_movement(glm::vec3(0.0f));

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_game_is_running  = false;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        // Quit the game with a keystroke
                        g_game_is_running  = false;
                        break;

                    case SDLK_SPACE:
                        // Jump
                        if (g_game_state.player->m_collided_bottom)
                        {
                            g_game_state.player->m_is_jumping = true;
                            Mix_PlayChannel(-1, g_game_state.jump_sfx, 0);
                        }
                        break;

                    case SDLK_k:
                        // Jump
                        if (g_game_state.player->m_collided_bottom)
                        {
                            g_game_state.player->m_is_jumping = true;
                            Mix_PlayChannel(-1, g_game_state.jump_sfx, 0);
                        }
                        break;

                    case SDLK_RETURN:   //I am playing with level switch here
                        // Jump
                        LOG("ret pressed"<<" "<<"lockLife: "<<lockLife<<std::endl);
                        if(!keyLock){
                            LOG("actually fired"<<'\n');
                            meta_lvl_count++;
                            handleMem();    //delete everything from previous level
                            meta_init();
                            keyLock = true;
                        }
                         //initialize differently based on level by calling meta_init
                        break;

                    default:
                        break;
                }

            default:
                break;





        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT] || key_state[SDL_SCANCODE_A])
    {
        g_game_state.player->move_left();
        g_game_state.player->m_animation_indices = g_game_state.player->m_walking[g_game_state.player->LEFT];
    }
    else if (key_state[SDL_SCANCODE_RIGHT] || key_state[SDL_SCANCODE_D])
    {
        g_game_state.player->move_right();
        g_game_state.player->m_animation_indices = g_game_state.player->m_walking[g_game_state.player->RIGHT];
    }

    // This makes sure that the player can't move faster diagonally
    if (glm::length(g_game_state.player->get_movement()) > 1.0f)
    {
        g_game_state.player->set_movement(glm::normalize(g_game_state.player->get_movement()));
    }
}

std::string instruction(){
    if(g_game_state.player->m_is_active){
    if(ENEMY_COUNT == enemyCount){
        return("Welcome to the game! run to the right!");
    }else if(enemyCount< ENEMY_COUNT && enemyCount > 0){
        return("Good job! Keep it up!");
    }else if(0 == enemyCount){
        return("Yesss! You killed 'em all!!! YESSS!");
    }else{
        return("Ooops something went wrong hahahah sorry orzzz!");
    }
    }
    return "";
}

//std::string guide(){
//    if(g_game_state.player->m_position.x>3){
//
//    }else(){
//
//    }
//}



void endMsg(){
    if(g_game_state.player->m_is_active){
        if(0 == enemyCount || meta_lvl_count>4){
            draw_text(&g_shader_program, g_text_texture_id, std::string("Winner!"), 0.35f, 0.0f,
                      glm::vec3(g_game_state.player->m_position.x, g_game_state.player->m_position.y+1.00f, 0.0f));
        }

    }else{  //player dead!
        draw_text(&g_shader_program, g_text_texture_id, std::string("Game Over"), 0.35f, 0.0f,
                  glm::vec3(g_game_state.player->m_position.x-1.00f, -1.0f, 0.0f));
    }
}


void checkContact(int ind){
    float xCon = fabs(g_game_state.player->m_position.x - g_game_state.enemies[ind].m_position.x);
    float yCon = fabs(g_game_state.player->m_position.y - g_game_state.enemies[ind].m_position.y);
    if(xCon<0.5f && yCon<1.2f ){
        meta_player_life --;
        g_game_state.player->set_position(glm::vec3(0.0f, 5.0f, 0.0f));//always airdrop player lol

        if(meta_player_life<=0){
            g_game_state.player->m_is_active=false;
        }
    }

}

void update()
{


    if(g_game_state.player->m_position.x>13){
        meta_lvl_count++;
        LOG("reached destination point"<<" "<<"lockLife: "<<lockLife<<std::endl);
        if(!keyLock){
            LOG("actually fired"<<'\n');
            meta_lvl_count++;
                handleMem();    //delete everything from previous level
                meta_init();
                keyLock = true;


        }
    }


//    LOG("player is at x pos: "<<g_game_state.player->m_position.x<<'\n');

    if(g_game_state.player->m_is_active == false) GameOver = true;  //game over!

    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    delta_time += g_accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP)
    {
        g_game_state.player->update(FIXED_TIMESTEP, g_game_state.player, NULL, 0, g_game_state.map);
        enemyCount = 0; //recount each time
        for (int i = 0; i < ENEMY_COUNT; i++) {
            g_game_state.enemies[i].update(FIXED_TIMESTEP, g_game_state.player, g_game_state.player, 1, g_game_state.map);
//            LOG(g_game_state.enemies[i].m_ai_type);
            checkContact(i);
            if(fabs(g_game_state.player->m_position.x - g_game_state.enemies[i].m_position.x) < 1.0f){
                if(g_game_state.enemies[i].m_collided_bottom){
                    g_game_state.enemies[i].m_is_jumping = true;
                }
//                LOG(fabs(g_game_state.player->m_position.x - g_game_state.enemies[i].m_position.x));
            }
            if(g_game_state.enemies[i].m_is_active){
                enemyCount ++;
            }
        }
        delta_time -= FIXED_TIMESTEP;
    }

    g_accumulator = delta_time;
    if(keyLock){
        if(delta_time-lockLife>0.01f){
            keyLock = false;
        }
        lockLife = delta_time;
    }

    g_view_matrix = glm::mat4(1.0f);
    g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_game_state.player->get_position().x, 0.0f, 0.0f));//moving camera to follow player
}

void render()
{
    g_shader_program.set_view_matrix(g_view_matrix);

    glClear(GL_COLOR_BUFFER_BIT);

    g_game_state.player->render(&g_shader_program);

    for (int i = 0; i < ENEMY_COUNT; i++)    {
        g_game_state.enemies[i].render(&g_shader_program);
    }

    g_game_state.map->render(&g_shader_program);


    /////////////////////////////////////added a ton of text as instructions///////////////////////////////////////////////
    draw_text(&g_shader_program, g_text_texture_id, std::string("Life Remaining: "), 0.25f, 0.0f, glm::vec3(g_game_state.player->m_position.x-3.00f, 3.0f, 0.0f));
    draw_text(&g_shader_program, g_text_texture_id, std::string(std::to_string(meta_player_life)), 0.25f, 0.0f, glm::vec3(g_game_state.player->m_position.x-3.00f, 2.8f, 0.0f));
    draw_text(&g_shader_program, g_text_texture_id, std::string(instruction()), 0.25f, 0.0f, glm::vec3((g_game_state.player->m_position.x-5.0f)/2, 2.0f, 0.0f));
//    draw_text(&g_shader_program, g_text_texture_id, std::string(guide()), 0.25f, 0.0f, glm::vec3(-4.00f, 1.5f, 0.0f));
    endMsg();
    if(meta_lvl_count<1){
        Menu menu;
        menu.render(&g_shader_program);
        draw_text(&g_shader_program, g_text_texture_id, "Hello!", 0.55f, 0.0f, glm::vec3(-1.20f, 1.0f, 0.0f));
        draw_text(&g_shader_program, g_text_texture_id, "Press Enter to start", 0.25f, 0.0f, glm::vec3(-2.4f, 0.0f, 0.0f));
    }

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();

    delete[] g_game_state.enemies;
    delete    g_game_state.player;
    delete    g_game_state.map;
    Mix_FreeChunk(g_game_state.jump_sfx);
    Mix_FreeMusic(g_game_state.bgm);
}

// ————— GAME LOOP ————— //
int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running )
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
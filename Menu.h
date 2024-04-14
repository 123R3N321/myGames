#include "Map.h"

class Menu
{
public: //public everything!!!!!
    // ––––– ANIMATION ––––– //
    int* m_animation_right = NULL, // move to the right
    * m_animation_left = NULL, // move to the left
    * m_animation_up   = NULL, // move upwards
    * m_animation_down = NULL; // move downwards

    // ––––– PHYSICS (GRAVITY) ––––– //
    glm::vec3 m_position;

    // ————— TRANSFORMATIONS ————— //
    glm::vec3 m_movement;
    glm::mat4 m_model_matrix;


    float m_width = 1;
    float m_height = 1;


public:

//    bool dead = false;  //this becomes true when entity falls out of map, position -4 or -5

    // ————— STATIC VARIABLES ————— //
    static const int    SECONDS_PER_FRAME = 4;
    static const int    LEFT    = 0,
            RIGHT   = 1,
            UP      = 2,
            DOWN    = 3;

    // ————— ANIMATION ————— /

    int m_animation_frames  = 0,
            m_animation_index   = 0,
            m_animation_cols    = 0,
            m_animation_rows    = 0;

    int*    m_animation_indices = NULL;
    float   m_animation_time = 0.0f;

    GLuint    m_texture_id;

    Menu();
    void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index);
    void render(ShaderProgram* program);

};
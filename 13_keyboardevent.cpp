#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define MOVE_SPEED 0.05

GLuint VBO;
GLuint IBO;
GLuint g_model;
GLuint g_view;
GLuint g_projection;

char* vertex_shader_path = "../shader/vertex_shader.vert";
char* fragment_shader_path = "../shader/fragment_shader.frag";

bool move_keys[4] = {false, false, false, false};

struct camera {
    glm::vec3 pos;
    glm::vec3 target;
    glm::vec3 up;
} cam = {
    glm::vec3(0., 0., -4.), 
    glm::vec3(0., 0., 0.), 
    glm::vec3(0., 1., 0.)
};


GLchar* read_shader_file(char* file_path)
{
    FILE* fp = fopen(file_path, "r");
    assert(fp != NULL);
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    GLchar* buffer = (GLchar*) malloc(size * sizeof(GLchar));
    fread(buffer, sizeof(GLchar), size, fp);
    return buffer;
}


void keyboard_press_callback(unsigned char key, int mouse_x, int mouse_y)
{
    switch (key)
    {
    case 27:
        exit(0);
        break;
    case 'w':
        move_keys[0] = true;
        break;
    case 's':
        move_keys[1] = true;
        break;
    case 'a':
        move_keys[2] = true;
        break;
    case 'd':
        move_keys[3] = true;
        break;
    default:
        break;
    }
}


void keyboard_release_callback(unsigned char key, int mouse_x, int mouse_y)
{
    switch (key)
    {
    case 27:
        exit(0);
        break;
    case 'w':
        move_keys[0] = false;
        break;
    case 's':
        move_keys[1] = false;
        break;
    case 'a':
        move_keys[2] = false;
        break;
    case 'd':
        move_keys[3] = false;
        break;
    default:
        break;
    }
}


void poll_camera_move()
{
    if (move_keys[0] == true) {
        cam.pos[2] += MOVE_SPEED;
        cam.target[2] += MOVE_SPEED;
    }
    if (move_keys[1] == true) {
        cam.pos[2] -= MOVE_SPEED;
        cam.target[2] -= MOVE_SPEED;
    }
    if (move_keys[2] == true) {
        cam.pos[0] += MOVE_SPEED;
        cam.target[0] += MOVE_SPEED;
        
    }
    if (move_keys[3] == true) {
        cam.pos[0] -= MOVE_SPEED;
        cam.target[0] -= MOVE_SPEED;
    }
}


glm::mat4 get_projection_matrix(float fov, float aspect_ratio, float n, float f)
{
    float top = n * tan(glm::radians(fov / 2));
    float bottom = -top;
    float right = top * aspect_ratio;
    float left = - right;

    glm::mat4 projection, scale, translate, perspective, mirror;
    scale = glm::mat4(
        2 / (right - left), 0, 0, 0,
        0, 2 / (top - bottom), 0, 0,
        0, 0, 2 / (n - f), 0,
        0, 0, 0, 1
    );
    
    translate = glm::mat4(
        1, 0, 0, -(right + left) / 2,
        0, 1, 0, -(top + bottom) / 2,
        0, 0, 1, (n + f) / 2,
        0, 0, 0, 1
    );

    perspective = glm::mat4(
        n, 0, 0, 0,
        0, n, 0, 0,
        0, 0, n + f, n * f,
        0, 0, -1, 0
    );
    
    mirror = glm::mat4(
        1, 0,  0, 0,
        0, 1,  0, 0,
        0, 0, -1, 0,
        0, 0,  0, 1
    );

    projection = perspective * translate * scale;
    
    return projection;
}


glm::mat4 get_look_at_matrix(glm::vec3 camera_pos, glm::vec3 target_pos, glm::vec3 up_pos)
{
    glm::vec3 front_vector = glm::normalize(target_pos - camera_pos);
    glm::vec3 right_vector = glm::normalize(glm::cross(front_vector, up_pos));
    glm::vec3 up_vector = glm::cross(right_vector, front_vector);

    glm::mat4 mat_left = glm::mat4(
        right_vector[0], right_vector[1], right_vector[2], 0.,
        up_vector[0], up_vector[1], up_vector[2], 0.,
        -front_vector[0], -front_vector[1], -front_vector[2], 0.,
        0., 0., 0., 1.
    );

    glm::mat4 mat_right = glm::mat4(
        1., 0., 0., -camera_pos[0], 
        0., 1., 0., -camera_pos[1], 
        0., 0., 1., -camera_pos[2], 
        0., 0., 0., 1.
    );

    glm::mat4 look_at_mat = mat_right * mat_left;
    
    return look_at_mat;
}


void render_scene_callback()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static float scale_value = 0.0;
    scale_value += 0.01;

    poll_camera_move();

    glm::mat4 model = glm::mat4(
        cos(scale_value), 0., - sin(scale_value), 0.,
        0., 1., 0., 0.,
        sin(scale_value), 0.,  cos(scale_value), 0.,
        0., 0., 0., 1.
    );
    glm::mat4 view = get_look_at_matrix(cam.pos, cam.target, cam.up);
    glm::mat4 projection = get_projection_matrix(45.0f, 1.78f, 0.1f, 100.f);

    glUniformMatrix4fv(g_model, 1, GL_TRUE, glm::value_ptr(model));
    glUniformMatrix4fv(g_view, 1, GL_TRUE, glm::value_ptr(view));
    glUniformMatrix4fv(g_projection, 1, GL_TRUE, glm::value_ptr(projection));

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(0);
    glutSwapBuffers();
}


void create_vertex_buffer()
{
    glm::vec3 vertex_list[4];
    vertex_list[0] = glm::vec3(-1., -1., -0.5);
    vertex_list[1] = glm::vec3(0., -1., 1.);
    vertex_list[2] = glm::vec3(1., -1., -0.5);
    vertex_list[3] = glm::vec3(0., 1., 0.);

    glGenBuffers(1, & VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_list), vertex_list, GL_STATIC_DRAW);

    std::cout << "[INFO] vertex buffer has been created." << std::endl;
}


void create_index_buffer()
{
    unsigned int index_list[12] = {
        0, 3, 1,
        1, 3, 2,
        2, 3, 0,
        0, 1, 2
    };

    glGenBuffers(1, & IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_list), index_list, GL_STATIC_DRAW);
}

void add_shader(GLuint shader_program, char* shader_text, GLenum shader_type)
{
    GLuint shader_obj = glCreateShader(shader_type);
    if (shader_obj == 0) {
        std::cerr << "ERROR create shader: " << shader_type << std::endl;
        exit(1);
    }

    GLchar* program[1];
    program[0] = shader_text;
    GLint lengths[1];
    lengths[0] = strlen(shader_text);
    glShaderSource(shader_obj, 1, program, lengths);
    glCompileShader(shader_obj);

    GLint success;
    glGetShaderiv(shader_obj, GL_COMPILE_STATUS, & success);
    if (! success) {
        GLchar info[1024];
        glGetShaderInfoLog(shader_obj, 1024, NULL, info);
        std::cerr << info << std::endl;
        exit(1);
    }

    glAttachShader(shader_program, shader_obj);
}

void compile_shaders()
{
    GLuint shader_program = glCreateProgram();
    if (shader_program == 0) {
        std::cerr << "ERROR create shader program." << std::endl;
        exit(1);
    }

    GLchar* vertex_shader = read_shader_file(vertex_shader_path);
    GLchar* fragment_shader = read_shader_file(fragment_shader_path);

    add_shader(shader_program, vertex_shader, GL_VERTEX_SHADER);
    add_shader(shader_program, fragment_shader, GL_FRAGMENT_SHADER);

    GLint success = 0;
    GLchar error[1024] = {0};
    glLinkProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, & success);
    if (success == 0) {
        glGetProgramInfoLog(shader_program, sizeof(error), NULL, error);
        std::cerr << "ERROR linking shader program: " << error << std::endl;
        exit(1);
    }

    glValidateProgram(shader_program);
    glGetProgramiv(shader_program, GL_VALIDATE_STATUS, & success);
    if (! success) {
        glGetProgramInfoLog(shader_program, sizeof(error), NULL, error);
        std::cerr << "ERROR invalid shader program: " << error << std::endl;
        exit(1);
    }
    glUseProgram(shader_program);
    g_model = glGetUniformLocation(shader_program, "g_model");
    assert(g_model != 0xFFFFFFFF);
    g_view = glGetUniformLocation(shader_program, "g_view");
    assert(g_view != 0xFFFFFFFF);
    g_projection = glGetUniformLocation(shader_program, "g_projection");
    assert(g_projection != 0xFFFFFFFF);
}


int main(int argc, char* argv[])
{
    glutInit(& argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

    glutInitWindowSize(1920, 1080);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Camera Space");

    glutKeyboardFunc(keyboard_press_callback);
    glutKeyboardUpFunc(keyboard_release_callback);
    glutDisplayFunc(render_scene_callback);
    glutIdleFunc(render_scene_callback);

    GLenum result = glewInit();
    if (result != GLEW_OK) {
        std::cerr << "ERROR: " << glewGetErrorString(result) << std::endl;
        return 1;
    }

    glEnable(GL_DEPTH_TEST);
    glClearColor(0., 0., 0., 0.);

    create_vertex_buffer();
    create_index_buffer();
    compile_shaders();

    glutMainLoop();
    return 0;
}

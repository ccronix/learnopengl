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

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.hpp"

#define MOVE_SPEED 0.05f
#define MOUSE_SPEED 0.05

#define SIZE_WIDTH 1920
#define SIZE_HEIGHT 1080

#define TEX_PATH "texture.png"

GLuint VBO;
GLuint IBO;
GLuint TEX;
GLuint g_model;
GLuint g_view;
GLuint g_projection;
GLuint g_sampler;

struct {
    GLuint color;
    GLuint direction;
    GLuint intensity;
} g_parallel_light;

GLuint g_camera_pos;
GLuint g_specular;

float specular = 1.0f;

char* vertex_shader_path = "../shader/vtx_specular_shader.vert";
char* fragment_shader_path = "../shader/frag_specular_shader.frag";

bool move_keys[4] = {false, false, false, false};

GLfloat pitch = 0.f, yaw = 0.f;

struct camera {
    glm::vec3 pos;
    glm::vec3 target;
    glm::vec3 up;
} cam = {
    glm::vec3(0., 0., 4.), 
    glm::vec3(0., 0., -1.), 
    glm::vec3(0., 1., 0.),
};


struct vertex {
    glm::vec3 position;
    glm::vec2 texcoord;
    glm::vec3 normal;

    vertex() {}

    vertex(glm::vec3 pos, glm::vec2 uv, glm::vec3 nrm) {
        position = pos;
        texcoord = uv;
        normal = nrm;
    }
};


struct AmbientLight {
    glm::vec3 color;
    float intensity;

    AmbientLight() {}

    AmbientLight(glm::vec3 clr, float ist) {
        color = clr;
        intensity = ist;
    }
};


struct ParallelLight {
    glm::vec3 color;
    glm::vec3 direction;
    float intensity;

    ParallelLight() {}

    ParallelLight(glm::vec3 clr, glm::vec3 dir, float ist) {
        color = clr;
        direction = dir;
        intensity = ist;
    }
};


struct PointLight {
    glm::vec3 color;
    glm::vec3 position;
    float intensity;

    PointLight() {}
    
    PointLight(glm::vec3 clr, glm::vec3 pos, float ist) {
        color = clr;
        position = pos;
        intensity = ist;
    }
};


ParallelLight parallel_light = ParallelLight(
    glm::vec3(1.0), 
    glm::vec3(1., -1., 0.), 
    1.5f
);


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


void mouse_move_callback(int mouse_x, int mouse_y)
{
    yaw += 50 * (mouse_x - SIZE_WIDTH / 2) / (GLfloat) SIZE_WIDTH;
    yaw = glm::mod(yaw + 180.f, 360.f) - 180.f;

    pitch += -50 * (mouse_y - SIZE_HEIGHT / 2) / (GLfloat) SIZE_HEIGHT;
    pitch = glm::clamp(pitch, -89.f, 89.f);

    glutWarpPointer(SIZE_WIDTH / 2, SIZE_HEIGHT / 2);
    glutPostRedisplay();
}


void poll_camera_move()
{
    if (move_keys[0] == true) {
        cam.pos += MOVE_SPEED * cam.target;
    }
    if (move_keys[1] == true) {
        cam.pos -= MOVE_SPEED * cam.target;
    }
    if (move_keys[2] == true) {
        cam.pos -= MOVE_SPEED * glm::normalize(glm::cross(cam.target, cam.up));
    }
    if (move_keys[3] == true) {
        cam.pos += MOVE_SPEED * glm::normalize(glm::cross(cam.target, cam.up));
    }
    glm::vec3 direction = glm::vec3(
        cos(glm::radians(pitch)) * sin(glm::radians(yaw)),
        sin(glm::radians(pitch)),
        -cos(glm::radians(pitch)) * cos(glm::radians(yaw))
    );
    cam.target = direction;
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
    glm::mat4 view = get_look_at_matrix(cam.pos, cam.pos + cam.target, cam.up);
    glm::mat4 projection = get_projection_matrix(45.0f, 1.78f, 0.1f, 100.f);

    glUniform3f(g_camera_pos, cam.pos.r, cam.pos.g, cam.pos.b);

    glUniformMatrix4fv(g_model, 1, GL_TRUE, glm::value_ptr(model));
    glUniformMatrix4fv(g_view, 1, GL_TRUE, glm::value_ptr(view));
    glUniformMatrix4fv(g_projection, 1, GL_TRUE, glm::value_ptr(projection));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (const GLvoid*) 12);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (const GLvoid*) 20);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TEX);

    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glutSwapBuffers();
}


void create_vertex_buffer()
{
    vertex vertex_list[4] = {
        vertex(glm::vec3(-1., -1., -0.5), glm::vec2(0., 0.), glm::vec3(0.801784, -0.267261, -0.534522)),
        vertex(glm::vec3(0., -1., 1.), glm::vec2(0.5, 0.), glm::vec3(-0.801784, -0.267261, -0.534522)),
        vertex(glm::vec3(1., -1., -0.5), glm::vec2(1., 0.), glm::vec3(0., -0.242536, 0.970143)),
        vertex(glm::vec3(0., 1., 0.), glm::vec2(0.5, 1.), glm::vec3(0., 1., 0.))
    };

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


void create_texture_buffer(const char* image_path)
{
    glUniform1i(g_sampler, 0);
    glGenTextures(1, & TEX);
    glBindTexture(GL_TEXTURE_2D, TEX);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, channels;
    unsigned char* content = stbi_load(image_path, & width, & height, & channels, 0);

    if (content) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, content);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cerr << "[ERROR] loading texture failed." << std::endl;
    }
    stbi_image_free(content);
}


void create_light_uniform_variable()
{
    glUniform3f(g_parallel_light.color, parallel_light.color.r, parallel_light.color.g, parallel_light.color.b);
    glUniform3f(g_parallel_light.direction, parallel_light.direction.r, parallel_light.direction.g, parallel_light.direction.b);
    glUniform1f(g_parallel_light.intensity, parallel_light.intensity);
    glUniform1f(g_specular, specular);
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
    g_sampler = glGetUniformLocation(shader_program, "g_sampler");
    assert(g_sampler != 0xFFFFFFFF);
    g_parallel_light.color = glGetUniformLocation(shader_program, "g_parallel_light.color");
    assert(g_parallel_light.color != 0xFFFFFFFF);
    g_parallel_light.direction = glGetUniformLocation(shader_program, "g_parallel_light.direction");
    assert(g_parallel_light.direction != 0xFFFFFFFF);
    g_parallel_light.intensity = glGetUniformLocation(shader_program, "g_parallel_light.intensity");
    assert(g_parallel_light.intensity != 0xFFFFFFFF);
    g_camera_pos = glGetUniformLocation(shader_program, "g_camera_pos");
    assert(g_camera_pos != 0xFFFFFFFF);
    g_specular = glGetUniformLocation(shader_program, "g_specular");
    assert(g_specular != 0xFFFFFFFF);
}


int main(int argc, char* argv[])
{
    glutInit(& argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

    glutInitWindowSize(SIZE_WIDTH, SIZE_HEIGHT);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Camera Space");
    glutSetCursor(GLUT_CURSOR_NONE);
    glutWarpPointer(SIZE_WIDTH / 2, SIZE_HEIGHT / 2);
    
    glutKeyboardFunc(keyboard_press_callback);
    glutPassiveMotionFunc(mouse_move_callback);
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

    compile_shaders();
    
    create_vertex_buffer();
    create_index_buffer();
    create_texture_buffer(TEX_PATH);
    create_light_uniform_variable();

    glutMainLoop();
    return 0;
}

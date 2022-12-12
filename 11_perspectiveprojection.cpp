#include <cmath>
#include <iostream>
#include <assert.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

GLuint VBO;
GLuint IBO;
GLuint g_model;
GLuint g_view;
GLuint g_projection;

GLchar* vertex_shader = "\
#version 330 \n \
layout (location = 0) in vec3 Position; \n \
uniform mat4 g_model; \n \
uniform mat4 g_view; \n \
uniform mat4 g_projection; \n \
out vec4 color;\n \
void main() \n \
{ \n \
    gl_Position = g_projection * g_view * g_model * vec4(Position, 1.0); \n \
    color = vec4(clamp(Position, 0.0, 1.0), 1.0);\n \
} \n \
";

GLchar* fragment_shader = "\
#version 330 \n \
in vec4 color;\n \
out vec4 FragColor; \n \
void main() \n \
{ \n \
    FragColor = color; \n \
} \n \
";


glm::mat4 get_projection_matrix(float eye_fov, float aspect_ratio, float zNear, float zFar)
{
    // TODO: Use the same projection matrix from the previous assignments
    float half_fov_arc = glm::radians(eye_fov / 2);
    float top = zNear * tan(half_fov_arc);
    float bottom = -top;
    float right = top * aspect_ratio;
    float left = - right;

    glm::mat4 projection, scale, translate, perspective, mirror;
    scale = glm::mat4(
        2 / (right - left), 0, 0, 0,
        0, 2 / (top - bottom), 0, 0,
        0, 0, 2 / (zNear - zFar), 0,
        0, 0, 0, 1
    );
    
    translate = glm::mat4(
        1, 0, 0, -(right + left) / 2,
        0, 1, 0, -(top + bottom) / 2,
        0, 0, 1, -(zNear + zFar) / 2,
        0, 0, 0, 1
    );

    perspective = glm::mat4(
        zNear, 0, 0, 0,
        0, zNear, 0, 0,
        0, 0, zNear + zFar, - zNear * zFar,
        0, 0, 1, 0
    );
    
    mirror = glm::mat4(
        1, 0,  0, 0,
        0, 1,  0, 0,
        0, 0, -1, 0,
        0, 0,  0, 1
    );

    projection =  perspective * translate * scale;
    return projection;

}


void render_scene_callback()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static float scale_value = 0.0;
    scale_value += 0.01;

    glm::mat4 model = glm::mat4(
        cos(scale_value), 0., - sin(scale_value), 0.,
        0., 1., 0., 0.,
        sin(scale_value), 0.,  cos(scale_value), 0.,
        0., 0., 0., 1.
    );

    glm::mat4 view = glm::mat4(
        1., 0., 0., 0.,
        0., 1., 0., 0.,
        0., 0., 1., -5.,
        0., 0., 0., 1.
    );

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.78f, 0.1f, 100.0f);
    // glm::mat4 projection = get_projection_matrix(45.f, 1.78f, 0.1f, 50.f);

    // for (int i = 0; i < 4; i++) {
    //     for (int j = 0; j < 4; j++) {
    //         std::cout << projection[i][j] << " ";
    //     }
    //     std::cout << std::endl;
    // }

    glUniformMatrix4fv(g_model, 1, GL_TRUE, glm::value_ptr(model));
    glUniformMatrix4fv(g_view, 1, GL_TRUE, glm::value_ptr(view));
    glUniformMatrix4fv(g_projection, 1, GL_FALSE, glm::value_ptr(projection));

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
    vertex_list[0] = glm::vec3(-1., -1., 0.);
    vertex_list[1] = glm::vec3(0., -1., 0.5);
    vertex_list[2] = glm::vec3(1., -1., 0.);
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
    glutCreateWindow("Index Draw");

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

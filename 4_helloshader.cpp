#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "fvector.hpp"

GLuint VBO;

GLchar* vertex_shader = "\
#version 330 \n \
layout (location = 0) in vec3 Position; \n \
void main() \n \
{ \n \
    gl_Position = vec4(Position.x, Position.y, Position.z, 1.0); \n \
} \n \
";

GLchar* fragment_shader = "\
#version 330 \n \
out vec4 FragColor; \n \
void main() \n \
{ \n \
    FragColor = vec4(0.3, 0.4, 0.5, 1.0); \n \
} \n \
";


void render_scene_callback()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisableVertexAttribArray(0);
    glutSwapBuffers();
    glFlush();
}


void create_vertex_buffer()
{
    Vector3f vertex_list[3];
    vertex_list[0] = Vector3f(-0.6, -0.6, 0.);
    vertex_list[1] = Vector3f(0.6, -0.6, 0.);
    vertex_list[2] = Vector3f(0., 0.6, 0.);
    glGenBuffers(1, & VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_list), vertex_list, GL_STATIC_DRAW);
    std::cout << vertex_list[0] << std::endl;
    std::cout << vertex_list[1] << std::endl;
    std::cout << vertex_list[2] << std::endl;
    std::cout << sizeof(vertex_list) << std::endl;
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
}


int main(int argc, char* argv[])
{
    glutInit(& argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(640, 360);
    glutCreateWindow("Hello Shader Window");

    glutDisplayFunc(render_scene_callback);

    GLenum result = glewInit();
    if (result != GLEW_OK) {
        std::cerr << "ERROR: " << glewGetErrorString(result) << std::endl;
        return 1;
    }
    
    glClearColor(0., 0., 0., 0.);

    create_vertex_buffer();
    compile_shaders();

    glutMainLoop();
    return 0;
}
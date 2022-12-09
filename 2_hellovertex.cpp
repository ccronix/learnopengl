#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "fvector.hpp"

GLuint VBO;


void render_scene_callback()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glPointSize(30.f);
    glDrawArrays(GL_POINTS, 0, 1);

    glDisableVertexAttribArray(0);
    glutSwapBuffers();
}


void create_vertex_buffer()
{
    Vector3f vertex_list[1];
    vertex_list[0] = Vector3f(0., 0., 0.);
    glGenBuffers(1, & VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_list), vertex_list, GL_STATIC_DRAW);
}


int main(int argc, char* argv[])
{
    glutInit(& argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(640, 360);
    glutCreateWindow("Hello Vertex Window");

    glutDisplayFunc(render_scene_callback);

    GLenum result = glewInit();
    if (result != GLEW_OK) {
        std::cerr << "ERROR: " << glewGetErrorString(result) << std::endl;
        return 1;
    }

    glClearColor(0., 0., 0., 0.);
    create_vertex_buffer();

    glutMainLoop();
    return 0;
}
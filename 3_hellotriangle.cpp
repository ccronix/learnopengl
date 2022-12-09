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

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisableVertexAttribArray(0);
    glutSwapBuffers();
    glFlush();
}


void create_vertex_buffer()
{
    Vector3f vertex_list[3];
    vertex_list[0] = Vector3f(-0.5, -0.5, 0.);
    vertex_list[1] = Vector3f(0.5, -0.5, 0.);
    vertex_list[2] = Vector3f(0., 0.5, 0.);
    glGenBuffers(1, & VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_list), vertex_list, GL_STATIC_DRAW);
    std::cout << vertex_list[0] << std::endl;
    std::cout << vertex_list[1] << std::endl;
    std::cout << vertex_list[2] << std::endl;
    std::cout << sizeof(vertex_list) << std::endl;
}


int main(int argc, char* argv[])
{
    glutInit(& argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(640, 360);
    glutCreateWindow("Hello Triangle Window");

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
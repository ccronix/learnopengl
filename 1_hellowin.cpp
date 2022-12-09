#include <GL/freeglut.h>


void render_scene_callback()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glutSwapBuffers();
}


int main(int argc, char* argv[])
{
    glutInit(& argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(640, 360);
    glutCreateWindow("Hello OpenGL Window");

    glutDisplayFunc(render_scene_callback);

    glClearColor(0.5, 0.5, 0.5, 0.5);
    glutMainLoop();
    return 0;
}
g++ -static 10_indexdraw.cpp -D FREEGLUT_STATIC -D GLEW_STATIC -I . -I C:\Users\Cronix\Documents\cronix_dev\lib_install\freeglut_3.4_mingw\include -I C:\Users\Cronix\Documents\cronix_dev\lib_install\glew_2.1_mingw\include -I C:\Users\Cronix\Documents\cronix_dev\lib_install\glm_0.9.9.5\include -L C:\Users\Cronix\Documents\cronix_dev\lib_install\freeglut_3.4_mingw\lib -L C:\Users\Cronix\Documents\cronix_dev\lib_install\glew_2.1_mingw\lib -L "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\um\x64" -lfreeglut_static -lglew32 -lopengl32 -lwinmm -lgdi32 -o build\indexdraw
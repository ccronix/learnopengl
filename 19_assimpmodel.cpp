#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cassert>

#include <vector>
#include <string>
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.hpp"

#define MOVE_SPEED 0.5f
#define MOUSE_SPEED 0.05

#define SIZE_WIDTH 1920
#define SIZE_HEIGHT 1080

#define MAX_POINT_LIGHT 2

#define VERTEX_SHADER "../shader/vtx_specular_shader.vert"
#define FRAGMENT_SHADER "../shader/frag_point_shader.frag"

#define SCENE_PATH "C:/Users/Cronix/Documents/cronix_dev/mcguire_cg_archive/erato/erato.obj"

// #define LOAD_TEXTURE


GLuint g_model;
GLuint g_view;
GLuint g_projection;
GLuint g_sampler;

struct {
    GLuint color;
    GLuint direction;
    GLuint intensity;
} g_parallel_light;

struct {
    GLuint color;
    GLuint position;

    GLuint constant;
    GLuint linear;
    GLuint quadratic;
} g_point_light_list[MAX_POINT_LIGHT];

GLuint g_point_light_num;
GLuint g_camera_pos;
GLuint g_specular;

float specular = 1.0f;

bool move_keys[4] = {false, false, false, false};

GLfloat pitch = 0.f, yaw = 0.f;

struct camera {
    glm::vec3 pos;
    glm::vec3 target;
    glm::vec3 up;
} cam = {
    glm::vec3(0., 18., 60.), 
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


class texture {

public:
    GLuint TEX;
    GLuint sampler;

    const char* image_path;
    unsigned char* content;

    int width;
    int height;
    int channels;

    bool loaded;

    texture() { load_default_color(); }

    texture(const char* path, GLuint spl) {
            image_path = path;
            sampler = spl;
            loaded = true;
            load_image();
    }

    ~texture() {}

    void load_image()
    {
        content = stbi_load(image_path, & width, & height, & channels, 0);

        if (!content) {
            std::cerr << "[WARNING] can not load image: " << image_path << std::endl;
            load_default_color();
            loaded = false;
        }

        create_texture_buffer();
    }

    void load_default_color()
    {
        width = 1024;
        height = 1024;
        channels = 3;
        content = (unsigned char*) malloc(sizeof(unsigned char) * width * height * channels);
        memset(content, 128, width * height * channels);

        create_texture_buffer();
    }   

    void create_texture_buffer()
    {
        glUniform1i(sampler, 0);
        glGenTextures(1, & TEX);
        glBindTexture(GL_TEXTURE_2D, TEX);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, content);
        glGenerateMipmap(GL_TEXTURE_2D);
        set_texture_parameter();
    }

    void set_texture_parameter()
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
};


class mesh {

public:
    std::vector<vertex> vertices;
    std::vector<unsigned int> indices;
    texture mesh_tex;

    GLuint VBO;
    GLuint IBO;

    mesh() {}

    mesh(std::vector<vertex> vtxs, std::vector<unsigned int> idxs, texture& tex) 
    {
        vertices = vtxs;
        indices = idxs;
        mesh_tex = tex;

        create_vertex_buffer();
        create_index_buffer();
    }

    void create_vertex_buffer()
    {
        int buffer_size = sizeof(vertex) * vertices.size();

        glGenBuffers(1, & VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, buffer_size, &vertices[0], GL_STATIC_DRAW);
    }

    void create_index_buffer()
    {
        int buffer_size = sizeof(unsigned int) * indices.size();

        glGenBuffers(1, & IBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer_size, &indices[0], GL_STATIC_DRAW);

    }
};


class scene {

public:
    std::string scene_path;
    std::string scene_dir;

    std::vector<mesh> scene_meshes;
    std::vector<texture> scene_textures;

    scene() {}

    scene(std::string path)
    {
        scene_path = path;
        scene_dir = get_scene_dir();

        load_scene();
    }


    std::string get_scene_dir()
    {
        std::string::size_type slash_index = scene_path.find_last_of("/");
        std::string scene_dir;

        if (slash_index == std::string::npos) {
            scene_dir = ".";
        }
        else if (slash_index == 0) {
            scene_dir = "/";
        }
        else {
            scene_dir = scene_path.substr(0, slash_index);
        }
        return scene_dir;
    }

    void load_scene()
    {
        Assimp::Importer importer;

        const aiScene* scene_ptr = importer.ReadFile(
            scene_path.c_str(), 
            aiProcess_Triangulate |
            aiProcess_GenSmoothNormals | 
            aiProcess_FlipUVs
        );

        if (!scene_ptr) {
            std::cerr << "[ERROR] load scene failed:" << scene_path << std::endl;
            std::cerr << importer.GetErrorString();
            exit(1);
        }

        init_material(scene_ptr);
        init_scene(scene_ptr);
    }

    void init_scene(const aiScene* scn)
    {

        for (int i = 0; i < scn->mNumMeshes; i++) {
            aiMesh* msh = scn->mMeshes[i];
            scene_meshes.push_back(init_mesh(msh));
        }
    }

    mesh init_mesh(aiMesh* msh)
    {
        std::vector<vertex> vertices;
        aiVector3D default_uv(0., 0., 0.);
        for (int i = 0; i < msh->mNumVertices; i++) {
            aiVector3D position = msh->mVertices[i];
            aiVector3D normal = msh->mNormals[i];
            aiVector3D uv = msh->HasTextureCoords(0) ? msh->mTextureCoords[0][i] : default_uv;

            vertex vtx(
                glm::vec3(position.x, position.y, position.z),
                glm::vec2(uv.x, uv.y),
                glm::vec3(normal.x, normal.y, normal.z)
            );
            vertices.push_back(vtx);
        }
        std::vector<unsigned int> indices = init_indices(msh);
        unsigned int index = msh->mMaterialIndex;

        #ifdef LOAD_TEXTURE
            texture tex = scene_textures[index];
        #else
            texture tex = texture();
        #endif
    
        return mesh(vertices, indices, tex);
    }

    std::vector<unsigned int> init_indices(aiMesh* mesh)
    {
        std::vector<unsigned int> indices;
        for (int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        return indices;
    }

    void init_material(const aiScene* scn)
    {
        for (int i = 0; i < scn->mNumMaterials; i++) {
            const aiMaterial* mat = scn->mMaterials[i];
            if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
                aiString path;
                if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                    std::string abs_path = scene_dir + "/" + path.data;
                    scene_textures.push_back(texture(abs_path.c_str(), g_sampler));
                }
            }
        }
    }
};


scene base_scene;


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

    float constant;
    float linear;
    float quadratic;

    PointLight() {}
    
    PointLight(glm::vec3 clr, glm::vec3 pos, float cst, float lin, float quad) {
        color = clr;
        position = pos;

        constant = cst;
        linear = lin;
        quadratic = quad;
    }
};


ParallelLight parallel_light = ParallelLight(
    glm::vec3(1.0), 
    glm::vec3(1., -1., 0.), 
    1.5f
);


PointLight point_light_list[MAX_POINT_LIGHT] = {
    PointLight(
        glm::vec3(0.0),
        glm::vec3(0.0),
        0.0,
        0.0,
        0.0
    )
};


PointLight point_light_a = PointLight(
    glm::vec3(3.0, 4.0, 5.0),
    glm::vec3(8.0, 25.0, -4.0),
    0.0,
    1.0,
    0.0
);


PointLight point_light_b = PointLight(
    glm::vec3(4.0, 4.0, 3.0),
    glm::vec3(-6.0, 30.0, 3.0),
    1.0,
    0.6,
    0.3
);


void create_point_lights()
{
    point_light_list[0] = point_light_a;
    point_light_list[1] = point_light_b;
}


GLchar* read_shader_file(const char* file_path)
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

    for (int i = 0; i < base_scene.scene_meshes.size(); i++) {
        mesh msh = base_scene.scene_meshes[i];
        glBindBuffer(GL_ARRAY_BUFFER, msh.VBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), 0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (const GLvoid*) 12);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (const GLvoid*) 20);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, msh.IBO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, msh.mesh_tex.TEX);

        glDrawElements(GL_TRIANGLES, msh.indices.size(), GL_UNSIGNED_INT, 0);
    }
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glutSwapBuffers();
}


void create_texture_buffer(GLuint& TEX, const char* image_path)
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

    create_point_lights();
    glUniform1i(g_point_light_num, MAX_POINT_LIGHT);

    for (int i = 0; i < MAX_POINT_LIGHT; i++) {
        glUniform3f(g_point_light_list[i].color, point_light_list[i].color.r, point_light_list[i].color.g, point_light_list[i].color.b);
        glUniform3f(g_point_light_list[i].position, point_light_list[i].position.r, point_light_list[i].position.g, point_light_list[i].position.b);
        glUniform1f(g_point_light_list[i].constant, point_light_list[i].constant);
        glUniform1f(g_point_light_list[i].linear, point_light_list[i].linear);
        glUniform1f(g_point_light_list[i].quadratic, point_light_list[i].quadratic);
    }
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

    GLchar* vertex_shader = read_shader_file(VERTEX_SHADER);
    GLchar* fragment_shader = read_shader_file(FRAGMENT_SHADER);

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
    g_point_light_num = glGetUniformLocation(shader_program, "g_point_light_num");
    assert(g_specular != 0xFFFFFFFF);
    char buffer[100];
    for (int i = 0; i < MAX_POINT_LIGHT; i++) {
        sprintf(buffer, "g_point_light_list[%d].color", i);
        g_point_light_list[i].color = glGetUniformLocation(shader_program, buffer);
        assert(g_specular != 0xFFFFFFFF);
        sprintf(buffer, "g_point_light_list[%d].position", i);
        g_point_light_list[i].position = glGetUniformLocation(shader_program, buffer);
        assert(g_specular != 0xFFFFFFFF);
        sprintf(buffer, "g_point_light_list[%d].constant", i);
        g_point_light_list[i].constant = glGetUniformLocation(shader_program, buffer);
        assert(g_specular != 0xFFFFFFFF);
        sprintf(buffer, "g_point_light_list[%d].linear", i);
        g_point_light_list[i].linear = glGetUniformLocation(shader_program, buffer);
        assert(g_specular != 0xFFFFFFFF);
        sprintf(buffer, "g_point_light_list[%d].quadratic", i);
        g_point_light_list[i].quadratic = glGetUniformLocation(shader_program, buffer);
        assert(g_specular != 0xFFFFFFFF);
    }
}


int main(int argc, char* argv[])
{
    glutInit(& argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

    glutInitWindowSize(SIZE_WIDTH, SIZE_HEIGHT);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Assimp Model");
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

    base_scene = scene(SCENE_PATH);
    
    create_light_uniform_variable();

    glutMainLoop();
    return 0;
}

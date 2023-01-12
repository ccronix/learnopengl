#version 330

in vec2 uv_coord;
in vec3 normal;
in vec3 point;

out vec4 FragColor;


struct AmbientLight {
    vec3 color;
    float intensity;
};

struct ParallelLight {
    vec3 color;
    vec3 direction;
    float intensity;
};

struct PointLight {
    vec3 color;
    vec3 position;
    float intensity;
};


uniform vec3 g_camera_pos;
uniform float g_specular;
uniform sampler2D g_sampler;
uniform ParallelLight g_parallel_light;


void main()
{
    vec4 ambient_light, diffuse_light, specular_light;
    ambient_light = vec4(0.1, 0.1, 0.1, 1.0);
    float diffuse_factor = dot(normalize(normal), -normalize(g_parallel_light.direction));
    if (diffuse_factor > 0) {
        diffuse_light = vec4(g_parallel_light.color, 1.0) * g_parallel_light.intensity * diffuse_factor;

        vec3 camera_direction = normalize(g_camera_pos - point);
        vec3 reflection = normalize(reflect(g_parallel_light.direction, normal));
        float specular_factor = dot(camera_direction, reflection);

        if (specular_factor > 0) {
            specular_light = vec4(g_parallel_light.color, 1.0) * specular_factor * g_specular;
        }
        else {
            specular_light = vec4(0, 0, 0, 0);
        }
    }
    else {
        diffuse_light = vec4(0, 0, 0, 0);
    }
    FragColor = texture(g_sampler, uv_coord) * (ambient_light + diffuse_light + specular_light);
}

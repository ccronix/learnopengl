#version 330

in vec2 uv_coord;
in vec3 normal;

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


uniform sampler2D g_sampler;
uniform ParallelLight g_parallel_light;


void main()
{
    vec4 diffuse_light;
    float diffuse_factor = dot(normalize(normal), -normalize(g_parallel_light.direction));
    if (diffuse_factor > 0) {
        diffuse_light = vec4(g_parallel_light.color, 1.0) * g_parallel_light.intensity * diffuse_factor;
    }
    else {
        diffuse_light = vec4(0, 0, 0, 0);
    }
    FragColor = texture(g_sampler, uv_coord) * diffuse_light;
}

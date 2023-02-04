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

    float constant;
    float linear;
    float quadratic;
};


uniform vec3 g_camera_pos;
uniform float g_specular;
uniform int g_point_light_num;
uniform sampler2D g_sampler;
uniform ParallelLight g_parallel_light;
uniform PointLight g_point_light_list[2];


vec4 parallel_diffuse(ParallelLight parallel_light, vec3 nrm)
{
    float factor = dot(normalize(normal), -normalize(parallel_light.direction));
    if (factor > 0) {
        return vec4(parallel_light.color, 1.0) * parallel_light.intensity * factor;
    }
    else {
        return vec4(0, 0, 0, 0);
    }
}


vec4 parallel_specular(ParallelLight parallel_light, vec3 cam_pos, vec3 obj_pos, vec3 nrm)
{
    vec3 camera_direction = normalize(cam_pos - obj_pos);
    vec3 reflection = normalize(reflect(parallel_light.direction, normal));
    float factor = dot(camera_direction, reflection);
    if (factor > 0) {
        return vec4(parallel_light.color, 1.0) * factor * g_specular;
    }
    else {
        return vec4(0, 0, 0, 0);
    }
}


vec4 calc_parallel_light(ParallelLight parallel_light, vec3 cam_pos, vec3 obj_pos, vec3 nrm)
{
    vec4 diffuse_light, specular_light;
    diffuse_light = parallel_diffuse(parallel_light, nrm);
    if (diffuse_light != vec4(0, 0, 0, 0)) {
        specular_light = parallel_specular(parallel_light, cam_pos, obj_pos, nrm);
    }
    else {
        specular_light = vec4(0, 0, 0, 0);
    }
    return diffuse_light + specular_light;
}


vec4 point_diffuse(PointLight point_light, vec3 obj_pos, vec3 nrm)
{
    vec3 light_direction = point_light.position - obj_pos;
    float factor = dot(normalize(normal), -normalize(light_direction));
    if (factor > 0) {
        return vec4(point_light.color, 1.0) * factor;
    }
    else {
        return vec4(0, 0, 0, 0);
    }
}


vec4 point_specular(PointLight point_light, vec3 cam_pos, vec3 obj_pos, vec3 nrm)
{
    vec3 camera_direction = normalize(cam_pos - obj_pos);
    vec3 light_direction = point_light.position - obj_pos;
    vec3 reflection = normalize(reflect(light_direction, normal));
    float factor = dot(camera_direction, reflection);
    if (factor > 0) {
        return vec4(point_light.color, 1.0) * factor * g_specular;
    }
    else {
        return vec4(0, 0, 0, 0);
    }
}

vec4 calc_point_light(PointLight point_light, vec3 cam_pos, vec3 obj_pos, vec3 nrm)
{
    float light_distance = length(point_light.position - obj_pos);
    float attenuation = point_light.constant
     + point_light.linear * light_distance 
     + point_light.quadratic * pow(point_light.quadratic, 2);

    vec4 diffuse = point_diffuse(point_light, obj_pos, nrm);
    vec4 specular = point_specular(point_light, cam_pos, obj_pos, nrm);
    return (diffuse + specular) / attenuation;
}


void main()
{
    vec4 ambient_light, parallel_light, point_light;
    ambient_light = vec4(0.1, 0.1, 0.1, 1.0);
    parallel_light = calc_parallel_light(g_parallel_light, g_camera_pos, point, normal);
    for (int i = 0; i < g_point_light_num; i++) {
        point_light += calc_point_light(g_point_light_list[i], g_camera_pos, point, normal);
    }
    FragColor = texture(g_sampler, uv_coord) * (ambient_light + parallel_light + point_light);
}

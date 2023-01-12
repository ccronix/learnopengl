#version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoord;
layout (location = 2) in vec3 Normal;

uniform mat4 g_model;
uniform mat4 g_view;
uniform mat4 g_projection;

out vec2 uv_coord;
out vec3 normal;


void main()
{
    gl_Position = g_projection * g_view * g_model * vec4(Position, 1.0);
    uv_coord = TexCoord;
    normal = (g_model * vec4(Normal, 0.0)).xyz;
}

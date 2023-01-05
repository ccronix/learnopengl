#version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoord;

uniform mat4 g_model;
uniform mat4 g_view;
uniform mat4 g_projection;

out vec4 color;
out vec2 uv_coord;

void main()
{
    gl_Position = g_projection * g_view * g_model * vec4(Position, 1.0);
    uv_coord = TexCoord;
}
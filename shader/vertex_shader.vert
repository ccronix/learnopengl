#version 330

layout (location = 0) in vec3 Position;

uniform mat4 g_model;
uniform mat4 g_view;
uniform mat4 g_projection;

out vec4 color;

void main()
{
    gl_Position = g_projection * g_view * g_model * vec4(Position, 1.0);
    color = vec4(clamp(Position, 0.0, 1.0), 1.0);
}
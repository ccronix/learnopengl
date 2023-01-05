#version 330

in vec2 uv_coord;

out vec4 FragColor;

uniform sampler2D g_sampler;

void main()
{
    FragColor = texture(g_sampler, uv_coord);
}
#version 410 core

in block
{
    vec2 Texcoord;
    vec2 Position;
} In;

uniform vec3 LineColor;

layout(location = 0, index = 0) out vec4  Color;

void main(void)
{
    Color = vec4(LineColor, 1);
}
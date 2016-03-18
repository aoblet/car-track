#version 410 core

in block
{
    vec2 Texcoord;
    vec2 Position;
} In;

uniform sampler2D Texture;
uniform mat3 Homography;

layout(location = 0, index = 0) out vec4  Color;

void main(void)
{
    vec2 coord = In.Texcoord;
    vec3 texcoord = inverse(Homography) * vec3(coord, 1);
    texcoord /= texcoord.z;

    Color = texture(Texture, texcoord.xy);
}
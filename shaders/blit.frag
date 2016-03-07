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
    vec2 pos = In.Position;
    vec3 texcoord = inverse(Homography) * vec3(pos, 1);
    texcoord /= texcoord.z;
    texcoord = texcoord * 0.5 + 0.5;
    Color = texture(Texture, texcoord.xy);
}
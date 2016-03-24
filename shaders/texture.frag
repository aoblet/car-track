#version 410 core

in block{
    vec2 Texcoord;
    vec2 Position;

} In;

out vec4 Color;

uniform sampler2D Texture;

void main(){
    Color = texture(Texture, In.Texcoord);
}
#version 410 core

in block{
    vec2 Texcoord;
} In;

out vec4 Color;

uniform sampler2D A;
uniform sampler2D B;


void main(){
    Color = texture(A, In.Texcoord) + texture(B, In.Texcoord);
}
#version 410 core
in block{
    vec2 Texcoord;
    vec2 Position;
} In;

uniform float Random;
uniform sampler2D Texture;
layout(location = 0, index = 0) out vec4  Color;

float rand(float n){return fract(sin(n) * 43758.5453123);}


float noise(float p){
    float fl = floor(p);
    float fc = fract(p);
    return mix(rand(fl), rand(fl + 1.0), fc);
}

void main(void){
//    Color = vec4(normalize(Random), 1);
    Color = texture(Texture, In.Texcoord.xy + noise(Random));
}
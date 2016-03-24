#version 410 core

in block{
    vec2 Texcoord;
    vec2 Position;
} In;

uniform vec3 Random;
uniform sampler2D Texture;
uniform sampler2D GlitchTexture;
uniform vec2 ScreenDim;

layout(location = 0, index = 0) out vec4  Color;

float EPSILON = 0.01;

float rand(float n){
    return fract(sin(n) * 43758.5453123);
}

float noise(float p){
    float fl = floor(p);
    float fc = fract(p);
    return mix(rand(fl), rand(fl + 1.0), fc);
}

vec2 randomPixel(vec2 randomValue, float padding){
    vec2 pixel = vec2(rand(randomValue.x), rand(randomValue.y));
    pixel = pixel * 0.5 - 0.5;

    pixel.x /= ScreenDim.x;
    pixel.y /= ScreenDim.y;
    pixel *= padding;
    return pixel;
}

void main(void){
    vec3 colorGlitch = texture(Texture, In.Texcoord.xy - randomPixel(In.Texcoord.xy * Random.xy, 80)).xyz;
    if(colorGlitch.x < EPSILON && colorGlitch.y < EPSILON && colorGlitch.z < EPSILON){
//        Color = vec4(Random,1);
        Color = 0.5 * pow(texture(GlitchTexture, In.Texcoord.xy - randomPixel(In.Texcoord.y * Random.xy + Random.yx, 500)), vec4(8.0));
    }
    else{
       float seed = int(In.Texcoord.x * ScreenDim.x) * int(In.Texcoord.y * ScreenDim.y);
       Color = vec4(rand(Random.x),rand(Random.y),rand(Random.z),1);
//       Color = vec4(1,1,1,1);
    }
}
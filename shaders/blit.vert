#version 410 core
#define POSITION 0
#define TEXCOORD 1
layout(location = POSITION) in vec2 Position;
layout(location = TEXCOORD) in vec2 Texcoord;

out block
{
    vec2 Texcoord;
    vec2 Position;
} Out;

uniform mat3 Homography;

void main()
{   
    Out.Texcoord = Texcoord;
    Out.Position = Position;
    gl_Position = vec4(Position, 0.0, 1.0);
}
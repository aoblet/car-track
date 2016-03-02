#version 330 core

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexColor;
layout(location = 2) in vec2 VertexUVCoord;

out vec3 vFragColor;
out vec2 vUVCoord;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;

void main() {
    vFragColor = VertexColor;
    vUVCoord = VertexUVCoord;

    gl_Position = ProjectionMatrix * ViewMatrix * vec4(VertexPosition, 1);
}

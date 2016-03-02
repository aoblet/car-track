#version 330 core

in vec3 vFragColor;
in vec2 vUVCoord;

out vec4 fFragColor;

void main() {
    fFragColor = vec4(vFragColor, 1);
}

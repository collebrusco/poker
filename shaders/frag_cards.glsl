#version 410 core
out vec4 outColor;

in vec2 iUV;
in vec2 iPos;

uniform sampler2D uTexslot;

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}
void main(){
    vec4 color = texture(uTexslot, iUV);
    outColor = color;//vec4(1.f, 1.f, 1.f, 1.f);
}

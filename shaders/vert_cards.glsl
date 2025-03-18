#version 410 core
/*
 This is an example of a simple vertex shader
 The three options below are different levels of MVP matrix implementation
 The CPU calculates view and proj matricies from the position and type of the camera,
 and calculates the model matrix based on the transform (pos/rotation/scale) of the object.
 */
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

uniform ivec2 uSheetPos;

out vec2 iUV;
out vec2 iPos;
void main() {
    const vec2 card_size = vec2(1.f / 13.f, 1.f / 5.f);
    iUV = (aUV + vec2(float(uSheetPos.x), float(uSheetPos.y))) * card_size;
    iPos = aPos;
    gl_Position = uProj * uView * uModel * vec4(aPos, 0.f, 1.0f);
    // gl_Position = uView * uModel * vec4(aPos, 0.f, 1.0f);
}

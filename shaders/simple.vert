#version 330 core

uniform vec2 resolution;
uniform float time;

layout (location = 0) in vec2 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 uv;

out vec4 out_color;
out vec2 out_uv;

vec2 convert_screen_2_ndc(vec2 p) {
    float x = (2 * p.x / resolution.x) - 1;
    float y = (1 - (2 * p.y / resolution.y)) * -1;
    return vec2(x, y);
}

void main() {
    gl_Position = vec4(convert_screen_2_ndc(position), 0.0, 1.0);
    out_color = color;
    out_uv = uv;
}

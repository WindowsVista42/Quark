#version 450

layout (location = 0) in vec4 in_color;

layout (location = 0) out vec4 out_color;

vec3 aces(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
    out_color = vec4(aces(pow(in_color.rgb, vec3(2.2f))), in_color.a);
}

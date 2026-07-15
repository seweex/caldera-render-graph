#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

layout(push_constant) uniform MaterialPush
{
    mat4 view;
}
pcMaterial;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = vec4(pcMaterial.view * vec4(inPosition, 1.f));
    fragColor = vec3(inUV, 0.f);
}

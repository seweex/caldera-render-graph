#version 450

layout(push_constant) uniform Settings
{
    mat4 cameraMatrix;
    float deltaTime;
    float timeSpeed;
} frameSettings;

layout(location=0) in vec4 planetPositionAndMass;
layout(location=1) in vec4 planetColorAndRadius;
layout(location=2) in vec4 speed;

layout(location=0) out vec4 colorAndRadius;

void main()
{
    gl_Position = frameSettings.cameraMatrix * vec4(planetPositionAndMass.xyz, 1.f);
    float worldDistance = distance(planetPositionAndMass.xyz, vec3(-100.0f, 45.0f, -100.0f));

    if (worldDistance <= 0.001) {
        gl_PointSize = 1.f;
        return;
    }

    gl_PointSize = max(planetColorAndRadius.w / worldDistance, 1.f);
    colorAndRadius = vec4(planetColorAndRadius.xyz, gl_PointSize / 1.f);
}
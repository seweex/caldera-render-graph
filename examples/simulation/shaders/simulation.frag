#version 450

layout(location=0) in vec4 colorAndRadius;
layout(location=0) out vec4 resultColor;

void main()
{
    vec2 pointCoord = gl_PointCoord - vec2(0.5);

    if (dot(pointCoord, pointCoord) > 0.25)
        discard;

    resultColor = vec4(colorAndRadius.xyz, 1.f);
}

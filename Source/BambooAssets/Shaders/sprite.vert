#version 420 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 Color;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in float aTexIndex;

layout(std140 ,binding = 0) uniform Camera
{
    mat4 u_ViewProjection;
};

out vec4 ourColor;
out vec2 TexCoord;
layout(location = 4) out float TexIndex;


void main() 
{
    TexCoord = aTexCoord;
    TexIndex = aTexIndex;
    gl_Position = u_ViewProjection* vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
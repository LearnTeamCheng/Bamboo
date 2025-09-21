#version 420 core
uniform sampler2D textureSampler;

in vec2 TexCoord;
out vec4 FragColor;

void main()
{
    vec4 color = texture(textureSampler, TexCoord);
    FragColor = color;
}
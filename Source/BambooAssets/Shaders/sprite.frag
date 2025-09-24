#version 420 core
layout(binding = 0) uniform sampler2D textureSampler[2];

in vec2 TexCoord;
layout (location = 4)in flat float TexIndex;
out vec4 FragColor;

void main()
{
    vec4 color = vec4(1.0);
    switch(int(TexIndex)){
        case 0:
            color = texture(textureSampler[0], TexCoord);
            break;
        case 1:
            color = texture(textureSampler[1], TexCoord);
            break;
        default:
            break;
    }
    FragColor = color;
}
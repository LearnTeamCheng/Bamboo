#version 420 core
layout(binding = 0) uniform sampler2D textureSampler[32];

in vec2 TexCoord;
in vec4 ourColor;
layout (location = 4)in flat float TexIndex;
out vec4 FragColor;

void main()
{
    vec4 color = vec4(1.0);
    switch(int(TexIndex)){
        case 0:
            color = ourColor*texture(textureSampler[0], TexCoord);
            break;
        case 1:
            color = ourColor*texture(textureSampler[1], TexCoord);
            break;
        case 2:
            color = ourColor*texture(textureSampler[2], TexCoord);
            break;
        case 3:
            color = ourColor*texture(textureSampler[3], TexCoord);
            break;
        case 4:
            color = ourColor*texture(textureSampler[4], TexCoord);
            break;
        case 5:
            color = ourColor*texture(textureSampler[5], TexCoord);
            break;
        default:
            break;
    }
    FragColor = color;
}
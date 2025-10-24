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
        case 6:
            color = ourColor*texture(textureSampler[6], TexCoord);
            break;
        case 7:
            color = ourColor*texture(textureSampler[7], TexCoord);
            break;
        case 8:
            color = ourColor*texture(textureSampler[8], TexCoord);
            break;
        case 9:
            color = ourColor*texture(textureSampler[9], TexCoord);
            break;
        case 10:
            color = ourColor*texture(textureSampler[10], TexCoord);
            break;
        case 11:
            color = ourColor*texture(textureSampler[11], TexCoord);
            break;
        case 12:
            color = ourColor*texture(textureSampler[12], TexCoord);
            break;
        case 13:
            color = ourColor*texture(textureSampler[13], TexCoord);
            break;
        case 14:
            color = ourColor*texture(textureSampler[14], TexCoord);
            break;
        case 15:
            color = ourColor*texture(textureSampler[15], TexCoord);
            break;
        case 16:
            color = ourColor*texture(textureSampler[16], TexCoord);
            break;
        case 17:
            color = ourColor*texture(textureSampler[17], TexCoord);
            break;
        case 18:
            color = ourColor*texture(textureSampler[18], TexCoord);
            break;
        case 19:
            color = ourColor*texture(textureSampler[19], TexCoord);
            break;
        case 20:
            color = ourColor*texture(textureSampler[20], TexCoord);
            break;
        case 21:
            color = ourColor*texture(textureSampler[21], TexCoord);
            break;
        case 22:
            color = ourColor*texture(textureSampler[22], TexCoord);
            break;
        case 23:
            color = ourColor*texture(textureSampler[23], TexCoord);
            break;
        case 24:
            color = ourColor*texture(textureSampler[24], TexCoord);
            break;
        case 25:
            color = ourColor*texture(textureSampler[25], TexCoord);
            break;
        case 26:
            color = ourColor*texture(textureSampler[26], TexCoord);
            break;
        case 27:
            color = ourColor*texture(textureSampler[27], TexCoord);
            break;
        case 28:
            color = ourColor*texture(textureSampler[28], TexCoord);
            break;
        case 29:
            color = ourColor*texture(textureSampler[29], TexCoord);
            break;
        case 30:
            color = ourColor*texture(textureSampler[30], TexCoord);
            break;
        case 31:
            color = ourColor*texture(textureSampler[31], TexCoord);
        default:
            break;
    }
    FragColor = color;
}
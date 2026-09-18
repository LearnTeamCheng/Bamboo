#version 420 core

out vec4 FragColor;

in vec3 ourColor;
in vec3 ourPosition;
in float ourRadius;

int main() {

    float l = length(ourPosition);
    if(l >ourRadius){
        discard
    }

    FragColor = vec4(ourColor,1.0);    
}  
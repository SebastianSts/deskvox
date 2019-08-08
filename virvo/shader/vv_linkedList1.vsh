#version 430

layout (location = 0) in vec3 VerPos;
layout (location = 1) in float VerClr;


out vec3 Color;
out float Volumen;

uniform mat4 MVP;

void main()
{
    Color = VerPos;
    Volumen = VerClr;
    gl_Position = MVP *vec4(VerPos,1.0); 
    
}

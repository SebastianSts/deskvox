#version 430

layout (location = 0) in vec3 VerPos;
//layout (location = 1) in vec3 VerClr;

//out vec3 EntryPoint;
//out vec4 ExitPointCoord;



uniform mat4 invMVP;
//uniform mat4 MVP;

void main()
{
   // EntryPoint = VerPos;
    gl_Position =   vec4(VerPos,1.0);  // MVP *
   // ExitPointCoord = gl_Position;
    
	
  
}

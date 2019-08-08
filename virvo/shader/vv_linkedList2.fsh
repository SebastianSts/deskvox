#version 430

#define FRAGMENT_LIMIT 30
#define eps 0.00001f
out vec4 FragColor;
//in vec3 EntryPoint;
//in vec4 ExitPointCoord;
// Uniforms are variables that are passed in and modified from the host
uniform sampler3D VolumeTex;	// Volumetric texture to be rendered
uniform sampler1D TransferFunc;	// Transfer function
uniform float     StepSize;	// Step size of the samples
uniform float 	  ScreenSizeX;	// X
uniform float     ScreenSizeY;	// Y
uniform mat4 	  ScaleMatrix;
//uniform uint 	  MaxNodes;	//
uniform mat4      invMVP;	// Inverse MVP
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Linked List
struct NodeType {
  // vec4 color;
  float depth;
  uint next;
 };

layout( binding = 0, r32ui) uniform uimage2D headPointers;
layout( binding = 0, std430 ) buffer linkedLists {
  NodeType nodes[];
};

uint ll_prev(uint start, uint current)
	{ 
		uint it = start;
		while(nodes[it].next != current )
			{
			it = nodes[it].next;
			}			
		return it;
	}
	
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void main() {
  vec4 color =vec4(1.0,1.0,0.9,1.0);
  uint  count = 0;
  uint prev = 0;

  NodeType frags[FRAGMENT_LIMIT];
  
  
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Index of the head of the list
  uint n = imageLoad(headPointers, ivec2(gl_FragCoord.xy)).r;
  	if(n == 0xffffffff)
  		discard;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//insert sort 
	uint it = nodes[n].next;
	while (it != 0xffffffff )
	{
		NodeType toInsert = nodes[it];
		uint jt = it;
		while (jt != n && toInsert.depth < nodes[ll_prev(n,jt)].depth)
		{
			//nodes[jt].color = nodes[ll_prev(n,jt)].color;
			nodes[jt].depth = nodes[ll_prev(n,jt)].depth;
			jt = ll_prev(n,jt);
			
		}
		//nodes[jt].color = toInsert.color;
		nodes[jt].depth = toInsert.depth;
		it = nodes[it].next;
		
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//draw cube with linked list
	/*while(n != 0xffffffff)
		{
		//color = mix( color, nodes[n].color, nodes[n].color.a);
		count+=1;
		n = nodes[n].next;
		}	
		//FragColor = color;*/
		/*uint entry;
		uint exit;
		uint exitNode;
		while(n != 0xffffffff && nodes[n].next != 0xffffffff)
			{
			entry = n;
			exit = n;
			while(abs(nodes[nodes[n].next].depth - nodes[nodes[nodes[n].next].next].depth)<eps && nodes[nodes[nodes[n].next].next].next != 0xffffffff)
			{
				n = nodes[nodes[n].next].next;
				exit = n;
			}
			exitNode = nodes[exit].next;
			n = nodes[exitNode].next;
			count++;
		}*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//transfer function	
vec4 colorAccum = vec4(0.0);				// Accumulated color of the ray
float x = 2.0* (gl_FragCoord.x / ScreenSizeX) -1.0;
float y = 2.0* (gl_FragCoord.y / ScreenSizeY) -1.0 ;
uint entry;
uint exit;
while(n != 0xffffffff && nodes[n].next != 0xffffffff)
	{
	entry = n;
	exit = n;
	while(abs(nodes[nodes[n].next].depth - nodes[nodes[nodes[n].next].next].depth)<eps && nodes[nodes[nodes[n].next].next].next != 0xffffffff)
		{
			n = nodes[nodes[n].next].next;
			exit = n;
		}
	
	float z = 2.0* nodes[entry].depth -1.0 ;
	//EntryPoint in world coord
	vec4 EntryPoint = invMVP*vec4(x,y,z,1.0);
	EntryPoint.w = 1.0 / EntryPoint.w;
	EntryPoint.xyz *=EntryPoint.w;
	
	//exitNode in world coord
	uint exitNode = nodes[exit].next;
	 z = 2.0 * nodes[exitNode].depth -1.0;
	vec4 exitPoint = invMVP*vec4(x,y,z,1.0);
	exitPoint.w = 1.0 / exitPoint.w;
	exitPoint.xyz *=exitPoint.w;
 


    	//Ray setup
    	vec3 dir = exitPoint.xyz - EntryPoint.xyz; 		// compute the direction vector
    	float len = length(dir);            		// length from front to back 
    	vec3 deltaDir = normalize(dir) * StepSize;	// distance and direction 	between samples
   	vec3 voxelCoord = EntryPoint.xyz;       		// start the loop at the entry point    			
	int numSteps = int(ceil(len / StepSize));  	// the number of samples that will be taken along the ray
   	
    	//loop along the ray
    	for(int i = 0; i < numSteps; i++)
   	{	
    		float intensity =  texture(VolumeTex, voxelCoord).x; // sample scalar intensity value from the volume
		vec4 colorSample = texture(TransferFunc, intensity);	// evaluate transfer function by sampling the texture    	
		if (colorSample.a > 0.0)
         	   {
    	 	   colorSample.a = 1.0 - pow(1.0 - colorSample.a, StepSize*200.0f);
    	 	   colorAccum.rgb += (1.0 - colorAccum.a) * colorSample.rgb * colorSample.a;
    	 	   colorAccum.a += (1.0 - colorAccum.a) * colorSample.a;
    	 	   }

    		voxelCoord += deltaDir;

		if (colorAccum.a > .97) 
		{
         	 	colorAccum.a = 1.0;
          	  	break;
	        }
	} 
	n = nodes[exitNode].next;
	//count++;
}	

    FragColor = mix(color, colorAccum, colorAccum.a); // blend the background color using an "under" blend operation*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//show number of fragments per pixel

// vec4 color = vec4 (1.0,1.0,1.0,0.0);
/* if(count == 0)
	color = vec4(1.0, 1.0, 1.0, 1.0);
 if(count == 1)
	color = vec4(0.0, 0.0, 0.0, 1.0);
 if(count == 2)
	color = vec4(1.0, 0.0, 0.0, 1.0);
 if(count == 3)
	color = vec4(0.0, 0.0, 1.0, 1.0);
 if(count == 4)
	color = vec4(0.0, 1.0, 0.0, 1.0);
 if(count == 5)
	color = vec4(0.5, 0.5, 0.5, 1.0);
 if(count == 6)
	color = vec4(0.0, 0.0, 1.0, 1.0);
 if(count == 7)
	color = vec4(1.0, 1.0, 1.0, 1.0);
 if(count == 8)
	color = vec4(0.0, 0.0, 0.0, 1.0);
	
	FragColor = color;*/
}

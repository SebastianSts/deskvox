#version 430

#define SORT_FRAGMENTS 1
#define COUNT_INTEGRATION_STEPS 0
#define MERGE_FRAGMENTS 0
//#define theta 0.00002f 
#define theta 0.002f 
		     
out vec4 FragColor;

// Uniforms are variables that are passed in and modified from the host
uniform sampler3D VolumeTex;	// Volumetric texture to be rendered
uniform sampler1D TransferFunc;	// Transfer function
uniform float     StepSize;	// smallest quotient of axis (length/#voxel along the axis)
uniform float 	  ScreenSizeX;	// X
uniform float     ScreenSizeY;	// Y
uniform mat4      invMVP;	// transformation matrix (back to object sapce)
uniform float	  ScaleX;	// length x axis of the volume
uniform float	  ScaleY;	// length y axis of the volume
uniform float	  ScaleZ;	// length z axis of the volume	

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Linked List
struct NodeType {
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


uint currCount=0;
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void main() {
//Index of the head of the list
  uint index = imageLoad(headPointers, ivec2(gl_FragCoord.xy)).r;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//insert sort 
#if SORT_FRAGMENTS
    {
	uint it = nodes[index].next;
	while (it != 0xffffffff )
	{
		NodeType toInsert = nodes[it];
		uint jt = it;
		while (jt != index && toInsert.depth < nodes[ll_prev(index,jt)].depth)
			{			
			nodes[jt].depth = nodes[ll_prev(index,jt)].depth;
			jt = ll_prev(index,jt);			
		}		
		nodes[jt].depth = toInsert.depth;
		it = nodes[it].next;		
	}
     }
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//transfer function	
vec4 colorAccum = vec4(0.0);			// Accumulated color of the ray
uint entry;
uint exit;

float x = 2.0* ((gl_FragCoord.x) / ScreenSizeX) -1.0;
float y = 2.0* ((gl_FragCoord.y) / ScreenSizeY) -1.0 ;

while(index != 0xffffffff && nodes[index].next != 0xffffffff)
	{
	entry = index;
	exit = nodes[index].next;
	
#if MERGE_FRAGMENTS
	while(abs(nodes[exit].depth - nodes[nodes[exit].next].depth)<theta)
		{
		exit = nodes[nodes[exit].next].next;		
		}
#endif
	
	float z = 2.0* nodes[entry].depth -1.0;	
	//entryPoint in obect sapce
	vec4 entryPoint = invMVP*vec4(x,y,z,1.0);	
	entryPoint.xyz /=entryPoint.w;
		
	//exitNode in object space
	uint exitNode = exit;
	z = 2.0 * nodes[exitNode].depth -1.0;
	vec4 exitPoint = invMVP*vec4(x,y,z,1.0);	
	exitPoint.xyz /=exitPoint.w;	
		
	
    	//Ray setup
    	vec3 dir = (exitPoint.xyz - entryPoint.xyz);					// ray direction vector	
    	float len = length(dir);         						// length from front to back 
    	vec3 deltaDir = (normalize(dir) * StepSize) /vec3( ScaleX,ScaleY,ScaleZ) ;	// deltaDir (scaled to texture space)
   	   
   	vec3 texCoord = entryPoint.xyz/vec3( ScaleX,ScaleY,ScaleZ);      		// entryPoint.xyz is in object space, translation to texture space
   	float t = 0.0;   	
    	//loop along the ray   	
    	while(t<len)
   	{	
   		vec3 rotateTexCoord = vec3(texCoord.x,1.0f-texCoord.y,1.0-texCoord.z);
    		float intensity =  texture(VolumeTex, rotateTexCoord).x; 					// sample scalar intensity value from the volume
		vec4 colorSample = texture(TransferFunc, intensity);						// evaluate transfer function by sampling the texture    	
		//if (colorSample.w > 0.0)
         	  // {	 	 			  
    	 		// premultiplied alpha
    	 		colorSample.xyz *= colorSample.w; 
    	 		// compositing
    	 		colorAccum += colorSample * (1.0f - colorAccum.w);
    	 	
    	 	 //  }
    		texCoord += deltaDir;	
    		t+= StepSize;   		
    		 
#if COUNT_INTEGRATION_STEPS 	   
    		 currCount++;
#endif	

	} 
	index = nodes[exitNode].next;	
}

#if COUNT_INTEGRATION_STEPS
    	uint last = imageAtomicExchange(headPointers, ivec2(gl_FragCoord.xy), currCount);    	
#endif	 
   
    	FragColor = colorAccum;    
}
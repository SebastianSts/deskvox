#version 430

#define eps 0.00001f
out vec4 FragColor;

// Uniforms are variables that are passed in and modified from the host
uniform sampler3D VolumeTex;	// Volumetric texture to be rendered
uniform sampler1D TransferFunc;	// Transfer function
uniform float     StepSize;	// Step size of the samples
uniform float 	  ScreenSizeX;	// X
uniform float     ScreenSizeY;	// Y
uniform mat4      texMat;	// Matrix (back to texture coords)
uniform float	  ScaleX;	
uniform float	  ScaleY;
uniform float	  ScaleZ;

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
layout( binding = 0, rgba32ui) uniform writeonly uimage2D Counter;

uint ll_prev(uint start, uint current)
	{ 
		uint it = start;
		while(nodes[it].next != current )
			{
			it = nodes[it].next;
			}			
		return it;
	}

//bool sort =true;
bool sort = false;
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void main() {
float count =0.0;
//Index of the head of the list
  uint n = imageLoad(headPointers, ivec2(gl_FragCoord.xy)).r;
  	//if(n == 0xffffffff)
  		//discard;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//insert sort 
    if(sort ==true)
    {
	uint it = nodes[n].next;
	while (it != 0xffffffff )
	{
		NodeType toInsert = nodes[it];
		uint jt = it;
		while (jt != n && toInsert.depth < nodes[ll_prev(n,jt)].depth)
			{			
			nodes[jt].depth = nodes[ll_prev(n,jt)].depth;
			jt = ll_prev(n,jt);			
		}		
		nodes[jt].depth = toInsert.depth;
		it = nodes[it].next;		
	}
     }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//transfer function	
vec4 colorAccum = vec4(0.0);			// Accumulated color of the ray
uint entry;
uint exit;
//float llen=0.0;
while(n != 0xffffffff && nodes[n].next != 0xffffffff)
	{
	entry = n;
	exit = n;
	/*while(abs(nodes[nodes[n].next].depth - nodes[nodes[nodes[n].next].next].depth)<eps && nodes[nodes[nodes[n].next].next].next != 0xffffffff)
		{
		n = nodes[nodes[n].next].next;
		exit = n;
		}*/
	float x = 2.0* ((gl_FragCoord.x) / ScreenSizeX) -1.0;
	float y = 2.0* ((gl_FragCoord.y) / ScreenSizeY) -1.0 ;
	float z = 2.0* nodes[entry].depth -1.0 ;	
	
	//EntryPoint in texCoord
	vec4 entryPoint = texMat*vec4(x,y,z,1.0);
	entryPoint.w = 1.0 / entryPoint.w;
	entryPoint.xyz *=entryPoint.w;
		
	//exitNode in texCoord
	uint exitNode = nodes[exit].next;
	 z = 2.0 * nodes[exitNode].depth -1.0;
	vec4 exitPoint = texMat*vec4(x,y,z,1.0);
	exitPoint.w = 1.0 / exitPoint.w;
	exitPoint.xyz *=exitPoint.w;
	
	/*vec3 bboxmin = vec3(-128.0,-128.0,-112.5);
	vec3 bboxmax = vec3(128.0,128.0,112.5);
	vec3 bboxsize = vec3(256.0,256.0,225.0);
	
	vec3 entryPointBla = entryPoint.xyz * bboxsize + bboxmin;
	vec3 exitPointBla = exitPoint.xyz * bboxsize + bboxmin;
	llen=length(entryPointBla-exitPointBla)/256.0;*/	
	
    	//Ray setup
    	vec3 dir = ((exitPoint.xyz*vec3(ScaleX,ScaleY,ScaleZ)) - (entryPoint.xyz*vec3(ScaleX,ScaleY,ScaleZ)));  // Ray direction (scaled)
    	float len = length(dir);            									// length from front to back 
    	vec3 deltaDir = normalize(dir)/vec3(ScaleX,ScaleY,ScaleZ) * StepSize ;					// distance and direction 	between samples
   	vec3 texCoord = entryPoint.xyz;      									// start the loop at the entry point
   	float t = 0.0;   	
    	//loop along the ray   	
    	while(t<len)
   	{	
   		vec3 rotateTexCoord = vec3(texCoord.x,1.0f-texCoord.y,1.0-texCoord.z);
    		float intensity =  texture(VolumeTex, rotateTexCoord).x; 					// sample scalar intensity value from the volume
		vec4 colorSample = texture(TransferFunc, intensity);						// evaluate transfer function by sampling the texture    	
		//if (colorSample.w > 0.0)
         	  // {
         		// opacity correction
    	 		//colorSample.w = 1.0f - pow(1.0f - colorSample.w, StepSize*(1/StepSize));    	 	 			  
    	 		// premultiplied alpha
    	 		colorSample.xyz *= colorSample.w; 
    	 		// compositing
    	 		colorAccum += colorSample * (1.0f - colorAccum.w);
    	 		 // colorAccum += vec4(1.0/512.0);
    	 	 //  }
    		texCoord += deltaDir;	
    		t+= StepSize;     		 
	} 
	n = nodes[exitNode].next;	
}
    	imageStore(Counter, ivec2(gl_FragCoord.xy), uvec4(23));
    //FragColor = mix(color, colorAccum, colorAccum.w); // blend the background color using an "under" blend operation*/
    	FragColor = colorAccum;
    //FragColor=vec4(StepSize*128.0,0.0,0.0,count/255.0 );
}
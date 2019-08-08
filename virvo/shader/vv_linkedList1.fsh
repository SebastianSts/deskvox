#version 430

in vec3 Color;
in float Volumen;
//out vec4 FragColor;

uniform uint MaxNodes;

struct NodeType {
 // vec4 color;
  float depth;
  uint next;
};

layout( binding = 0, r32ui) uniform uimage2D headPointers;
layout( binding = 0, offset = 0) uniform atomic_uint nextNodeCounter;
layout( binding = 0, std430 ) buffer linkedLists {
  NodeType nodes[];
};


void main() {
    //discard all fragments with "Volumen <1"
   if(Volumen < 0.99)
    	discard;
  //atomic counter - every fragment gets an unique nodeIdx  	
  uint nodeIdx = atomicCounterIncrement(nextNodeCounter);

  //if( nodeIdx < MaxNodes ) 
	//{
	//get prev head of the list and write new head in headPointers
    	uint prevHead = imageAtomicExchange(headPointers, ivec2(gl_FragCoord.xy), nodeIdx);  
     	//nodes[nodeIdx].color = vec4(Color,0.5);
    	nodes[nodeIdx].depth = gl_FragCoord.z;   
    	nodes[nodeIdx].next = prevHead;	
    	//FragColor = vec4(1.0,1.0,0.0,1.0);// only for test
  //	}
}

#version 430
#extension GL_ARB_fragment_shader_interlock : enable

struct NodeType {
  float depth;
  uint next;
};

layout( pixel_interlock_ordered ) in;
layout( binding = 0, r32ui) uniform uimage2D headPointers;
layout( binding = 0, offset = 0) uniform atomic_uint nextNodeCounter;
layout( binding = 0, std430 ) buffer linkedLists {
  NodeType nodes[];
};


void main() {  

     	beginInvocationInterlockARB();
 		//atomic counter - every fragment gets an unique nodeIdx 	
 		uint nodeIdx = atomicCounterIncrement(nextNodeCounter);
  		uint prevHead = imageAtomicExchange(headPointers, ivec2(gl_FragCoord.xy), nodeIdx);      	
  		nodes[nodeIdx].depth = gl_FragCoord.z;   
  		nodes[nodeIdx].next = prevHead; 
    	endInvocationInterlockARB();
    	
}
// Author: Emil Hedemalm
// Date: 2016-06-20
// Name: Deferred lighting n shadow mapping
#version 330

// Output from the deferred. DiffuseOut and DepthOut? Diffuse is in reality the final colors.
layout (location = 0) out vec4 diffuseOut;	// Diffuse, normal, position, in that 

// Uniforms
// 2D Texture texture. Set these to -1 if you are not going to use them!
uniform sampler2D diffuseMap; // Same namespace as the regular diffuse/specular/normal maps in 

// Input data from the fragment shader
varying vec2 UV_Coord;		// Just passed on
varying vec3 constantColor;

void main()
{
	// Texture image data. This will be the base for the colors.
	vec3 color = texture2D(diffuseMap, UV_Coord).xyz;	
	diffuseOut.xyz = color;
	diffuseOut.w = 1;
//	gl_FragColor.xyz = color;
//	gl_FragColor.w = distance;
//	gl_FragDepth = texture2D(depthMap, UV_Coord).x;
//	gl_Position.w = distance;
	return;
}

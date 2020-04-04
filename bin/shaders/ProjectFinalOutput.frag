/// Author: Emil Hedemalm
/// Date: 2016-08-08
/// Made to project final results quad onto the screen or viewport
#version 120

// Uniforms
// 2D Texture texture
uniform sampler2D baseImage;
uniform sampler2D emissiveMap;

// Color applied to all stuff in the final.
uniform vec4 primaryColorVec4 = vec4(1.0, 1.0, 1.0, 1.0);

uniform vec4 global_ambient = vec4(1,1,1,1);

uniform float emissiveMapFactor = 1;

// Input data from the fragment shader
varying vec3 normal;		// Interpolated coordinates that have been transformed to view space
varying vec2 UV_Coord;	// Just passed on
varying vec3 worldCoord;	// World coordinates of the fragment
varying vec3 vecToEye;	// Vector from vertex to eye
varying vec3 position;

void main(){
	// Texture image data. This will be the base for the colors.
	vec4 baseFrag = texture2D(baseImage, UV_Coord);
	vec4 emissiveFrag = texture2D(emissiveMap, UV_Coord);
	gl_FragColor = clamp(baseFrag, 0, 1);
//	gl_FragColor *= primaryColorVec4;
//	gl_FragColor *= global_ambient;	
//	gl_FragColor.x = 0.9;
	// Add emissive ness
	vec4 emissiveFactor = clamp(emissiveFrag * emissiveMapFactor, 0, 1);
//	gl_FragColor += emissiveFactor.xyz * emissiveFactor.w;
}

// Author: Emil Hedemalm
// Date: 2014-09-17
#version 120

// Uniforms
// 2D Texture texture
uniform sampler2D baseImage;

// Color applied to all stuff in the final.
uniform vec4 primaryColorVec4 = vec4(1.0, 1.0, 1.0, 1.0);

uniform vec4 global_ambient = vec4(1,1,1,1);

// Input data from the fragment shader
varying vec3 normal;		// Interpolated coordinates that have been transformed to view space
varying vec2 UV_Coord;	// Just passed on
varying vec3 worldCoord;	// World coordinates of the fragment
varying vec3 vecToEye;	// Vector from vertex to eye
varying vec4 position;

// Particle color
varying vec4 particleColorMultiplier;

void main(){
	// Texture image data. This will be the base for the colors.
	vec4 baseFrag = texture2D(baseImage, UV_Coord);
	baseFrag *= particleColorMultiplier;
	gl_FragColor = baseFrag;
}

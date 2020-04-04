// Author: Emil Hedemalm
// Date: 2015-02-23
// Shader for TIFS game-specific Tool-particles
#version 120

// Uniforms
// 2D Texture texture
uniform sampler2D baseImage;

// Color applied to all stuff in the final.
uniform vec4 primaryColorVec4 = vec4(1.0, 1.0, 1.0, 1.0);

// Light data
uniform vec4 global_ambient = vec4(1,1,1,1);

// Input data from the fragment shader
varying vec3 v_w_normal;		// Interpolated coordinates that have been transformed to view space
varying vec2 v_UV_Coord;	// Just passed on
varying vec3 worldCoord;	// World coordinates of the fragment
varying vec3 vecToEye;	// Vector from vertex to eye
varying vec3 v_w_position;

// Particle color
varying vec4 color;
varying vec2 lifeTimeDuration;
varying vec3 debugColor;

// Particle lifetime stats
varying	float lifeTime;
varying	float halfLife;
varying float duration;
varying float relativeTimeLived;
varying float secondsLeft;


/// 0 - No, 1 - linearly, 2 - quadratically, 3 - cubically
uniform int particleDecayAlphaWithLifeTime;
uniform float fadeOutEnd = 1;

void main()
{
	vec4 f_debugColor = vec4(debugColor,1);
	// Texture image data. This will be the base for the colors.
	vec4 baseFrag = texture2D(baseImage, v_UV_Coord);

	baseFrag *= color;

	// Lighting?
	
	baseFrag = color;
//	baseFrag.w = 1;
	if (secondsLeft < 1)
	{
		baseFrag.w *= pow(secondsLeft,2);
	}
	if (baseFrag.w < 0.1)
		baseFrag.w = 0.1;
	baseFrag.xyz *= baseFrag.w;
//	f_debugColor.x = baseFrag.w;
	if (baseFrag.w < 0.05)
		f_debugColor.y = 1;
	
	gl_FragColor = baseFrag;
	if (f_debugColor.x > 0 || f_debugColor.y > 0 || f_debugColor.z > 0)
		gl_FragColor = f_debugColor;
}

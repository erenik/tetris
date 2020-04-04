// Author: Emil Hedemalm
// Date: 2012-10-29
// Name: Simple UI Shader
#version 120

// Uniforms
// 2D Texture texture
uniform sampler2D baseImage;

// Yush.
uniform vec4 primaryColorVec4 = vec4(1,1,1,1);
uniform vec4 secondaryColorVec4 = vec4(.5,.5,.5,1);
/// Highlight that is added linearly upon the final product.
uniform vec4 highlightColorVec4 = vec4(0,0,0,0);

// Input data from the fragment shader
varying vec2 UV_Coord;	// Just passed on
varying vec3 worldCoord;	// World coordinates of the fragment
varying vec3 vecToEye;	// Vector from vertex to eye
varying vec3 position;

varying vec3 debugColor;

/// 0 for No, 1 for Yes
uniform int hoveredOver;

/** Default 0 - pass through.
	1 - Simple White font - apply primaryColorVec4 multiplicatively
	2 - Replacer. Replaces a set amount of colors in the font for other designated colors (primarily primaryColorVec4 and se)
*/
uniform int colorEquation = 2;

void main(void) 
{
	// Texture image data. This will be the base for the colors.
	vec4 baseFrag = texture2D(baseImage, UV_Coord);
	vec4 color = baseFrag;

	/// Convert color according to designated equation and source font properties.
	if (colorEquation == 2)
	{
		// Replace! :D
		// Check distance to closest applicable thingy.
		vec3 main = vec3(0.752, 0.745, 0.6627);
		vec3 second = vec3(0.502, 0.474, 0.365);
		vec3 color3 = color.xyz;
		float distToMain =  1 - color3.x;
		float distToSecond = color3.x;
	//	color3.xyz *= 0;
	//	color3.x = distToMain;
//		color3.y = distToSecond;
		
		/// Add 'em based on distance!
		vec3 newColor = vec3(0,0,0);
		
		vec3 newMain = primaryColorVec4.xyz;
//		newMain = vec3(1,0,0);
		vec3 newSecond; 
		newSecond = newMain * 0.6;
		// = secondaryColorVec4.xyz;
		// newSecond = vec3(0,1,0);
		if (hoveredOver == 1)
		{
			newMain.xyz *= 1.1;
			newMain.xyz += vec3(1,1,1) * 0.1;
			newSecond.xyz = newMain * 0.8;
		}
		// Blend to orange. o.o'
		float orangeFactor = 0.5;
		newSecond.xyz = newSecond.xyz * (1 - orangeFactor) + vec3(1,0.5,0) * orangeFactor;
		
		float mainRatio = (1 - distToMain);
		float ratioRemaining = 1 - mainRatio;
		
		float secondRatio = ratioRemaining; // 1 - ((1 - distToSecond));
		
		newColor += newMain * mainRatio;
		newColor += newSecond * secondRatio;
		
		if (distToMain < distToSecond)
		{
	//		color3 = vec3(1,0,0);
		}
		else
		{
	//		color3 = vec3(0,1,0);
		}
	//	color3 = main;
		
//		color3 = second;
//		color3.xy = vec2(0,0);
		color.xyz = newColor;
	//	color.xyz = color3;
	}
	
	gl_FragColor = color;
//	gl_FragColor *= primaryColorVec4;
//	gl_FragColor += highlightColorVec4;	
//	gl_FragColor.xw = vec2(.5,0);
	
	if (debugColor.x > 0)
		gl_FragColor = vec4(debugColor.xyz, 1);
}



// Author: Emil Hedemalm
// Date: 2012-10-29
// Name: Simple UI Shader
#version 120

// Uniforms
// 2D Texture texture
uniform sampler2D baseImage;

// Light statistics
uniform bool	lightActive				= true;
uniform vec4	lightAmbient			= vec4(0.0, 0.0, 0.0, 1.0);
uniform vec4	lightDiffuse			= vec4(1.0, 1.0, 1.0, 1.0);
uniform vec4	lightSpecular			= vec4(1.0, 1.0, 1.0, 1.0);
uniform vec4	lightDirOrPos			= vec4(1.0, 2.0, -1.0, 1.0);
uniform int		lightType				= 0;
uniform vec3	lightAttenuation		= vec3(1.0, 0.0, 0.0);

// Material statistics
uniform vec4	materialAmbient		= vec4(0.2, 0.2, 0.2, 1.0);
uniform vec4	materialDiffuse		= vec4(0.8, 0.8, 0.8, 1.0);
uniform vec4	materialSpecular	= vec4(1.1, 1.1, 1.1, 1.0);
uniform int		materialShininess	= 8;

// Input data from the fragment shader
in vec3 normal;		// Interpollated coordinates that have been transformed to view space
in vec2 UV_Coord;	// Just passed on
in vec3 worldCoord;	// World coordinates of the fragment
in vec3 vecToEye;	// Vector from vertex to eye
in vec3 position;

void main(void) {
	// Texture image data. This will be the base for the colors.
	vec4 baseFrag = texture2D(baseImage, UV_Coord);
	vec4 otherFrag = baseFrag;
	
	for(float i = 0.1; i < 1.2; i+=0.1){
		if(otherFrag.x < i){
			if(mod(i, 0.2) == 0){
				otherFrag.x = baseFrag.y;
			}
			else{
				otherFrag.x = baseFrag.z;
			}
			break;
		}
	}
	for(float i = 0.1; i < 1.2; i+=0.1){
		if(otherFrag.y < i){
			if(mod(i, 0.2) == 0){
				otherFrag.y = baseFrag.z;
			}
			else{
				otherFrag.y = baseFrag.x;
			}
			break;
		}
	}
	for(float i = 0.1; i < 1.2; i+=0.1){
		if(otherFrag.z < i){
			if(mod(i, 0.2) == 0){
				otherFrag.z = baseFrag.x;
			}
			else{
				otherFrag.z = baseFrag.y;
			}
			break;
		}
		
	}
	
	if(otherFrag.x > 0.1)
		otherFrag -= vec4(0.2, 0.2, 0.2, 0.0);
	else
		otherFrag += vec4(0.0, 0.0, 0.1, 0.0);
	
	//gl_FragColor = otherFrag;
	gl_FragColor = baseFrag;
	//gl_FragColor += vec4(position.x / 2000, position.y / 1200, position.z / 4, 0)*1.0;
//	gl_FragColor = vec4(0,0,0,1);
	return;
}

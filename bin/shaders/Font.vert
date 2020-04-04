// Author: Emil Hedemalm
// Date: 2015-05-04
// Name: Text font shader
#version 120

// Uniforms
// Projection matrix provided by the client.
uniform mat4 projectionMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
uniform mat4 viewMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
uniform mat4 modelMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);

// Input data for the shader
// in_Position was bound to attribute index 0, UV to index 1 and Normals to index 2.
attribute vec3 in_Position;
attribute vec2 in_UV;

// Output data for the fragment shader
varying vec2 UV_Coord;		// Just passed on
varying vec3 position;

/// Which character in the font we are to render.
uniform int character = 65;
uniform vec2 pivot = vec2(0,0);

varying vec3 debugColor;

void main(void) 
{
	debugColor = vec3(0,0,0);
	// Calculate matrices
	mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;
	
	// Multiple position with text scale.
	vec3 pos = in_Position;
	pos.xy += pivot;
	
	// Multiply mvp matrix onto the vertex coordinates.
	gl_Position = mvp * vec4(pos, 1);
	position = pos;

	float characterX = mod(character, 16);
	float characterY = character / 16;
	float x1 = (characterX / 16);
	float x2 = (characterX + 1) / 16.;
	float y2 = (16 - characterY - 1) / 16.0f;
		
	UV_Coord = in_UV;
	UV_Coord.x = x1 + in_UV.x / 16;
	UV_Coord.y = y2 + in_UV.y / 16;
}
 
 

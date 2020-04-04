// Author: Emil Hedemalm
// Date: 2016-06-20
// Name: Deferred lighting n shadow mapping
#version 330

// Output from the deferred. DiffuseOut and DepthOut? Diffuse is in reality the final colors.
layout (location = 0) out vec4 diffuseOut;	// Diffuse, normal, position, in that 

// Uniforms
// 2D Texture texture. Set these to -1 if you are not going to use them!
uniform sampler2D diffuseMap; // Same namespace as the regular diffuse/specular/normal maps in 
uniform sampler2D specularMap; // Simple/non-deferred or in deferred-gatherer render-passes.
uniform sampler2D normalMap;
uniform sampler2D positionMap; // For deferred.
uniform sampler2D emissiveMap;
uniform sampler2D depthMap; // For writing the correct depth from earlier.

// Lights
#define MAX_LIGHTS	150
uniform vec4 light_ambient = vec4(1,1,1,1);
uniform vec4 light_diffuse[MAX_LIGHTS];
uniform vec4 light_specular[MAX_LIGHTS];
uniform vec3 light_position[MAX_LIGHTS];	
uniform vec3 light_attenuation[MAX_LIGHTS];	// Constant, linear and quadratic
uniform int light_castsShadows[MAX_LIGHTS]; // -1 if not, positive if a specific shadow map? :D

// Position or direction, depending on w-parameter 0 = direction, 1 = positional
uniform int light_type[MAX_LIGHTS];	

// Spotlight data
uniform vec3 light_spotDirection[MAX_LIGHTS];
uniform float light_spotCutoffMin[MAX_LIGHTS]; // Add support for this!
uniform float light_spotCutoff[MAX_LIGHTS];
uniform int light_spotExponent[MAX_LIGHTS];
// Total amount of active lights
uniform int activeLights = 0;	// Lights active, from index 0 to activeLights-1

/// Variables needing replacement/new buffers/textures/uniforms.
/*
	material-properties.
	emissiveMap
*/
uniform vec4	materialAmbient		= vec4(0.2, 0.2, 0.2, 1.0);
uniform vec4	materialDiffuse		= vec4(0.8, 0.8, 0.8, 1.0);
uniform vec4	materialSpecular	= vec4(1.1, 1.1, 1.1, 1.0);
uniform int		materialShininess	= 8	;

/// Shadow-mapping o.o
uniform sampler2D shadowMap;
uniform mat4 shadowMapMatrix;

// Matrix to be multiplied for the normals (for rotations primarily)
uniform mat4 normalMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
// Position of eye in the world.
uniform vec4 eyePosition	= vec4(0.0, 5.0, 0.0, 0.0);

// Input data from the fragment shader
varying vec2 UV_Coord;		// Just passed on
varying vec3 constantColor;

/// Explicit out locations for the destination FrameBuffer 
// Use for later maybe, next pass.
// layout (location = 0) out vec4 diffuseOut;
uniform int pickingID = 1;
uniform vec4 primaryColorVec4 = vec4(1,1,1,1);

/// Gets attenuation based on vectors of constant, linear, quadratic + distance to object.
float GetAttenuation(vec3 att, float dist)
{
	return att.x / (1 + att.y * dist + att.z * pow(dist, 2)); 
}

void main()
{
	vec3 constantColor2 = vec3(0,0,0);
	// Texture image data. This will be the base for the colors.
	vec3 white = vec3(1,1,1);

	vec3 position = texture2D(positionMap, UV_Coord).xyz;
	vec3 vecToEye = position - eyePosition.xyz + vec3(0,5,0);
	/// Assume eye is at 0,0,0 for now
	float distanceTexEye = length(vecToEye);
	float intensity = 1.0f;
	intensity = 1.0f / (1.0f + distanceTexEye);
	
	vec3 diffuse = texture2D(diffuseMap, UV_Coord).xyz;
	vec3 specular = texture2D(specularMap, UV_Coord).xyz;
	vec3 normal = texture2D(normalMap, UV_Coord).xyz;
	vec3 normalizedNormal = normalize(vec3(normal.xyz));

	// DO NOTE: Alpha has to be Positive for the value to be printed out to the texture AT ALL! 
	vec3 color = vec3(0,0,0);

	// Luminosity for each color
	vec3 diffuseLuminosity = vec3(0,0,0);
	vec3 specularLuminosity = vec3(0,0,0);

	bool doLights = true;
	
	float distanceTexelEye = length(eyePosition.xyz - position);
//	if (distanceTexelEye > 500)
	//	doLights = false;
	if (doLights)
	{
		for (int i = 0; i < activeLights; ++i)
		{			
			vec3 lightLuminosity = vec3(0,0,0);
			vec3 lightSpecular = vec3(0,0,0);
					
	//		lightLuminosity.xyz = vec3(1,1,1);
	//		lightSpecular.xyz = vec3(1,1,1);
			// Directional lighting ^^
			if (light_type[i] == 2)
			{
			//	continue;
				// Diffuse
				vec3 lightDirection = normalize(light_position[i].xyz);
				float difIntensity = max(dot(lightDirection, normalizedNormal), 0.0);
				/// Opt out if the diffuse-intensity is 0. (L-Dot-N == 0) 
				/// The specular has no chance to be above 0 if this is the case!
				if (difIntensity <= 0)
					continue;
				lightLuminosity += difIntensity * light_diffuse[i].xyz; // * materialDiffuse;
			
				// Only appl7y constant light attenuation for the directional lights!?
				float distanceLightTexel = length(light_position[i].xyz - position.xyz);
				float attenuation = GetAttenuation(light_attenuation[i], distanceLightTexel);
				lightLuminosity *= attenuation;

				// Specular - intensity same, but consider the half vector and eye position.
				vec3 vectorToEye = normalize(vecToEye);
				vec3 halfVector = normalize((lightDirection + vectorToEye));
				float initialBrightness = max(dot(halfVector, normalizedNormal), 0.0);
				
				int smoothness = 12;
				float totalBrightness = initialBrightness;
				for (int i = 0; i < materialShininess; ++i){
					totalBrightness *= initialBrightness;
				}
				lightSpecular += totalBrightness * light_specular[i].xyz;
				lightSpecular *= attenuation;
				
			//	lightSpecular *= 0.1;
			//		lightSpecular *= 0;
			
			}
			
			// Positional lighting o-o;
			else if (light_type[i] == 1)
			{
		//		continue;				
//				continue;
				float distanceLightTexel = length(light_position[i].xyz - position.xyz);
				if (distanceLightTexel > 50)
					continue;
				vec3 attVec = light_attenuation[i];
				if (attVec.z <= 0.2)
					constantColor2.z = 1;
				float attenuation = GetAttenuation(light_attenuation[i], distanceLightTexel);
				if (attenuation < 0.01)
					continue;
	//			if (attenuation > 25)
//					constantColor2.x = attenuation / (attenuation + 0.1);
	
				// Diffuse
				vec3 lightDirection = normalize(position.xyz - light_position[i].xyz);
				
				float difIntensity = max(-dot(lightDirection, normalizedNormal), 0.0);
	//			constantColor.x = difIntensity;
	//
				/// Opt out if the diffuse-intensity is 0. (L-Dot-N == 0) 
				/// The specular has no chance to be above 0 if this is the case!
				if (difIntensity <= 0)
					continue;
							
				lightLuminosity += difIntensity * light_diffuse[i].xyz; // * materialDiffuse;
				
			//	gl_FragColor.xyz += lightLuminosity * texel.xyz;


				
				// Specular - intensity same, but consider the half vector and eye position.
				// First calc vector to eye from the fragment
				vec3 vectorToEye = normalize(vecToEye);
			
				// Calculate the mid-way vector between the angle of the light and the vector to the eye.
				// Since the light direction is in the opposite direction at first, negate it.
				vec3 halfVector = normalize((-lightDirection + vectorToEye) / 2);
				float specularBrightness = max(dot(halfVector, normalizedNormal), 0.0);
				float totalSpecularBrightness = pow(specularBrightness, materialShininess);
				lightSpecular += totalSpecularBrightness * light_specular[i].xyz;
				
				/// Apply attenuation.
				lightSpecular *= attenuation;
				lightLuminosity *= attenuation;		
			}
			// Spotlights!!!
			else if (light_type[i] == 3)
			{
				/// First compare lightDirection with the spot's direction.
				vec3 lightToTexel = position.xyz - light_position[i].xyz;
				vec3 lightToTexelDirection = normalize(lightToTexel);		
				float spotDotCutoff = dot(normalize(light_spotDirection[i]), lightToTexelDirection);
				/// 0.3 cutoff, (1-0.3) -> 0.7, div 2, 0.35, 1-x, 0.65
				float cutoff = light_spotCutoff[i];
				if (spotDotCutoff > cutoff)
				{
					float spotIntensity = 1;
					float difIntensity = max(dot(-lightToTexelDirection, normalizedNormal), 0.0);
					
					float distance = length(lightToTexel);
					float att = GetAttenuation(light_attenuation[i], distance);
					spotIntensity = difIntensity * att;

					// Apply cutoff and spot exponential equations.
					float cutoffMin = 1 - (1 - cutoff) / 2;
					float distIn = spotDotCutoff - cutoff;
					float distToMin = cutoffMin - cutoff;
					float relDist = distIn / distToMin;
					float relativeCutoffLuminosity = min(distIn / distToMin, 1);
					float towardsOuterCutoff = distIn; //(1 - );
					if (distIn > distToMin)
						spotIntensity *= 1; // light_; cutoff
					else
						spotIntensity *= pow(relDist, light_spotExponent[i]);

					
					// Specular - intensity same, but consider the half vector and eye position.
					vec3 vectorToEye = normalize(vecToEye);					
					vec3 halfVector = normalize((-lightToTexelDirection + vectorToEye));
					float specularBrightness = max(dot(halfVector, normalizedNormal), 0.0);
					float totalSpecularBrightness = pow(specularBrightness, materialShininess);
					
					lightLuminosity += spotIntensity * light_diffuse[i].xyz;
					lightSpecular += spotIntensity * totalSpecularBrightness * light_specular[i].xyz;
					
					
					/// Apply the spot intensity to both the added specular and diffuse luminosity
					lightSpecular *= spotIntensity;
					lightLuminosity *= spotIntensity;
				}
			//	lightLuminosity *= 0;
			}

			// o.o;
			if (light_castsShadows[i] >= 0)
			{
//				continue;
				// replace diffuse-map with the shadow-map?
				int shadowSamples = 9;
				/// Should use like, 1 sample, 3, 5, or 9, 13, etc.
				float inShadow = 0;
				float ssd = 0.00025; /// shadow sampler distance
				float ssd2 = ssd*0.5;
				for (int i = 0; i < shadowSamples; ++i)
				{
					// Check stuff.
					vec3 shadowPos = position;
					vec4 shadowSpace = shadowMapMatrix * vec4(shadowPos, 1);
					if (i == 1) 		shadowSpace.xy += vec2(ssd,ssd);  // 
					else if (i == 2)	shadowSpace.xy += vec2(-ssd,-ssd); //-
					else if (i == 3)	shadowSpace.xy += vec2(ssd,-ssd);
					else if (i == 4)	shadowSpace.xy += vec2(-ssd,ssd);
					else if (i == 5)	shadowSpace.xy += vec2(-ssd,0); // +
					else if (i == 6)	shadowSpace.xy += vec2(ssd,0);
					else if (i == 7)	shadowSpace.xy += vec2(0,-ssd);
					else if (i == 8)	shadowSpace.xy += vec2(0,ssd);
					else if (i == 9)	shadowSpace.xy += vec2(ssd2,ssd); // / & \ ?
					else if (i == 10)	shadowSpace.xy += vec2(-ssd2,-ssd); // / & \ ?
					else if (i == 11)	shadowSpace.xy += vec2(-ssd2,ssd); // / & \ ?
					else if (i == 12)	shadowSpace.xy += vec2(ssd2,-ssd); // / & \ ?
					else if (i == 13)	shadowSpace.xy += vec2(ssd,ssd2); // Like 9 but horizontal
					else if (i == 14)	shadowSpace.xy += vec2(-ssd,-ssd2); // / & \ ?
					else if (i == 15)	shadowSpace.xy += vec2(ssd,-ssd2); // / & \ ?
					else if (i == 16)	shadowSpace.xy += vec2(-ssd,ssd2); // / & \ ?
					
					float depthAtLocationInShadowMap = texture2D(shadowMap, shadowSpace.xy).z;
					float fragmentDepthInLightSpace = shadowSpace.z;
					fragmentDepthInLightSpace = fragmentDepthInLightSpace;
					if (depthAtLocationInShadowMap < fragmentDepthInLightSpace - 0.001)
						inShadow += 1;
				// IF false positive, due to co-ordinates outside the map, make it shadow? If not including all objects, instead extend the shadow map/camera properties of the light rendering setup?
				if (shadowSpace.x > 1 || shadowSpace.x < 0 ||
					shadowSpace.y > 1 || shadowSpace.y < 0 ||
					shadowSpace.z > 1 || shadowSpace.z < 0)
					inShadow += 2;

			//		constantColor.xyz = vec3(1,1,1) * depthAtLocationInShadowMap;
			//		constantColor.xyz = vec3(1,1,1) * fragmentDepthInLightSpace;
			//			constantColor.x = shadowSpace.x;					
				}
				inShadow /= shadowSamples;
				lightSpecular.xyz *= 1-inShadow;
				lightLuminosity.xyz *= 1-inShadow;
			}

			
			// Add the diffuseLuminosity
			diffuseLuminosity += lightLuminosity;
			specularLuminosity += lightSpecular;
		}	
	}

	// Final illumination done, multiply with diffuse ^^
	vec3 diffuseTotal = diffuse.xyz * diffuseLuminosity * materialDiffuse.xyz;
	vec3 specularTotal = vec3(0,0,0);
	/// Check if using specular map.
	vec4 specTex = texture2D(specularMap, UV_Coord);
	specularTotal.xyz += specTex.xyz * specularLuminosity * materialSpecular.xyz;
	
	// Sample emissive map?
	vec3 emissive = texture2D(emissiveMap, UV_Coord).xyz;
	
	/// Add global ambient ^^
	vec3 diffuseFactor = clamp(diffuseTotal.xyz, 0, 1);
	vec3 specularFactor = clamp(specularTotal, 0, 1);
	vec3 emissiveFactor = clamp(emissive*2, 0, 2);
	vec3 ambientFactor = clamp(diffuse.xyz * light_ambient.xyz, 0, 1);
	
	color = diffuseFactor + specularFactor + emissiveFactor + ambientFactor;

	if (constantColor.x != 0)
		color = constantColor;
	if (constantColor2.x != 0)
		color = constantColor2;
//	color.x += distance;
//	color = specTex.xyz;
//	color = diffuse.xyz;
		
	diffuseOut = vec4(color, 1);
	
//	gl_FragColor.xyz = color;
//	gl_FragColor.w = distance;
	gl_FragDepth = texture2D(depthMap, UV_Coord).x;
//	gl_Position.w = distance;
	return;
}

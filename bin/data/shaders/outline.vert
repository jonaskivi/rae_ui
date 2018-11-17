#version 120

// Input vertex data
attribute vec3 inPosition;
attribute vec2 inUV; //unused TODO remove
attribute vec3 inNormal;

// Constant data
uniform mat4 modelViewProjectionMatrix;
uniform float screenSizeFactor;

void main()
{
	gl_Position = modelViewProjectionMatrix * vec4(inPosition + (inNormal * screenSizeFactor), 1.0);
}

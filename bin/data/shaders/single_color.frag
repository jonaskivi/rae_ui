#version 120

// Constant data
uniform vec3 lineColor;

void main()
{
	//float distanceToLight = length( lightPosition_worldspace - position_worldspace );
	gl_FragColor.rgb = lineColor;
}

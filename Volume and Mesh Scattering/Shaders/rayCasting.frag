#version 330
uniform sampler1D transferFunctionText;
uniform sampler2D backFaceText;
uniform sampler3D volumeText;
uniform vec2 screenSize;
uniform float stepSize;
uniform vec3 lightPos;
uniform bool lighting;
uniform vec3 viewPos;
uniform vec3 ambientComp;
uniform vec3 diffuseComp;
uniform vec3 specularComp;
in vec3 inCoordinate;
in vec3 positionFrag;

out vec4 fragColor;

vec3 getGradient(vec3 rayPosition) 
{ 	
	float x1, x2, y1, y2, z1, z2, lenghtGradient;
    vec3 gradient, normal;
    x1 = texture3D(volumeText, rayPosition - vec3(0.01, 0.0, 0.0)).r;
    x2 = texture3D(volumeText, rayPosition + vec3(0.01, 0.0, 0.0)).r;
	y1 = texture3D(volumeText, rayPosition - vec3(0.0, 0.01, 0.0)).r;
    y2 = texture3D(volumeText, rayPosition + vec3(0.0, 0.01, 0.0)).r;
    z1 = texture3D(volumeText, rayPosition - vec3(0.0, 0.0, 0.01)).r;
    z2 = texture3D(volumeText, rayPosition + vec3(0.0, 0.0, 0.01)).r;
    gradient.x = x2 - x1;
    gradient.y = y2 - y1;
    gradient.z = z2 - z1;	
    lenghtGradient = length (gradient);
    normal = vec3(0.0, 1.0, 0.0); 
	if (lenghtGradient > 0.0) 
		normal = gradient / lenghtGradient;
    return normal;
}

vec4 illuminate(vec3 position, vec4 actualColor) 
{ 
	vec3 gradient, N, L, V;
	float diffuse, specular;
	gradient = getGradient(position);
    gradient = gradient * vec3(2.0) - vec3(1.0);
	N = normalize(normalize(lightPos) - position);	  
    L = normalize(lightPos - positionFrag);
	V = normalize(viewPos - positionFrag);
	diffuse = abs(dot(N, gradient));
	specular = pow(max(dot(N, normalize(L + V)), 0.0), 64.0);
	actualColor.rgb = actualColor.rgb * (ambientComp + (diffuseComp * diffuse) + (specularComp * specular));	
	return actualColor;
}

vec4 rayCasting(vec3 direction, float lenghtInOut)
{
	vec4 accumulatedColor, actualColor;
	float i, density;
	vec3 position, stepRay;
	stepRay = direction * stepSize;
	accumulatedColor = vec4(0.0, 0.0, 0.0, 1.0);
	position = inCoordinate;
	for(i = 0.0f; i < lenghtInOut; i += stepSize)
	{
		density = texture(volumeText, position).x;
		actualColor = texture(transferFunctionText, density);
		if (lighting)
			actualColor = illuminate(position, actualColor);
    	actualColor.a = 1.0 - exp(-0.5 * actualColor.a);
    	accumulatedColor.rgb += accumulatedColor.a * actualColor.rgb * actualColor.a;
    	accumulatedColor.a *= (1.0 - actualColor.a);
		if (1.0 - accumulatedColor.a > 0.95) break;
		position += stepRay;
	}
	accumulatedColor.a = 1.0 - accumulatedColor.a;
	return accumulatedColor;
}

void main()
{
	vec3 outCoordinate, direction, rayStep;
	float lenghtInOut;
	outCoordinate = texture(backFaceText, gl_FragCoord.st/screenSize.xy).xyz;
	if (inCoordinate != outCoordinate)
	{
		direction = outCoordinate - inCoordinate;
		lenghtInOut = length(direction);
		fragColor = rayCasting(normalize(direction), lenghtInOut);
	}
	else
		fragColor = vec4(1.0 , 1.0, 1.0, 0.0);
}

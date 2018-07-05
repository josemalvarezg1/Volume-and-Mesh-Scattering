#version 330
//Falta Combinacion, kernel en vecindad
uniform sampler1D transfer_function_text;
uniform sampler2D back_face_text;
uniform sampler3D volume_text;
uniform sampler3D light_volume_text;
uniform vec2 screen_size;
uniform float step_size;
uniform int num_of_lights;
uniform vec3 light_pos;
uniform bool lighting;
uniform vec3 camera_pos;
uniform vec4 back_radiance;

in vec3 in_coord;
in vec3 frag_pos;

out vec4 color;

vec3 get_gradient(vec3 ray_position) 
{ 	
	float x1, x2, y1, y2, z1, z2, lenght_gradient;
    vec3 gradient, normal;
    x1 = texture(volume_text, ray_position - vec3(0.01, 0.0, 0.0)).r;
    x2 = texture(volume_text, ray_position + vec3(0.01, 0.0, 0.0)).r;
	y1 = texture(volume_text, ray_position - vec3(0.0, 0.01, 0.0)).r;
    y2 = texture(volume_text, ray_position + vec3(0.0, 0.01, 0.0)).r;
    z1 = texture(volume_text, ray_position - vec3(0.0, 0.0, 0.01)).r;
    z2 = texture(volume_text, ray_position + vec3(0.0, 0.0, 0.01)).r;
    gradient.x = x2 - x1;
    gradient.y = y2 - y1;
    gradient.z = z2 - z1;	
    lenght_gradient = length(gradient);
    normal = vec3(0.0, 1.0, 0.0); 
	if (lenght_gradient > 0.0) 
		normal = gradient / lenght_gradient;
    return normal;
}

vec4 illuminate(vec3 position, vec4 actual_color) 
{ 
	vec3 gradient, N, L, V;
	float diffuse, specular;
	gradient = get_gradient(position);
    gradient = gradient * vec3(2.0) - vec3(1.0);
	N = normalize(normalize(light_pos) - position);	  
    L = normalize(light_pos - frag_pos);
	V = normalize(camera_pos - frag_pos);
	diffuse = abs(dot(N, gradient));
	specular = pow(max(dot(N, normalize(L + V)), 0.0), 64.0);
	actual_color.rgb = actual_color.rgb * (diffuse + specular);	
	return actual_color;
}

vec4 ray_casting(vec3 direction, float lenght_in_out)
{
	vec4 accumulated_color, actual_color, light_color;
	float i, density, shadow;
	vec3 position, ray_step, c, li;
	ray_step = direction * step_size;
	accumulated_color = vec4(0.0, 0.0, 0.0, 1.0);
	position = in_coord;
	for(i = 0.0f; i < lenght_in_out; i += step_size)
	{
		density = texture(volume_text, position).x;
		light_color = texture(light_volume_text, position).rgba;
		//actual_color = light_color;
		actual_color = texture(transfer_function_text, density) * vec4(light_color.rgb, 1.0f) * light_color.a;
		//actual_color = texture(transfer_function_text, density);
		/*if (lighting)
			actual_color = illuminate(position, actual_color);*/
    	//actual_color.a = 1.0 - exp(-0.5 * actual_color.a);
    	accumulated_color.rgb += accumulated_color.a * actual_color.rgb * actual_color.a;
    	accumulated_color.a *= (1.0 - actual_color.a);
		if (1.0 - accumulated_color.a > 0.95) break;
		position += ray_step;
	}
	accumulated_color.a = 1.0 - accumulated_color.a;
	return accumulated_color;
}

void main()
{
	vec3 out_coord, direction, ray_step;
	float lenght_in_out;
	out_coord = texture(back_face_text, gl_FragCoord.st/screen_size.xy).xyz;
	if (in_coord != out_coord)
	{
		direction = out_coord - in_coord;
		lenght_in_out = length(direction);
		color = ray_casting(normalize(direction), lenght_in_out);
	}
	else
		color = vec4(1.0, 1.0, 1.0, 0.0);
}

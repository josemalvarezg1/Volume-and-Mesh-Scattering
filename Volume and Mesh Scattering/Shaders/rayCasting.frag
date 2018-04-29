#version 330
uniform sampler1D transfer_function_text;
uniform sampler2D back_face_text;
uniform sampler3D volume_text;
uniform vec2 screen_size;
uniform float step_size;
uniform int num_of_lights;
uniform vec3 light_pos[3];
uniform bool lighting[3];
uniform vec3 camera_pos;
uniform vec3 ambient_comp[3];
uniform vec3 diffuse_comp[3];
uniform vec3 specular_comp[3];
uniform float radius;
uniform float asymmetry_param_g;
uniform vec4 back_radiance;
uniform vec3 scattering_coeff;
uniform vec3 extinction_coeff;

in vec3 in_coord;
in vec3 frag_pos;

out vec4 color;

vec3 calculate_ni_ast(vec3 xo, vec3 xi, vec3 ni)
{
	if (xo == xi)
		return ni;
	else
	{
		vec3 factor_1, factor_2;
		factor_1 = normalize(xo - xi);
		factor_2 = normalize(cross(ni, xo - xi));
		return cross(factor_1, factor_2);
	}
}

vec3 calculate_dr_pow(vec3 r, vec3 D, float miu_0, vec3 de, vec3 cos_beta, vec3 attenuation_coeff)
{
	if (miu_0 > 0.0f)
		return pow(r, vec3(2.0f)) + D * miu_0 * (D * miu_0 - 2 * de * cos_beta);
	else
		return pow(r, vec3(2.0f)) + 1 / (pow(3 * attenuation_coeff, vec3(2.0f)));
}

vec3 diffuse_part_prime(vec3 x, vec3 w12, vec3 r, vec3 no)
{
	vec3 effec_r, one_effec_r, factor_1, factor_2, factor_3, factor_4, effective_transport_coeff, c_phi_1, c_phi_2, D, c_e;
	float x_dot_w12, w12_dot_no, x_dot_no;
	effec_r = effective_transport_coeff * r;
	one_effec_r = vec3(1.0f) + effec_r;
	x_dot_w12 = dot(x, w12);
	w12_dot_no = dot(w12, no);
	x_dot_no = dot(x, no);
	factor_1 = (1 / (4 * c_phi_2)) * 2.4674011 * (exp(-effec_r) / pow(r, vec3(3.0f)));
	factor_2 = c_phi_1 * ((r * r) / D + 3 * one_effec_r * x_dot_w12);
	factor_3 = 3 * D * one_effec_r * w12_dot_no;
	factor_4 = (one_effec_r + 3 * D * (3 * one_effec_r + effec_r * effec_r) / (r * r) * x_dot_w12) * x_dot_no;
	return factor_1 * (factor_2 - c_e * (factor_3 - factor_4));
}

float fresnel_t(vec3 inv, vec3 n, float n_1)
{
	float cos_i, n_2, eta, sin_t;
	cos_i = clamp(-1.0f, 1.0f, dot(inv, n));
	n_2 = 1.0f;
	eta = n_1 / n_2;

	sin_t = eta * sqrt(max(0.0f, 1.0f - cos_i * cos_i));
	if (sin_t >= 1.0f)
		return 1.0f;

	float cos_t, const_t, cos_i_2, factor_1, factor_2, Ts, Tp;
	cos_t = sqrt(max(0.0f, 1.0f - sin_t * sin_t));

	const_t = eta * (cos_t / cos_i);
	cos_i_2 = (2 * cos_i);
	factor_1 = cos_i_2 / ((eta * cos_i) + cos_t);
	factor_2 = cos_i_2 / ((eta * cos_t) + cos_i);

	Ts = const_t * (factor_1 * factor_1);
	Tp = const_t * (factor_2 * factor_2);

	return (Ts + Tp) / 2.0f;
}

//void BSSRDF(vec3 frag_pos, vec3 frag_normal)
//{
//	vec3 xo, no, wo, Lo, Ll, xi, ni, wi, x, r, dr, dr_pow, w12, p, ni_ast, xv, dv, wv;
//	vec3 diffuse_part_prime_1, diffuse_part_prime_2, diffuse_part_d;
//	vec3 cos_beta, z_prime, R, T, diffuse_part;
//	float miu_0, Ti, To, theta;
//	mat2 rotation_samples_matrix;
//	vec4 light_diff;
//
//	xo = frag_pos;
//	no = normalize(frag_normal);
//	wo = normalize(camera_pos - frag_pos);
//
//	Lo = vec3(0.0f);
//	Ll = light_diff.xyz; 		/*Hasta ahora una sola luz por la parte que vamos en el paper*/
//
//	wi = normalize(light_pos);
//
//	/* Inicio: Generación de muestras */
//
//	for (int i = 0; i < 64; i++)
//	{
//		vec4 offset = vec4(xo, 1.0f);
//		// De espacio de vista a espacio de clipping
//		offset = vp_light * offset;
//		offset.xyz /= offset.w;
//		// El offset estará en un rango [0:1]
//		offset.xyz = offset.xyz * 0.5 + 0.5;
//		offset.xy += samples[i].xy;
//
//		theta = 2 * PI * radius;
//		rotation_samples_matrix = mat2(vec2(cos(theta), sin(theta)), vec2(-sin(theta), cos(theta)));
//		offset.xy = rotation_samples_matrix * offset.xy;
//
//		xi = texture(g_position, offset.xy).xyz;
//		ni = texture(g_normal, offset.xy).xyz;
//
//		float visibility = 1.0f;
//		float bias = 0.005 * tan(acos(dot(no, wi)));
//		bias = clamp(bias, 0.0f, 0.01f);
//		if (texture(g_depth, offset.xy).r  <  offset.z - bias)
//		{
//			visibility = 0.0;
//		}
//
//		if (visibility > 0.0f)
//		{
//			x = xo - xi;
//			r = vec3(length(x));
//			w12 = refract(wi, ni, refractive_index);
//
//			/* Inicio: Parte Difusa */
//
//			ni_ast = calculate_ni_ast(xo, xi, ni);
//
//			xv = xi + (2 * A * de * ni_ast);
//			dv = vec3(length(xo - xv));
//			wv = w12 - (2 * (dot(w12, ni_ast)) * ni_ast);
//
//			cos_beta = -sqrt((pow(r, vec3(2.0f)) - pow(dot(x, w12), 2)) / (pow(r, vec3(2.0f)) + pow(de, vec3(2.0f))));
//			miu_0 = dot(-no, w12);
//			dr_pow = calculate_dr_pow(r, D, miu_0, de, cos_beta, attenuation_coeff);
//			dr = sqrt(dr_pow);
//
//			diffuse_part_prime_1 = diffuse_part_prime(x, w12, dr, no);
//			diffuse_part_prime_2 = diffuse_part_prime(xo - xv, wv, dv, no);
//			diffuse_part_d = diffuse_part_prime_1 - diffuse_part_prime_2;
//
//			Ti = fresnel_t(wi, ni, refractive_index);
//			To = fresnel_t(wo, no, refractive_index);
//
//			diffuse_part = (Ti * diffuse_part_d * dot(ni, wi));
//
//			Lo += diffuse_part;
//
//			/* Fin: Parte Difusa */
//		}
//	}
//
//	Lo *= ((PI * Ll) / n_samples);
//
//	/* Fin: Generación de muestras */
//
//	color = vec4(Lo * diffuse_reflectance, 1.0f);
//}

vec3 get_gradient(vec3 ray_position) 
{ 	
	float x1, x2, y1, y2, z1, z2, lenght_gradient;
    vec3 gradient, normal;
    x1 = texture3D(volume_text, ray_position - vec3(0.01, 0.0, 0.0)).r;
    x2 = texture3D(volume_text, ray_position + vec3(0.01, 0.0, 0.0)).r;
	y1 = texture3D(volume_text, ray_position - vec3(0.0, 0.01, 0.0)).r;
    y2 = texture3D(volume_text, ray_position + vec3(0.0, 0.01, 0.0)).r;
    z1 = texture3D(volume_text, ray_position - vec3(0.0, 0.0, 0.01)).r;
    z2 = texture3D(volume_text, ray_position + vec3(0.0, 0.0, 0.01)).r;
    gradient.x = x2 - x1;
    gradient.y = y2 - y1;
    gradient.z = z2 - z1;	
    lenght_gradient = length (gradient);
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

	for (int i = 0; i < num_of_lights; i++) {
		if (lighting[i]) {
			N = normalize(normalize(light_pos[i]) - position);
			L = normalize(light_pos[i] - frag_pos);
			V = normalize(camera_pos - frag_pos);
			diffuse = abs(dot(N, gradient));
			specular = pow(max(dot(N, normalize(L + V)), 0.0), 64.0);
			actual_color.rgb = actual_color.rgb * (ambient_comp[i] + (diffuse_comp[i] * diffuse) + (specular_comp[i] * specular));
		}
	}
	return actual_color;
}

vec4 ray_casting(vec3 direction, float lenght_in_out)
{
	vec4 accumulated_color, actual_color;
	float i, density;
	vec3 position, ray_step;
	ray_step = direction * step_size;
	accumulated_color = vec4(0.0, 0.0, 0.0, 1.0);
	position = in_coord;
	for(i = 0.0f; i < lenght_in_out; i += step_size)
	{
		density = texture(volume_text, position).x;
		actual_color = texture(transfer_function_text, density);
		actual_color = illuminate(position, actual_color);
    	actual_color.a = 1.0 - exp(-0.5 * actual_color.a);
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
		color = vec4(1.0 , 1.0, 1.0, 0.0);
}

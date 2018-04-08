#version 330

in vec3 frag_pos;
in vec3 frag_normal;

uniform sampler2DArray scattered_map;
uniform sampler2DArray depth_map;
uniform vec3 camera_pos;
uniform vec3 light_pos;
uniform float bias;
uniform float epsilon;
uniform float refractive_index;
uniform int n_cameras;
uniform mat4 cameras_matrix[16];
uniform vec3 cameras_dirs[16];
uniform float gamma;
uniform int current_frame;

out vec4 color;

#define ARRAY_TEX_STEP 1.0f / 1024.0f
#define KERNEL_SIZE 5

const vec2 kernel[KERNEL_SIZE] = {
	vec2(0),
	0.5 * ARRAY_TEX_STEP * vec2(-1.0f, 3.0f),
	0.5 * ARRAY_TEX_STEP * vec2(3.0f, 1.0f),
	0.5 * ARRAY_TEX_STEP * vec2(1.0f, -3.0f),
	0.5 * ARRAY_TEX_STEP * vec2(-3.0f, 1.0f)
};

float sample_shadow_map(vec4 object_pos)
{
	float closestdepth = texture(depth_map, object_pos.xyz).r; 
    float currentdepth = object_pos.w;
    float shadow = 0.0;
    vec2 texelsize = vec2(1.0f / 1200.0f, 1.0f / 680.0f);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfdepth = texture(depth_map, vec3(object_pos.xy + vec2(x, y) * texelsize, object_pos.w)).r; 
            shadow += currentdepth - bias > pcfdepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    if(object_pos.w > 1.0)
        shadow = 0.0;
        
    return 1.0 - shadow;
}

vec4 sample_color_map(vec3 coord)
{
	return texture(scattered_map, coord);
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

void main(void)
{
	vec3 no = normalize(frag_normal);
	vec3 wo = normalize(camera_pos - frag_pos);
	color = vec4(0.0f);
	float div = 0.0f, vi;
	vec3 pos, offset, dir;
	vec4 l;
	for (int i = 0; i < n_cameras; i++)
	{
		dir = cameras_dirs[i];
		offset = epsilon * (no - dir * dot(no, dir));
		pos = frag_pos + offset;
		l = cameras_matrix[i] * vec4(pos, 1.0f);
		l.xyz /= l.w;
		l = l * 0.5 + 0.5;
		float visibility = 1.0f;
		vec4 kernel_j = vec4(l.xy, i, l.z);
		vi = sample_shadow_map(kernel_j.xyzw);
		visibility *= vi;
		
		vec4 sample_color_map = sample_color_map(vec3(l.xy, i));
		//color += sample_color_map;
		color += (sample_color_map / max(sample_color_map.a, 1.0f)) * visibility;
		div += visibility;
	}

	color /= max(div, 1.0);

	float fresnel = fresnel_t(wo, no, 1.0f / refractive_index);
	color *= clamp(fresnel, 0.0f, 1.0f);
	//color = vec4(vec3(vi), 1.0); 
}
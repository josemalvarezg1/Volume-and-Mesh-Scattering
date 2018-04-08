#version 430
in vec3 coord_text;

uniform sampler2DArray scattered_map;
uniform int scaling;

out vec4 color;

vec4 sampleTex(in sampler2DArray smp, in vec3 texcoord)
{
    return texture(smp,texcoord);
}

void main(void)
{
	vec4 c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, step_1, step_2, step_3;
	float adj_step, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, vstep_1, vstep_2, vstep_3;

	int current_layer = gl_Layer;
	adj_step = 1.0f / 1024.0f * 0.5f * scaling;
    
	c0  = sampleTex(scattered_map, vec3(coord_text.xy, current_layer));
    c1  = sampleTex(scattered_map, vec3(coord_text.xy + vec2(adj_step, 0.0f), current_layer));
    c2  = sampleTex(scattered_map, vec3(coord_text.xy - vec2(adj_step, 0.0f), current_layer));
    c3  = sampleTex(scattered_map, vec3(coord_text.xy + vec2(0.0f, adj_step), current_layer));
    c4  = sampleTex(scattered_map, vec3(coord_text.xy - vec2(0.0f, adj_step), current_layer));

    c5  = sampleTex(scattered_map, vec3(coord_text.xy + 2 * vec2(adj_step, 0.0f), current_layer));
    c6  = sampleTex(scattered_map, vec3(coord_text.xy - 2 * vec2(adj_step, 0.0f), current_layer));
    c7  = sampleTex(scattered_map, vec3(coord_text.xy + 2 * vec2(0.0f, adj_step), current_layer));
    c8  = sampleTex(scattered_map, vec3(coord_text.xy - 2 * vec2(0.0f, adj_step), current_layer));

    c9  = sampleTex(scattered_map, vec3(coord_text.xy + vec2(adj_step, adj_step), current_layer));
    c10 = sampleTex(scattered_map, vec3(coord_text.xy - vec2(adj_step, adj_step), current_layer));
    c11 = sampleTex(scattered_map, vec3(coord_text.xy + vec2(adj_step, -adj_step), current_layer));
    c12 = sampleTex(scattered_map, vec3(coord_text.xy - vec2(adj_step, -adj_step), current_layer));

    v0  = clamp(c0.a, 0.0f, 1.0f);
    v1  = clamp(c1.a, 0.0f, 1.0f);
    v2  = clamp(c2.a, 0.0f, 1.0f);
    v3  = clamp(c3.a, 0.0f, 1.0f);
    v4  = clamp(c4.a, 0.0f, 1.0f);
    v5  = clamp(c5.a, 0.0f, 1.0f);
    v6  = clamp(c6.a, 0.0f, 1.0f);
    v7  = clamp(c7.a, 0.0f, 1.0f);
    v8  = clamp(c8.a, 0.0f, 1.0f);
    v9  = clamp(c9.a, 0.0f, 1.0f);
    v10 = clamp(c10.a, 0.0f, 1.0f);
    v11 = clamp(c11.a, 0.0f, 1.0f);
    v12 = clamp(c12.a, 0.0f, 1.0f);

    step_1 = c0 * v0 + c1 * v1 + c2 * v2 + c3 * v3 + c4 * v4;
    vstep_1 = v0 + v1 + v2 + v3 + v4;
    step_2 = c5 * v5 + c6 * v6 + c7* v7 + c8 * v8;
    vstep_2 = v5 + v6 + v7 + v8;
    step_3 = c9 * v9 + c10 * v10 + c11 * v11 + c12 * v12;
    vstep_3 = v9 + v10 + v11 + v12;

    color = (step_1 + step_2 + step_3 ) / max(vstep_1 + vstep_2 + vstep_3, 1.0f);
}
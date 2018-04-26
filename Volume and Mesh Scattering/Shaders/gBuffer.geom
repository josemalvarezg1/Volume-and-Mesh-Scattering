#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices = 60) out;
uniform int num_of_lights;
uniform mat4 vp_light[3];

out vec3 frag_pos;
out vec3 frag_normal;

in VS_OUT
{
	vec3 frag_pos;
	vec3 frag_normal;
} gs_in[];

void main(void)
{
	for (int i = 0; i < num_of_lights; i++)
	{
		gl_Layer = i;
		for (int k = 0; k < 3; k++)
		{
			gl_Position = vp_light[i] * vec4(gs_in[k].frag_pos, 1.0);
			frag_normal = gs_in[k].frag_normal;
			frag_pos = gs_in[k].frag_pos;
			EmitVertex();
		}
		EndPrimitive();
	}
}
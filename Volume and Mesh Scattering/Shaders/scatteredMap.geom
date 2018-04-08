#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices = 60) out;
uniform mat4 model_matrix;
uniform int n_cameras;
uniform mat4 cameras_matrix[16];

out vec3 frag_pos;
out vec3 frag_normal;

in VS_OUT
{
	vec3 frag_pos;
	vec3 frag_normal;
} gs_in[];

void main(void)
{
	for (int i = 0; i < n_cameras; i++)
	{
		gl_Layer = i;
		for (int k = 0; k < 3; k++)
		{
			gl_Position = cameras_matrix[i] * vec4(gs_in[k].frag_pos, 1.0);
			frag_normal = gs_in[0].frag_normal;
			frag_pos = gs_in[0].frag_pos;
			EmitVertex();
		}
		EndPrimitive();
	}
}
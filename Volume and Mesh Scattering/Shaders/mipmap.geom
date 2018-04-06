#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices = 93) out;

uniform int n_cameras;

in vec3 geom_coord_text[3];

out vec3 coord_text;
out int current_layer;

void main(void)
{
	for (int i = 0; i < n_cameras; i++)
	{
		gl_Layer = i;
		current_layer = i;
		for (int k = 0; k < 3; k++)
		{
            coord_text = geom_coord_text[k];
			gl_Position = gl_in[k].gl_Position;
			EmitVertex();
		}
		EndPrimitive();
	}
}
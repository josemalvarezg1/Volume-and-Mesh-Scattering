#version 430

layout(location = 0) out vec4 out_color;

uniform sampler2D actual_text;
uniform ivec3 volume_size;
uniform mat4 vp_matrix;

uniform layout(binding = 1, rgba16f) writeonly image3D vol_ilum;

in vec3 in_coord;
in vec3 frag_pos;

void main()
{
	vec4 color, offset, I_0, I_1, S;
	ivec3 size;
	vec2 texel_size;

	color = vec4(0.0f);

	offset = vec4(frag_pos, 1.0f);
	offset = vp_matrix * offset;
	offset.xyz /= offset.w;
	offset.xyz = offset.xyz * 0.5 + 0.5;
	color = texture(actual_text, offset.xy);

	size = ivec3(in_coord.x * volume_size.x, in_coord.y * volume_size.y, in_coord.z * volume_size.z);
	imageStore(vol_ilum, size, color);

	out_color = color;
}
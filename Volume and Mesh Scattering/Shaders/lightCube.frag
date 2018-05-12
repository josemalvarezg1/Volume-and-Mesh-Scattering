#version 330

uniform sampler1D transfer_function_text;
uniform sampler3D volume_text;
uniform sampler2D previous_text;
//uniform layout(binding=10, rgba16f) writeonly image2D vol_ilum;


in vec2 in_coord;
out vec4 color;

void main() 
{	
	//float value;
	//vec4 accumulated_color, actual_color;

	//value = texture(volume_text, in_coord).r;
 //	actual_color = texture(transfer_function_text, value);
 	// accumulated_color = texture(previous_text, vec2(in_coord));

	//color.rgb = (1.0f - actual_color.a) * accumulated_color.rgb + actual_color.a * actual_color.rgb;
	//color.a = (1.0f - actual_color.a) * accumulated_color.a + actual_color.a;
	//imageStore(vol_ilum, ,color);
}
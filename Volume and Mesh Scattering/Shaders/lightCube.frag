#version 330

uniform sampler1D transfer_function_text;
uniform sampler2D quad_text;
uniform sampler2D previous_quad_text;

in vec2 in_coord;
out vec4 color;

void main() {	

	float value = texture2D(quad_text, in_coord).r;
 	color = texture1D(transfer_function_text, value);
 	
}
$input a_position, a_color0
$output v_position, v_color0

#include <bgfx_shader.sh>

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
	v_position = gl_Position.xyz;
	v_color0 = a_color0;
}
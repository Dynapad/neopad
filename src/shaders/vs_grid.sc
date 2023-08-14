$input a_position, a_color0
$output v_color0

#include <bgfx_shader.sh>

void main()
{
	// The grid quad is already an NDC unit quad (-1., 1., 1., -1.).
	// So, just pass it through.
	gl_Position = a_position;
	v_color0 = a_color0;
}
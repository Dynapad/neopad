$input v_position, v_color0

#include <bgfx_shader.sh>
#include "uniforms.sh"

float grid(vec2 st, float res){
    vec2 grid = fract(st * res);
    return 1. - (step(res, grid.x) * step(res, grid.y));
}

void main()
{
    vec2 st = v_position.xy;
    vec3 color = vec3(0.0);

    vec2 grid_st = st * vec2(u_viewRect.z / 2.0, u_viewRect.w / 2.0) + vec2(0.5, 0.5);
    color += vec3(0.2) * grid(grid_st, 1.0 / u_content_scale * 0.01);
    color += vec3(0.1) * grid(grid_st, 1.0 / u_content_scale * 0.02);

	gl_FragColor = vec4(color, 1.0);
}
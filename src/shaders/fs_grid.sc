$input v_position, v_color0

#include <bgfx_shader.sh>
#include "uniforms.sh"

float grid(vec2 st, float res) {
    vec2 grid = fract(st * res);
    return 1. - (step(res, grid.x) * step(res, grid.y));
}

float x_axis(vec2 st) {
    return (step(-0.002, st.y) - step(0.002, st.y));
}

float y_axis(vec2 st) {
    return (step(-0.001, st.x) - step(0.001, st.x));
}

void main()
{
    vec2 st = v_position.xy;
    vec3 color = vec3(0.0);

    float major = (1.0 / u_grid_major);
    float minor = (1.0 / u_grid_minor);

    vec2 grid_st = st * vec2(u_viewRect.z / 2.0, u_viewRect.w / 2.0);
    color += vec3(0.2) * grid(grid_st, u_zoom * (1.0 / u_content_scale) * major);
    color += vec3(0.1) * grid(grid_st, u_zoom * (1.0 / u_content_scale) * minor);

    color += vec3(1.0, 0.0, 0.0) * x_axis(st);
    color += vec3(0.0, 1.0, 0.0) * y_axis(st);

	gl_FragColor = vec4(color, 1.0);
}
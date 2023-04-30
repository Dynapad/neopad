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
    // I wrote this but I don't know how it works, sorry. - Dylan

    vec2 st = v_position.xy;
    vec3 color = vec3(0.0, 0.0, 0.0);
    vec2 offset = vec2(u_offset_x, u_offset_y);
    vec2 offset_st = (2.0 / u_viewRect.zw) * (offset / u_zoom * u_content_scale);

    float major = (1.0 / u_grid_major);
    float minor = (1.0 / u_grid_minor);

    vec2 grid_st = st * (u_viewRect.zw / 2.0) - (offset / u_zoom * u_content_scale);
    color += vec3(0.2, 0.2, 0.2) * grid(grid_st, major * u_zoom / u_content_scale);
    color += vec3(0.1, 0.1, 0.1) * grid(grid_st, minor * u_zoom / u_content_scale);

    color += vec3(1.0, 0.0, 0.0) * x_axis(st - offset_st);
    color += vec3(0.0, 1.0, 0.0) * y_axis(st - offset_st);

	gl_FragColor = vec4(color, 1.0);
}
$input v_color0

#include <bgfx_shader.sh>
#include "uniforms.sh"

// Single pixel line at x = x0 (or y = y0).
float hairline(float x, float x0, float width) {
    return step(x0 - width/2.0, x) - step(x0 + width/2.0, x);
}

// Grid with lines every tick world-pixels.
float grid(vec2 xy, float tick, float width) {
    float res = 1. / tick;
    vec2 grid = fract(xy * res);
    return max(0, 1. - (step(res, grid.x) * step(res, grid.y)));
}

#define gray333 vec3_splat(0.2)
#define gray111 vec3_splat(0.1)
#define red vec3(1.0, 0.0, 0.0)
#define green vec3(0.0, 1.0, 0.0)
#define blue vec3(0.0, 0.0, 1.0)

void main()
{
    // gl_FragCoord is in window-relative coordinates:
    //     x in [0, w], y in [0, h], z in [0, 1]
    //     (0, 0) is the bottom-left corner of the window.
    //     (0.5, 0.5) is the bottom-left pixel center.
    vec2 xy = gl_FragCoord.st;

    // Transform back to normalized device coordinates (matches quad geometry).
    // Our projection is orthographic, so this is equivalent to clip space.
    //     x in [-1, 1], y in [-1, 1], z in [-1, 1]
    //     (0, 0) is the center of the screen.
    //     (-1, -1) is the bottom left corner of the screen.
    vec2 xy_ndc = -1. + 2. * (xy / u_viewRect.zw);
    xy_ndc = vec2(1., -1.) * xy_ndc; // flip y-axis

    // Transform back to world coordinates using the inverse viewProj matrix.
    // This accounts for the camera's position and zoom.
    vec2 xy_world = mul(u_invViewProj, vec4(xy_ndc, 0.0, 1.0)).xy;

    vec3 color = vec3(0.0, 0.0, 0.0);

    // Approximately the world-pixel width of a single pixel at the current zoom level.
    float width = max(1.0, ceil(1.0 / u_zoom));

    // Single world-pixel major and minor grid lines.
    color += gray333 * grid(xy_world, u_grid_major, width);
    color += gray111 * grid(xy_world, u_grid_minor, width);

    // Single world-pixel axes lines.
    color += red   * hairline(xy_world.y, 0., width);
    color += green * hairline(xy_world.x, 0., width);

	gl_FragColor = vec4(color, 1.0);
}
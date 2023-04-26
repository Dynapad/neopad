#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION

#include <OpenGL/gl3.h>

#endif

#include <GLFW/glfw3.h>

#include "cglm/mat4.h"
#include "cglm/cglm.h"

#include <neopad/lib.h>

// May want to use GLAD / see: https://gen.glad.sh
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "common.h"
#include "shaders.h"


// XYZW coordinates for points of a unit quad (drawing as triangles).
float quad_coords[] = {-1.0f, 1.0f, 0.0f, 1.0f, // top left
                       -1.0f, -1.0f, 0.0f, 1.0f, // bottom left
                       1.0f, -1.0f, 0.0f, 1.0f, // bottom right
                       1.0f, -1.0f, 0.0f, 1.0f, // bottom right
                       1.0f, 1.0f, 0.0f, 1.0f,  // top right
                       -1.0f, 1.0f, 0.0f, 1.0f}; // top left

// RGBA colors for each point of the unit quad.
// top left: red
// top right: green
// bottom right: blue
// bottom left: white
float tex_data[] = {1.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    1.0f, 1.0f, 1.0f, 1.0f};

float tex_coords[] = {0.0f, 0.0f, // top left -> red
                      1.0f, 0.0f, // bottom left -> green
                      0.0f, 1.0f,
                      0.0f, 1.0f,
                      1.0f, 1.0f,
                      0.0f, 0.0f};

void error_callback(int error, const char *description) {
    eprintf("GLFW Error: %s\n", description);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

float zoom = 1;
void scroll_callback(GLFWwindow *window, double x_offset, double y_offset) {
    // If zoom is positive, zoom in. If zoom is negative, zoom out.
    printf("Scroll: %f\n", y_offset);
    // Limit zoom rate.
    zoom += glm_clamp((float)y_offset, -0.01f, 0.01f);
    // Clamp zoom between 0.1 and 1.
    zoom = glm_clamp(zoom, 0.1f, 1.0f);
}

int main() {
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        eprintf("Error: unable to initialize GLFW");
        exit(EXIT_FAILURE);
    }

    // Require OpenGL 3.3+
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // Require a forward-compatible core profile (needed for macOS)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(1280, 800, "Neopad Demo", NULL, NULL);
    if (!window) {
        eprintf("Error: unable to create window");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Obtain a context
    glfwMakeContextCurrent(window);

    // Print some debug information.
    printf("OpenGL:   %s\n", glGetString(GL_VERSION));
    printf("Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("GLSL:     %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("GLFW:     %s\n", glfwGetVersionString());

    // VAO: Vertex Array Object
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // VBO: Vertex Buffer Object
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(quad_coords),
            &quad_coords[0],
            GL_STATIC_DRAW
    );
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // TBO: Texture Buffer Object
    GLuint tbo;
    glGenBuffers(1, &tbo);
    glBindBuffer(GL_ARRAY_BUFFER, tbo);
    glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(tex_coords),
            &tex_coords[0],
            GL_STATIC_DRAW
    );
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Create a texture, and set texture parameters (not strictly necessary)
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Load the texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_FLOAT, tex_data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Compile and use the shader program.
    GLuint program = create_shader_program(vert_shader_src, frag_shader_src);
    glUseProgram(program);

    // Extract IDs of shader variables.
    GLint mvp_loc = glGetUniformLocation(program, "mvp");
    assert(mvp_loc >= 0);
    if (mvp_loc < 0) {
        eprintf("Error: unable to find mvp uniform\n");
        eprintf("OpenGL Error: %x\n", glGetError());
        exit(EXIT_FAILURE);
    }

    GLint tex_loc = glGetUniformLocation(program, "tex");
    if (tex_loc < 0) {
        eprintf("Error: unable to find texture uniform\n");
        eprintf("OpenGL Error: %x\n", glGetError());
        exit(EXIT_FAILURE);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(tex_loc, 0);

    // Set a background color (dark blue)
    glClearColor(0.0f, 0.0f, 0.4f, 1.0f);

    // Run!
    while (!glfwWindowShouldClose(window)) {
        // Update the viewport
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        // Update the projection matrix
        float aspect = (float) width / (float) height;

        // Update the model-view-projection matrix, applying the zoom level.

        mat4 proj_mat, model_view_mat, mvp_mat;
        glm_ortho_default(aspect, proj_mat);
        glm_mat4_identity(model_view_mat);
        glm_scale(model_view_mat, (vec3) {zoom, zoom, 1.0f});
        glm_mat4_mul(proj_mat, model_view_mat, mvp_mat);
        glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, (GLfloat *) mvp_mat);

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT);

        // Select geometry
        glBindVertexArray(vao);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        // Draw the geometry
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Unbind
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(0);
        glBindVertexArray(0);

        // Swap buffers
        glfwSwapBuffers(window);

        // Poll for events
        glfwPollEvents();
    }

    // Cleanup
    glDeleteTextures(1, &texture);
    glDeleteBuffers(1, &tbo);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
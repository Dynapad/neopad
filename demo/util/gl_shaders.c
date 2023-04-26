//
// Created by Dylan Lukes on 4/25/23.
//

#include <stdlib.h>
#include <stdio.h>
#include "GLFW/glfw3.h"
#include "neopad/lib.h"

#include "gl_shaders.h"
#include "../common.h"

const char *vert_shader_src =
        "#version 330 core                 \n"
        "layout(location = 0) in vec4 pos; \n"
        "layout(location = 1) in vec2 tex; \n"
        "smooth out vec2 uv;               \n"
        "uniform mat4 mvp;                 \n"
        "void main() {                     \n"
        "  gl_Position = mvp * pos;        \n"
        "  uv = tex;                       \n"
        "}";

const char *frag_shader_src =
        "#version 330 core               \n"
        "smooth in vec2 uv;              \n"
        "out vec3 color;                 \n"
        "uniform sampler2D tex;          \n"
        "void main() {                   \n"
        "  color = texture(tex, uv).rgb; \n"
        "}";


void check_shader(GLuint shader) {
    GLint res = GL_FALSE;
    GLint log_size = 0;

    // Check the shader.
    glGetShaderiv(shader, GL_COMPILE_STATUS, &res);
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
    if (log_size >= 1) {
        char *err_msg = (char *) malloc(log_size + 1);
        glGetShaderInfoLog(shader, log_size, NULL, err_msg);
        eprintf("Shader error: %s\n", err_msg);
        exit(EXIT_FAILURE);
    }
}

void check_program(GLuint program) {
    GLint res = GL_FALSE;
    GLint log_size = 0;

    // Check the program.
    glGetProgramiv(program, GL_LINK_STATUS, &res);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_size);
    if (log_size >= 1) {
        char *err_msg = (char *) malloc(log_size + 1);
        glGetProgramInfoLog(program, log_size, NULL, err_msg);
        eprintf("Program error: %s\n", err_msg);
        exit(EXIT_FAILURE);
    }
}

GLuint create_shader_program(const char *vert_src, const char *frag_src) {
    GLuint vs, fs, program;

    // Compile and check the vertex shader.
    vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vert_src, 0);
    glCompileShader(vs);
    check_shader(vs);

    // Compile and check the fragment shader.
    fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &frag_src, 0);
    glCompileShader(fs);
    check_shader(fs);

    // Link and check the program.
    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    check_program(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}
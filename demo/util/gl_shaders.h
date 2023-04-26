//
// Created by Dylan Lukes on 4/25/23.
//

#ifndef NEOPAD_DEMO_SHADERS_H
#define NEOPAD_DEMO_SHADERS_H

extern const char *vert_shader_src;
extern const char *frag_shader_src;

void check_shader(GLuint shader);
void check_program(GLuint program);
GLuint create_shader_program(const char *vert_src, const char *frag_src);

#endif //NEOPAD_DEMO_SHADERS_H

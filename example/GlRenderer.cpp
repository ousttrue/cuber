#include "GlRenderer.h"
// #include <DirectXMath.h>
#include <GL/glew.h>
#include <iostream>

static const struct {
  float x, y;
  float r, g, b;
} vertices[3] = {{-0.6f, -0.4f, 1.f, 0.f, 0.f},
                 {0.6f, -0.4f, 0.f, 1.f, 0.f},
                 {0.f, 0.6f, 0.f, 0.f, 1.f}};

static const char *vertex_shader_text = R"(#version 110
uniform mat4 MVP;
attribute vec3 vCol;
attribute vec2 vPos;
varying vec3 color;
void main()
{
    gl_Position = MVP * vec4(vPos, 0.0, 1.0);
    color = vCol;
}
)";

static const char *fragment_shader_text = R"(
#version 110
varying vec3 color;
void main()
{
    gl_FragColor = vec4(color, 1.0);
}
)";

GlRenderer::GlRenderer() {
  std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << std::endl;
  glewInit();
  std::cout << "GLEW_VERSION: " << glewGetString(GLEW_VERSION) << std::endl;

  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
  glCompileShader(vertex_shader);

  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
  glCompileShader(fragment_shader);

  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  int location = glGetUniformLocation(program, "MVP");
  if (location == -1) {
    throw std::runtime_error("glGetUniformLocation");
  }
  mvp_location = location;

  location = glGetAttribLocation(program, "vPos");
  if (location == -1) {
    throw std::runtime_error("glGetUniformLocation");
  }
  vpos_location = location;

  location = glGetAttribLocation(program, "vCol");
  if (location == -1) {
    throw std::runtime_error("glGetUniformLocation");
  }
  vcol_location = location;

  glEnableVertexAttribArray(vpos_location);
  glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                        sizeof(vertices[0]), (void *)0);
  glEnableVertexAttribArray(vcol_location);
  glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                        sizeof(vertices[0]), (void *)(sizeof(float) * 2));
}

GlRenderer::~GlRenderer() {}

void GlRenderer::RenderScene(RenderTime time, const CameraMatrix &camera) {
  // float ratio;
  // int width, height;

  // mat4x4_identity(m);
  // mat4x4_rotate_Z(m, m, (float)glfwGetTime());
  // mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
  // mat4x4_mul(mvp, p, m);

  // auto v = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4 *)view);
  // auto p = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4 *)projection);
  // auto vp = v * p;
  // DirectX::XMFLOAT4X4 mvp;
  // DirectX::XMStoreFloat4x4(&mvp, vp);

  glUseProgram(program);
  glUniformMatrix4fv(mvp_location, 1, GL_FALSE, camera.view);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

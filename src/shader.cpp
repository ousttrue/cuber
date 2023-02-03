#include "shader.h"
#include <GL/glew.h>

static uint32_t compile(const std::function<void(std::string_view)> &onError,
                        GLenum shaderType, std::span<std::string_view> srcs) {
  auto shader = glCreateShader(shaderType);

  std::vector<const GLchar *> string;
  std::vector<GLint> length;
  for (auto src : srcs) {
    string.push_back(src.data());
    length.push_back(src.size());
  }
  glShaderSource(shader, srcs.size(), string.data(), length.data());
  glCompileShader(shader);
  GLint isCompiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
  if (isCompiled == GL_FALSE) {
    GLint maxLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

    // The maxLength includes the NULL character
    std::vector<GLchar> errorLog(maxLength);
    glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

    // Provide the infolog in whatever manor you deem best.
    onError({errorLog.begin(), errorLog.end()});

    // Exit with failure.
    glDeleteShader(shader); // Don't leak the shader.
    return 0;
  }
  return shader;
}

namespace cuber {

ShaderProgram::ShaderProgram(uint32_t program) : program_(program) {}

ShaderProgram::~ShaderProgram() {}

std::shared_ptr<ShaderProgram>
ShaderProgram::Create(const std::function<void(std::string_view)> &onError,
                      std::span<std::string_view> vs_srcs,
                      std::span<std::string_view> fs_srcs) {

  auto vs = compile(onError, GL_VERTEX_SHADER, vs_srcs);

  return {};
}

} // namespace cuber

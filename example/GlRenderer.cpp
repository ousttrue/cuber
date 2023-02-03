#include "GlRenderer.h"
#include "Bvh.h"
#include <GL/glew.h>
#include <cuber.h>
#include <fstream>
#include <iostream>

template <typename T>
static std::vector<T> ReadAllBytes(const std::string &filename) {
  std::ifstream ifs(filename.c_str(), std::ios::binary | std::ios::ate);
  if (!ifs) {
    return {};
  }
  auto pos = ifs.tellg();
  auto size = pos / sizeof(T);
  if (pos % sizeof(T)) {
    ++size;
  }
  std::vector<T> buffer(size);
  ifs.seekg(0, std::ios::beg);
  ifs.read(buffer.data(), pos);
  return buffer;
}

struct GlRendererImpl {
  cuber::CubeRenderer cubes;

  GlRendererImpl() { cubes.Push(0.5f, {}); }
  ~GlRendererImpl() {}

  void Render(RenderTime time, const float projection[16],
              const float view[16]) {

    cubes.Render(projection, view);
  }
};

GlRenderer::GlRenderer() {
  std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << std::endl;
  glewInit();
  std::cout << "GLEW_VERSION: " << glewGetString(GLEW_VERSION) << std::endl;
  impl_ = new GlRendererImpl;
}

GlRenderer::~GlRenderer() { delete impl_; }

void GlRenderer::Load(std::string_view file) {

  auto bytes = ReadAllBytes<char>(std::string(file.begin(), file.end()));
  if (bytes.empty()) {
    return;
  }
  std::cout << "load: " << file << " " << bytes.size() << "bytes" << std::endl;
  Bvh bvh;
  if (!bvh.Parse({bytes.begin(), bytes.end()})) {
    return;
  }
  std::cout << bvh << std::endl;
}

void GlRenderer::RenderScene(RenderTime time, const float projection[16],
                             const float view[16]) {
  impl_->Render(time, projection, view);
}

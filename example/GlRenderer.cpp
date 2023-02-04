#include "GlRenderer.h"
#include "Bvh.h"
#include "BvhSolver.h"
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
  Bvh bvh;
  BvhSolver bvhSolver;

  GlRendererImpl() {}
  ~GlRendererImpl() {}

  std::optional<RenderTime> startTime_;

  void Render(RenderTime time, const float projection[16],
              const float view[16]) {
    if (!startTime_) {
      startTime_ = time;
    }
    RenderTime elapsed = time - *startTime_;
    auto index = bvh.TimeToIndex(elapsed);
    cubes.Render(projection, view, bvhSolver.GetFrame(index));
  }

  void Load(std::string_view file) {

    auto bytes = ReadAllBytes<char>(std::string(file.begin(), file.end()));
    if (bytes.empty()) {
      return;
    }
    std::cout << "load: " << file << " " << bytes.size() << "bytes"
              << std::endl;
    if (!bvh.Parse({bytes.begin(), bytes.end()})) {
      return;
    }
    std::cout << bvh << std::endl;

    // guess bvh scale
    float scalingFactor = 1.0f;
    if (bvh.max_height < 2) {
      // maybe meter scale. do nothing
    } else if (bvh.max_height < 200) {
      // maybe cm scale
      scalingFactor = 0.01f;
    }

    bvhSolver.Initialize();
    for (auto &joint : bvh.joints) {
      bvhSolver.PushJoint(joint);
    };
    int frameCount = bvh.FrameCount();
    for (int i = 0; i < frameCount; ++i) {
      auto frame = bvh.GetFrame(i);
      auto time = bvh.frame_time * i;
      bvhSolver.PushFrame(time, frame);
    }
  }
};

GlRenderer::GlRenderer() {
  std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << std::endl;
  glewInit();
  std::cout << "GLEW_VERSION: " << glewGetString(GLEW_VERSION) << std::endl;
  impl_ = new GlRendererImpl;
}

GlRenderer::~GlRenderer() { delete impl_; }

void GlRenderer::Load(std::string_view file) { impl_->Load(file); }

void GlRenderer::RenderScene(RenderTime time, const float projection[16],
                             const float view[16]) {
  impl_->Render(time, projection, view);
}

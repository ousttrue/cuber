#include "shader.h"

winrt::com_ptr<ID3DBlob> CompileShader(std::string_view src, const char *entry,
                                       const char *target) {
  UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
  flags |= D3DCOMPILE_DEBUG; // add more debug output
#endif
  winrt::com_ptr<ID3DBlob> vs_blob_ptr;
  winrt::com_ptr<ID3DBlob> error_blob;
  auto hr = D3DCompile(src.data(), src.size(), "shaders.hlsl", nullptr,
                       D3D_COMPILE_STANDARD_FILE_INCLUDE, entry, target, flags,
                       0, vs_blob_ptr.put(), error_blob.put());
  if (FAILED(hr)) {
    if (error_blob) {
      OutputDebugStringA((char *)error_blob->GetBufferPointer());
    }
    return {};
  }
  return vs_blob_ptr;
}

DXGI_FORMAT DxgiFormat(const cuber::VertexLayout &layout) {
  switch (layout.type) {
  case cuber::ValueType::Float:
    switch (layout.count) {
    case 2:
      return DXGI_FORMAT_R32G32_FLOAT;
    case 3:
      return DXGI_FORMAT_R32G32B32_FLOAT;
    case 4:
      return DXGI_FORMAT_R32G32B32A32_FLOAT;
    }
  }
  throw std::invalid_argument("not implemented");
}

#include "DxPlatform.h"
#include <Windows.h>
#include <d3d11.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <tchar.h>
#include <winrt/base.h>

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0 // From Windows SDK 8.1+ headers
#endif

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
                                                             UINT msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);

static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam,
                              LPARAM lParam);

struct DxPlatformImpl {
  WNDCLASSEXW wc = {sizeof(wc),
                    CS_CLASSDC,
                    &::WndProc,
                    0L,
                    0L,
                    GetModuleHandle(NULL),
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    L"ImGui Example",
                    NULL};

  HWND hwnd = {};
  winrt::com_ptr<ID3D11Device> device_;
  winrt::com_ptr<ID3D11DeviceContext> context_;
  winrt::com_ptr<IDXGISwapChain> swapchain_;
  winrt::com_ptr<ID3D11RenderTargetView> rtv_;
  winrt::com_ptr<ID3D11DepthStencilView> dsv_;

  DxPlatformImpl() {}

  ~DxPlatformImpl() {
    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();

    CleanupRenderTarget();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
  }

  void CleanupRenderTarget() {
    if (context_) {
      context_->OMSetRenderTargets(0, {}, {});
    }
    rtv_ = nullptr;
    dsv_ = nullptr;
  }

  HWND Create() {
    // Create application window
    // ImGui_ImplWin32_EnableDpiAwareness();
    ::RegisterClassExW(&wc);
    hwnd = ::CreateWindowW(wc.lpszClassName, L"CubeR DirectX11 Example",
                           WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL,
                           wc.hInstance, this);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd)) {
      return {};
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device_.get(), context_.get());

    return hwnd;
  }

  // Helper functions
  bool CreateDeviceD3D(HWND hWnd) {
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd{
        .BufferDesc =
            {
                .Width = 0,
                .Height = 0,
                .RefreshRate =
                    {
                        .Numerator = 60,
                        .Denominator = 1,
                    },
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
            },
        .SampleDesc =
            {
                .Count = 1,
                .Quality = 0,
            },
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount = 2,
        .OutputWindow = hWnd,
        .Windowed = TRUE,
        .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
        .Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH,
    };

    UINT createDeviceFlags = 0;
#if _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0,
    };
    HRESULT res = D3D11CreateDeviceAndSwapChain(
        NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags,
        featureLevelArray, 2, D3D11_SDK_VERSION, &sd, swapchain_.put(),
        device_.put(), &featureLevel, context_.put());
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software
                                       // driver if hardware is not available.
    {
      res = D3D11CreateDeviceAndSwapChain(
          NULL, D3D_DRIVER_TYPE_WARP, NULL, createDeviceFlags,
          featureLevelArray, 2, D3D11_SDK_VERSION, &sd, swapchain_.put(),
          device_.put(), &featureLevel, context_.put());
    }
    if (res != S_OK) {
      return false;
    }

    CreateRenderTarget();
    return true;
  }

  void CreateRenderTarget() {
    winrt::com_ptr<ID3D11Texture2D> pBackBuffer;
    swapchain_->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.put()));
    auto hr =
        device_->CreateRenderTargetView(pBackBuffer.get(), NULL, rtv_.put());
    assert(SUCCEEDED(hr));

    D3D11_TEXTURE2D_DESC backBufferDesc;
    pBackBuffer->GetDesc(&backBufferDesc);

    {
      winrt::com_ptr<ID3D11Texture2D> depth;
      D3D11_TEXTURE2D_DESC txDesc{
          .Width = backBufferDesc.Width,
          .Height = backBufferDesc.Height,
          .MipLevels = 1,
          .ArraySize = 1,
          .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
          .SampleDesc =
              {
                  .Count = 1,
                  .Quality = 0,
              },
          .Usage = D3D11_USAGE_DEFAULT,
          .BindFlags = D3D11_BIND_DEPTH_STENCIL,
          .CPUAccessFlags = 0,
          .MiscFlags = 0,
      };
      hr = device_->CreateTexture2D(&txDesc, NULL, depth.put());
      assert(SUCCEEDED(hr));

      D3D11_DEPTH_STENCIL_VIEW_DESC dsDesc{
          .Format = txDesc.Format,
          .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
          .Texture2D =
              {
                  .MipSlice = 0,
              },
      };
      hr = device_->CreateDepthStencilView(depth.get(), &dsDesc, dsv_.put());
      assert(SUCCEEDED(hr));
    }
  }

  // Win32 message handler
  // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell
  // if dear imgui wants to use your inputs.
  // - When io.WantCaptureMouse is true, do not dispatch mouse input data to
  // your main application, or clear/overwrite your copy of the mouse data.
  // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data
  // to your main application, or clear/overwrite your copy of the keyboard
  // data. Generally you may always pass all inputs to dear imgui, and hide them
  // from your application based on those two flags.
  LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
      return true;

    switch (msg) {
    case WM_SIZE:
      if (device_ != NULL && wParam != SIZE_MINIMIZED) {
        CleanupRenderTarget();
        swapchain_->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam),
                                  DXGI_FORMAT_UNKNOWN, 0);
        CreateRenderTarget();
      }
      return 0;
    case WM_SYSCOMMAND:
      if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
        return 0;
      break;
    case WM_DESTROY:
      ::PostQuitMessage(0);
      return 0;
    case WM_DPICHANGED:
      if (ImGui::GetIO().ConfigFlags &
          ImGuiConfigFlags_DpiEnableScaleViewports) {
        // const int dpi = HIWORD(wParam);
        // printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f *
        // 100.0f);
        const RECT *suggested_rect = (RECT *)lParam;
        ::SetWindowPos(hWnd, NULL, suggested_rect->left, suggested_rect->top,
                       suggested_rect->right - suggested_rect->left,
                       suggested_rect->bottom - suggested_rect->top,
                       SWP_NOZORDER | SWP_NOACTIVATE);
      }
      break;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
  }

  std::optional<std::chrono::milliseconds>
  NewFrame(const float clear_color[4]) {
    // Poll and handle messages (inputs, window resize, etc.)
    // See the WndProc() function below for our to dispatch events to the Win32
    // backend.
    MSG msg;
    while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
      if (msg.message == WM_QUIT) {
        return {};
      }
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();

    context_->ClearRenderTargetView(rtv_.get(), clear_color);
    context_->ClearDepthStencilView(
        dsv_.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // pipeline
    ID3D11RenderTargetView *rtv[] = {
        rtv_.get(),
    };
    context_->OMSetRenderTargets(1, rtv, dsv_.get());
    auto &io = ImGui::GetIO();
    D3D11_VIEWPORT viewport = {0.0f, 0.0f, io.DisplaySize.x, io.DisplaySize.y,
                               0.0f, 1.0f};
    context_->RSSetViewports(1, &viewport);

    return std::chrono::milliseconds(timeGetTime());
  }

  void EndFrame(ImDrawData *draw_data) {
    ImGui_ImplDX11_RenderDrawData(draw_data);
    swapchain_->Present(1, 0); // Present with vsync
    // g_pSwapChain->Present(0, 0); // Present without vsync
  }
};

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (msg == WM_CREATE) {
    auto p = reinterpret_cast<LPCREATESTRUCT>(lParam);
    SetWindowLongPtr(hWnd, GWLP_USERDATA,
                     reinterpret_cast<LONG_PTR>(p->lpCreateParams));
    return 0;
  }

  auto w =
      reinterpret_cast<DxPlatformImpl *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
  if (w) {
    return w->WndProc(hWnd, msg, wParam, lParam);
  } else {
    return DefWindowProcA(hWnd, msg, wParam, lParam);
  }
}

DxPlatform::DxPlatform() : impl_(new DxPlatformImpl) {}
DxPlatform::~DxPlatform() { delete impl_; }
bool DxPlatform::Create() { return impl_->Create() != nullptr; }
std::optional<std::chrono::milliseconds>
DxPlatform::NewFrame(const float clear_color[4]) {
  return impl_->NewFrame(clear_color);
}
void DxPlatform::EndFrame(ImDrawData *data) { impl_->EndFrame(data); }

winrt::com_ptr<struct ID3D11Device> DxPlatform::GetDevice() const {
  return impl_->device_;
}

#pragma once
#include <wincodec.h>
#include <dxgi1_3.h>
#include <d3d11_2.h>
#include <d2d1_2.h>
#include <d2d1_2helper.h>
#include <dcomp.h>
#include <wrl.h>
#include "./Gif.cpp"
using namespace Microsoft::WRL;
#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dcomp")
#pragma comment(lib, "ole32")

class Canvas {
private:
    HWND* window;
    ComPtr<ID3D11Device> d3dDevice;
    ComPtr<IDXGIDevice> dxgiDevice;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<IDXGIFactory2> dxFactory;
    ComPtr<IDXGISwapChain1> swapChain;
    ComPtr<ID2D1Factory2> d2Factory;
    ComPtr<ID2D1Device1> d2Device;

    ComPtr<IDXGISurface2> surface;
    ComPtr<ID2D1Bitmap1> bitmapTarget;
    ComPtr<IDCompositionDevice> dcompDevice;
    ComPtr<IDCompositionTarget> target;
    ComPtr<IDCompositionVisual> visual;
    ComPtr<ID2D1SolidColorBrush> brush;
public:
    ComPtr<ID2D1DeviceContext> d2dContext;
    Canvas(HWND* hWindow) {
        window = hWindow;
    }

    bool initializeCanvas() {
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1
        };

        HRESULT hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            creationFlags,
            featureLevels,
            ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,
            &d3dDevice,
            nullptr,
            &context
        );
        if (hr != S_OK) return false;


        hr = d3dDevice.As(&this->dxgiDevice);
        if (hr != S_OK) return false;

        hr = CreateDXGIFactory2(
            DXGI_CREATE_FACTORY_DEBUG,
            __uuidof(dxFactory),
            reinterpret_cast<void**>(dxFactory.GetAddressOf())
        );
        if (hr != S_OK) return false;

        //debuging
        DXGI_SWAP_CHAIN_DESC1 description = {};
        description.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        description.BufferCount = 2;
        description.SampleDesc.Count = 1;
        description.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;

        RECT rect = {};
        GetClientRect(*window, &rect);
        description.Width = rect.right - rect.left;
        description.Height = rect.bottom - rect.top;

        hr = dxFactory->CreateSwapChainForComposition(
            dxgiDevice.Get(),
            &description,
            nullptr,
            this->swapChain.GetAddressOf()
        );
        if (hr != S_OK) return false;

        // Create a single-threaded Direct2D factory with debugging information
        D2D1_FACTORY_OPTIONS const options = { D2D1_DEBUG_LEVEL_INFORMATION };
        hr = D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            options,
            d2Factory.GetAddressOf()
        );
        if (hr != S_OK) return false;

        // Create the Direct2D device that links back to the Direct3D device
        hr = d2Factory->CreateDevice(
            dxgiDevice.Get(),
            d2Device.GetAddressOf()
        );
        if (hr != S_OK) return false;

        // Create the Direct2D device context that is the actual render target
        // and exposes drawing commands
        hr = d2Device->CreateDeviceContext(
            D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
            this->d2dContext.GetAddressOf()
        );
        if (hr != S_OK) return false;

        // Retrieve the swap chain's back buffer
        hr = this->swapChain->GetBuffer(
            0,
            __uuidof(surface),
            reinterpret_cast<void**>(surface.GetAddressOf())
        );
        if (hr != S_OK) return false;

        // Create a Direct2D bitmap that points to the swap chain surface
        D2D1_BITMAP_PROPERTIES1 properties = {};
        properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
        properties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
        properties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET |
            D2D1_BITMAP_OPTIONS_CANNOT_DRAW;

        hr = this->d2dContext->CreateBitmapFromDxgiSurface(
            surface.Get(),
            properties,
            this->bitmapTarget.GetAddressOf()
        );
        if (hr != S_OK) return false;

        // Point the device context to the bitmap for rendering
        this->d2dContext->SetTarget(this->bitmapTarget.Get());

        hr = DCompositionCreateDevice(
            this->dxgiDevice.Get(),
            __uuidof(dcompDevice),
            reinterpret_cast<void**>(dcompDevice.GetAddressOf())
        );
        if (hr != S_OK) return false;

        hr = dcompDevice->CreateTargetForHwnd(
            *window,
            true,
            target.GetAddressOf()
        );
        if (hr != S_OK) return false;

        hr = dcompDevice->CreateVisual(visual.GetAddressOf());
        if (hr != S_OK) return false;

        hr = visual->SetContent(swapChain.Get());
        if (hr != S_OK) return false;

        hr = target->SetRoot(visual.Get());
        if (hr != S_OK) return false;

        hr = dcompDevice->Commit();

        return (hr == S_OK) ? true : false;
    }

    bool drawEntity() {
        this->d2dContext->BeginDraw();
        this->d2dContext->Clear();
        ComPtr<ID2D1SolidColorBrush> brush;
        D2D1_COLOR_F const brushColor = D2D1::ColorF(0.18f,  // red
            0.55f,  // green
            0.34f,  // blue
            0.75f); // alpha
        HRESULT hr = this->d2dContext->CreateSolidColorBrush(
            brushColor,
            brush.GetAddressOf()
        );
        if (hr != S_OK) return false;

        D2D1_POINT_2F const ellipseCenter = D2D1::Point2F(100.0f,  // x
            100.0f); // y
        D2D1_ELLIPSE const ellipse = D2D1::Ellipse(ellipseCenter,
            100.0f,  // x radius
            100.0f); // y radius

        d2dContext->FillEllipse(ellipse, brush.Get());

        hr = this->d2dContext->EndDraw();
        // Make the swap chain available to the composition engine
        hr = this->swapChain->Present(1, 0);
        return (hr != S_OK) ? true : false;
    }

    bool draw(ComPtr<ID2D1Bitmap> image) {
        if (image == nullptr) return false;
        d2dContext->BeginDraw();
        d2dContext->Clear();
        ComPtr<ID2D1SolidColorBrush> brush;
        D2D1_COLOR_F const brushColor = D2D1::ColorF(0.18f,  // red
            0.55f,  // green
            0.34f,  // blue
            0.75f); // alpha
        HRESULT hr = d2dContext->CreateSolidColorBrush(
            brushColor,
            brush.GetAddressOf()
        );
        if (hr != S_OK) return false;

        D2D1_SIZE_F rtSize = d2dContext->GetSize();
        D2D1_RECT_F rect = D2D1::Rect(
            0.0f,
            0.0f,
            rtSize.width,
            rtSize.height
        );

        d2dContext->DrawBitmap(
            image.Get(),
            rect,
            1.0F,
            D2D1_INTERPOLATION_MODE_LINEAR,
            nullptr
        );

        hr = d2dContext->EndDraw();
        hr = this->swapChain->Present(1, 0);
        return (hr != S_OK) ? true : false;
    }

    bool destroy() {
        d3dDevice->Release();
        dxgiDevice->Release();
        context->Release();
        dxFactory->Release();
        swapChain->Release();
        d2Factory->Release();
        d2Device->Release();
        d2dContext->Release();
        surface->Release();
        bitmapTarget->Release();
        dcompDevice->Release();
        target->Release();
        visual->Release();
        brush->Release();
        return true;
    }
};


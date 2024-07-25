#pragma once
#include <wincodec.h>
#include <d2d1_2.h>
#include <tchar.h>
#include <wrl.h>
#pragma comment(lib, "d2d1")
using namespace Microsoft::WRL;

class Gif {
private:
    ComPtr<IWICImagingFactory> wicFactory;
    ComPtr<IWICBitmapDecoder> decoder;

    const wchar_t* imgPath;
    UINT* frameCount = nullptr;
    UINT frameDelay = 0;

public:
    ComPtr<ID2D1Bitmap> frameBitmap;
    Gif(const wchar_t* path) {
        this->imgPath = path;
    }

    bool initilizeGif() {
        HRESULT hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&wicFactory)
        );
        if (hr != S_OK) return false;

        hr = wicFactory->CreateDecoderFromFilename(
            this->imgPath,
            nullptr,
            GENERIC_READ,
            WICDecodeMetadataCacheOnLoad,
            &decoder
        );
        if (hr != S_OK) return false;

        //total number of frames
        hr = decoder->GetFrameCount(frameCount);
        if (hr != S_OK) return false;

        if (frameCount == nullptr) {
            *frameCount = 1;
        }

        return hr == S_OK;
    }

    UINT getFrameCount() {
        return (frameCount != nullptr) ? *frameCount : 1;
    }

    UINT getFrameDelay() {
        return (frameDelay != NULL) ? frameDelay : 0;
    }

    ComPtr<ID2D1Bitmap> getBitmapFrameAt(
        int index,
        ComPtr<ID2D1DeviceContext> d2dContext
    ) {

        ComPtr<IWICFormatConverter> formatConverter = nullptr;
        ComPtr<IWICMetadataQueryReader> metadataReader = nullptr;
        ComPtr<IWICBitmapFrameDecode> frame = nullptr;

        HRESULT hr = decoder->GetFrame(index, &frame);
        if (hr != S_OK) return nullptr;

        hr = wicFactory->CreateFormatConverter(&formatConverter);
        if (hr != S_OK) return nullptr;

        hr = formatConverter->Initialize(
            frame.Get(),
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapDitherTypeNone,
            nullptr,
            0.0f,
            WICBitmapPaletteTypeCustom
        );
        if (hr != S_OK) return nullptr;

        hr = d2dContext->CreateBitmapFromWicBitmap(
            formatConverter.Get(),
            nullptr,
            &frameBitmap
        );
        if (hr != S_OK) return nullptr;

        hr = frame->GetMetadataQueryReader(&metadataReader);
        if (hr != S_OK) return nullptr;

        PROPVARIANT propValue;
        PropVariantInit(&propValue);

        hr = metadataReader->GetMetadataByName(
            L"/grctlext/Delay",
            &propValue
        );
        if (hr != S_OK || propValue.vt != VT_UI2) return nullptr;

        hr = UIntMult(propValue.uiVal, 10, &frameDelay);
        PropVariantClear(&propValue);
        if (hr != S_OK) return nullptr;

        formatConverter->Release();

        return (hr == S_OK) ? frameBitmap : nullptr;
    }

    bool erase() {
        wicFactory->Release();
        decoder->Release();
        free(&imgPath);
        return true;
    }
};


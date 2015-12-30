#pragma once

#include <string>
#include <wrl.h>

#define DIRECTX_CONSTANT_BUFFER_ALIGNMENT 16

struct ID3D11InputLayout;
struct ID3D11ComputeShader;
struct ID3D11Device;
struct ID3D11InputLayout;
struct ID3D11Buffer;

namespace Engine1
{
    class ComputeShader
    {
        public:

        bool                       isSame( const ComputeShader& shader ) const;
        //virtual ID3D11InputLayout& getInputLauout() const = 0;
        ID3D11ComputeShader&       getShader() const;
        bool                       isCompiled() const;

        protected:

        ComputeShader();
        virtual ~ComputeShader();

        static unsigned int compiledShadersCount;

        // Shader can be compiled only once and is assumed not to change over time.
        bool         compiled;
        unsigned int shaderId;

        Microsoft::WRL::ComPtr<ID3D11ComputeShader> shader;
        Microsoft::WRL::ComPtr<ID3D11Device>        device;
        Microsoft::WRL::ComPtr<ID3D11Buffer>        constantInputBuffer;

        // Copying is not allowed.
        ComputeShader( const ComputeShader& )          = delete;
        ComputeShader& operator=(const ComputeShader&) = delete;

    };
}
#pragma once

#include <string>
#include <wrl.h>

struct ID3D11InputLayout;
struct ID3D11PixelShader;
struct ID3D11Device;
struct ID3D11Buffer;

namespace Engine1
{
    class FragmentShader
    {

        public:

        bool isSame( const FragmentShader& shader ) const;
        ID3D11PixelShader& getShader() const;
        bool isCompiled() const;

        protected:

        FragmentShader();
        virtual ~FragmentShader();

        static unsigned int compiledShadersCount;

        // Shader can be compiled only once and is assumed not to change over time.
        bool compiled;
        unsigned int shaderId;

        Microsoft::WRL::ComPtr<ID3D11PixelShader> shader;
        Microsoft::WRL::ComPtr<ID3D11Device>      device;
        Microsoft::WRL::ComPtr<ID3D11Buffer>      constantInputBuffer;

        // Copying is not allowed.
        FragmentShader( const FragmentShader& ) = delete;
        FragmentShader& operator=(const FragmentShader&) = delete;
    };
}
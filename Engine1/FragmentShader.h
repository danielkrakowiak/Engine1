#pragma once

#include <memory>
#include <string>
#include <vector>
#include <wrl.h>

#define DIRECTX_CONSTANT_BUFFER_ALIGNMENT 16

struct ID3D11InputLayout;
struct ID3D11PixelShader;
struct ID3D11Device;
struct ID3D11Buffer;

namespace Engine1
{
    class FragmentShader
    {
        public:

        void loadAndInitialize( const std::string& path, Microsoft::WRL::ComPtr< ID3D11Device >& device );
        
        bool               isSame( const FragmentShader& shader ) const;
        ID3D11PixelShader& getShader() const;
        bool               isCompiled() const;

        protected:

        FragmentShader();
        virtual ~FragmentShader();
        
        void load( const std::string& path, Microsoft::WRL::ComPtr< ID3D11Device >& device );
        virtual void initialize( Microsoft::WRL::ComPtr< ID3D11Device >& device );

        virtual unsigned int getCompileFlags() const;

        static unsigned int compiledShadersCount;

        // Shader can be compiled only once and is assumed not to change over time.
        bool         m_compiled;
        unsigned int m_shaderId;

        Microsoft::WRL::ComPtr<ID3D11PixelShader> m_shader;
        Microsoft::WRL::ComPtr<ID3D11Device>      m_device;
        Microsoft::WRL::ComPtr<ID3D11Buffer>      m_constantInputBuffer;
        std::shared_ptr< std::vector< char > >    m_shaderBytecode;

        // Copying is not allowed.
        FragmentShader( const FragmentShader& )          = delete;
        FragmentShader& operator=(const FragmentShader&) = delete;
    };
}
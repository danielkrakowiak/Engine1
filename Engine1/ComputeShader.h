#pragma once

#include <memory>
#include <string>
#include <vector>
#include <wrl.h>

#define DIRECTX_CONSTANT_BUFFER_ALIGNMENT 16

struct ID3D11InputLayout;
struct ID3D11ComputeShader;
struct ID3D11Device3;
struct ID3D11InputLayout;
struct ID3D11Buffer;

namespace Engine1
{
    class ComputeShader
    {
        public:

        void loadAndInitialize( const std::string& path, Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        bool                       isSame( const ComputeShader& shader ) const;
        //virtual ID3D11InputLayout& getInputLauout() const = 0;
        ID3D11ComputeShader&       getShader() const;
        bool                       isCompiled() const;

        ComputeShader();
        virtual ~ComputeShader();

        protected:

        void load( const std::string& path, Microsoft::WRL::ComPtr< ID3D11Device3 >& device );
        virtual void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        virtual unsigned int getCompileFlags() const;

        static unsigned int compiledShadersCount;

        // Shader can be compiled only once and is assumed not to change over time.
        bool         m_compiled;
        unsigned int m_shaderId;

        Microsoft::WRL::ComPtr< ID3D11ComputeShader > m_shader;
        Microsoft::WRL::ComPtr< ID3D11Device3 >        m_device;
        Microsoft::WRL::ComPtr< ID3D11Buffer >        m_constantInputBuffer;
        std::shared_ptr< std::vector< char > >        m_shaderBytecode;

        // Copying is not allowed.
        ComputeShader( const ComputeShader& )          = delete;
        ComputeShader& operator=(const ComputeShader&) = delete;

    };
}
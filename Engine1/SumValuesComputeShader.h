#pragma once

#include "ComputeShader.h"

#include "Texture2D.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;

namespace Engine1
{
    template < typename PixelType >
    class SumValuesComputeShader : public ComputeShader
    {

        public:

        SumValuesComputeShader();
        virtual ~SumValuesComputeShader();

        void initialize();

        void setParameters( ID3D11DeviceContext3& deviceContext,
                            Texture2D< PixelType >& texture1,
                            Texture2D< PixelType >& texture2 );

        void setParameters( ID3D11DeviceContext3& deviceContext,
                            Texture2D< PixelType >& texture1,
                            Texture2D< PixelType >& texture2,
                            Texture2D< PixelType >& texture3 );

        void unsetParameters( ID3D11DeviceContext3& deviceContext );

        private:

        // Copying is not allowed.
        SumValuesComputeShader( const SumValuesComputeShader& ) = delete;
        SumValuesComputeShader& operator=( const SumValuesComputeShader& ) = delete;
    };

    template < typename PixelType >
    SumValuesComputeShader< PixelType >::SumValuesComputeShader() {}

    template < typename PixelType >
    SumValuesComputeShader< PixelType >::~SumValuesComputeShader() {}

    template < typename PixelType >
    void SumValuesComputeShader< PixelType >::initialize()
    {}

    template < typename PixelType >
    void SumValuesComputeShader< PixelType >::setParameters( ID3D11DeviceContext3& deviceContext,
                                               Texture2D< PixelType >& texture1,
                                               Texture2D< PixelType >& texture2 )
    {
        if ( !m_compiled )
            throw std::exception( "SumValueComputeShader::setParameters - Shader hasn't been compiled yet." );

        // Set input buffers and textures.
        const unsigned int resourceCount = 3;
        ID3D11ShaderResourceView* resources[ resourceCount ] = {
            texture1.getShaderResourceView(),
            texture2.getShaderResourceView(),
            nullptr
        };

        deviceContext.CSSetShaderResources( 0, resourceCount, resources );
    }

    template < typename PixelType >
    void SumValuesComputeShader< PixelType >::setParameters( ID3D11DeviceContext3& deviceContext,
                                               Texture2D< PixelType >& texture1,
                                               Texture2D< PixelType >& texture2,
                                               Texture2D< PixelType >& texture3 )
    {
        if ( !m_compiled )
            throw std::exception( "SumValueComputeShader::setParameters - Shader hasn't been compiled yet." );

        // Set input buffers and textures.
        const unsigned int resourceCount = 3;
        ID3D11ShaderResourceView* resources[ resourceCount ] = {
            texture1.getShaderResourceView(),
            texture2.getShaderResourceView(),
            texture3.getShaderResourceView()
        };

        deviceContext.CSSetShaderResources( 0, resourceCount, resources );
    }

    template < typename PixelType >
    void SumValuesComputeShader< PixelType >::unsetParameters( ID3D11DeviceContext3& deviceContext )
    {
        if ( !m_compiled )
            throw std::exception( "SumValueComputeShader::unsetParameters - Shader hasn't been compiled yet." );

        // Unset buffers and textures.
        ID3D11ShaderResourceView* nullResources[ 3 ] = { nullptr };
        deviceContext.CSSetShaderResources( 0, 3, nullResources );
    }
}








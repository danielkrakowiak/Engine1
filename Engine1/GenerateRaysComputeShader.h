#pragma once

#include "ComputeShader.h"

#include <string>

#include "float44.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class GenerateRaysComputeShader : public ComputeShader
    {

        public:

        GenerateRaysComputeShader();
        virtual ~GenerateRaysComputeShader();

        void compileFromFile( std::string path, ID3D11Device& device );

        private:

        // Copying is not allowed.
        GenerateRaysComputeShader( const GenerateRaysComputeShader& ) = delete;
        GenerateRaysComputeShader& operator=(const GenerateRaysComputeShader&) = delete;
    };
}


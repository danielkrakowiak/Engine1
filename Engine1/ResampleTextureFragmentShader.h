#pragma once

#include "FragmentShader.h"

#include "Texture2D.h"

#include "uchar4.h"
#include "float4.h"
#include "float2.h"

struct ID3D11SamplerState;

namespace Engine1
{
	class uchar4;

	class ResampleTextureFragmentShader : public FragmentShader
	{
	public:

		ResampleTextureFragmentShader();
		virtual ~ResampleTextureFragmentShader();

		void initialize(Microsoft::WRL::ComPtr< ID3D11Device >& device);
		void setParameters(ID3D11DeviceContext& deviceContext,
			Texture2DSpecBind< TexBind::ShaderResource, float4 >& texture,
			const int srcMipLevel);

		void unsetParameters(ID3D11DeviceContext& deviceContext);

	private:

		/*__declspec(align(DIRECTX_CONSTANT_BUFFER_ALIGNMENT))
			struct ConstantBuffer
		{
		};*/

		int m_resourceCount;

		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerStateLinearFilter;

		// Copying is not allowed.
		ResampleTextureFragmentShader(const ResampleTextureFragmentShader&) = delete;
		ResampleTextureFragmentShader& operator=(const ResampleTextureFragmentShader&) = delete;
	};
}




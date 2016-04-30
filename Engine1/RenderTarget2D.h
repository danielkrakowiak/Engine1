//#pragma once
//
//#include <wrl.h>
//
//#include "float4.h"
//
//struct ID3D11RenderTargetView;
//struct ID3D11DeviceContext;
//
//namespace Engine1
//{
//    class RenderTarget2D
//    {
//
//        public:
//
//        RenderTarget2D();
//        ~RenderTarget2D();
//
//        void initialize( ID3D11RenderTargetView& renderTarget );
//
//        virtual void clearOnGpu( float4 colorRGBA, ID3D11DeviceContext& deviceContext );
//
//        virtual ID3D11RenderTargetView* getRenderTarget();
//
//        protected:
//
//        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTarget;
//
//        private:
//
//        // Copying is not allowed.
//        RenderTarget2D( const RenderTarget2D& ) = delete;
//        RenderTarget2D& operator=(const RenderTarget2D&) = delete;
//    };
//}
//

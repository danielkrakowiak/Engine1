SamplerState g_samplerState  : register( s0 );

Texture2D<float> g_textureSrcMipmap : register( t0 );

cbuffer ConstantBuffer
{
    float2 srcPixelSizeInTexcoords;
    float2 pad1;
    float  srcMipmapLevel;
    float3 pad2;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
	float4 normal   : TEXCOORD0;
	float2 texCoord : TEXCOORD1;
};

//float main(PixelInputType input) : SV_Target
//{
//	const float4 values = g_textureSrcMipmap.Sample( g_samplerState, input.texCoord );
//    
//    // Return min of four pixels.
//    return min( values.w, min( values.z, min( values.x, values.y ) ) );
//}

float main(PixelInputType input) : SV_Target
{
    float minValue = 10000.f;

    const float texcoordShift = 1.0f;

    // #TODO: Smaller values should not spread equally to larger values...

    for ( float x = -srcPixelSizeInTexcoords.x; x <= srcPixelSizeInTexcoords.x; x += srcPixelSizeInTexcoords.x )
    {
        for ( float y = -srcPixelSizeInTexcoords.y; y <= srcPixelSizeInTexcoords.y; y += srcPixelSizeInTexcoords.y )
        {
            const float value = g_textureSrcMipmap.Sample( g_samplerState, input.texCoord + float2( x, y ) );

            // Note: This check is to avoid spreading very low values to far - hard shadows don't need to spread.
            //if ( value > srcMipmapLevel * srcMipmapLevel * 0.05f )
            minValue = min( minValue, value );
        }
    }

    return minValue;
}
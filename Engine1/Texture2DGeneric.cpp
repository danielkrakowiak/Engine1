#include "Texture2DGeneric.h"

using namespace Engine1;

//template<>
//inline unsigned char Texture2DGeneric< unsigned char >
//::sum( unsigned char val1, unsigned char val2, unsigned char val3, unsigned char val4 )
//{
//    // Note: Unsafe cast to smaller type.
//    return (unsigned char)( (int)val1 + (int)val2 + (int)val3 + (int)val4 );
//}
//
//template<>
//inline uchar4 Texture2DGeneric< uchar4 >
//::sum( uchar4 val1, uchar4 val2, uchar4 val3, uchar4 val4 )
//{
//    uint4 sum = (uint4)val1 + (uint4)val2 + (uint4)val3 + (uint4)val4;
//
//    // Note: Unsafe cast to smaller type.
//    return uchar4( (unsigned char)sum.x, (unsigned char)sum.y, (unsigned char)sum.z, (unsigned char)sum.w );
//}
//
//template<>
//inline float Texture2DGeneric< float >
//::sum( float val1, float val2, float val3, float val4 )
//{
//    return val1 + val2 + val3 + val4;
//}
//
//template<>
//inline float4 Texture2DGeneric< float4 >
//::sum( float4 val1, float4 val2, float4 val3, float4 val4 )
//{
//    return val1 + val2 + val3 + val4;
//}
//
////template<typename PixelType>
////inline PixelType Texture2DGeneric<PixelType>::sum( PixelType val1, PixelType val2, PixelType val3, PixelType val4 )
////{
////    return PixelType();
////}
#include "Asset.h"

#include <assert.h>

using namespace Engine1;

std::string Asset::toString( const Type type )
{
    switch ( type ) {
        case Type::BlockMesh:
            return "BlockMesh";
        case Type::SkeletonMesh:
            return "SkeletonMesh";
        case Type::Texture2D:
            return "Texture2D";
        case Type::BlockModel:
            return "BlockModel";
        case Type::SkeletonModel:
            return "SkeletonModel";
        case Type::SkeletonAnimation:
            return "SkeletonAnimation";
    }

    assert( false );

    return "";
}
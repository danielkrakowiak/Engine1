#pragma once

namespace Engine1
{
    class Actor
    {
        public:

        enum class Type
        {
            BlockActor = 0,
            SkeletonActor = 1
        };

        virtual Type getType() const = 0;

    };
}


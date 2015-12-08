#pragma once

namespace Engine1
{
    class Actor
    {
        public:

        enum class Type : char
        {
            BlockActor = 0,
            SkeletonActor = 1
        };

        virtual Type getType() const = 0;

    };
}


#pragma once

#include <memory>

namespace Engine1
{
    class float43;
    class Model;

    class Actor
    {
        public:

        enum class Type : char
        {
            BlockActor = 0,
            SkeletonActor = 1
        };

        virtual Type getType() const = 0;

        virtual const float43& getPose() const = 0;
        virtual       float43& getPose()       = 0;

        virtual std::shared_ptr< const Model > getBaseModel() const = 0;
        virtual std::shared_ptr< Model >       getBaseModel() = 0;

        virtual void setPose( const float43& pose ) = 0;

        void setCastingShadows( bool castShadows );
        bool isCastingShadows() const;

        protected:

        Actor();

        bool m_castsShadow;
    };
}


#pragma once

#include "PhysX/PxPhysicsAPI.h"

#include <malloc.h>
#include <assert.h>

namespace Engine1
{
    class PhysXAllocatorCallback : public physx::PxAllocatorCallback
    {
        public:

        // Memory has to be 16-byte aligned (guaranteed on x64 systems).
        void* allocate( size_t size, const char*, const char*, int ) override
        {
            void* ptr = malloc( size );

            // Make sure that memory is 16-byte aligned.
            assert( ( reinterpret_cast<size_t>( ptr ) & 15 ) == 0 );

            return ptr;
        }

        void deallocate( void* ptr ) override
        {
            free( ptr );
        }
    };

    class PhysXErrorCallback : public physx::PxErrorCallback
    {
        public:

        PhysXErrorCallback();
        ~PhysXErrorCallback();

        void reportError( physx::PxErrorCode::Enum code, const char* message, const char* file, int line ) override;
    };

    class PhysicsLibrary
    {
        public:

        PhysicsLibrary();
        ~PhysicsLibrary();

        PhysicsLibrary( PhysicsLibrary& ) = delete;
        void operator=( PhysicsLibrary& ) = delete;

        private:

        PhysXAllocatorCallback m_allocatorCallback;
        PhysXErrorCallback     m_errorCallback;

        physx::PxFoundation* m_foundation;
        physx::PxPvd*        m_visualDebugger;
        physx::PxPhysics*    m_physics;

        physx::PxScene*    m_scene;
        physx::PxMaterial* m_defaultMaterial;
    };
}

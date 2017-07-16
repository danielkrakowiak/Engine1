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

        static bool initialize();

        // Note: Not called anywhere normally.
        // No need to call it until you need to free some memory.
        static void deinitialize();

        static physx::PxFoundation& getFoundation();
        static physx::PxPvd&        getVisualDebugger();
        static physx::PxPhysics&    getPhysics();
        static physx::PxScene&      getScene();
        static physx::PxMaterial&   getDefaultMaterial();


        PhysicsLibrary()                  = delete;
        ~PhysicsLibrary()                 = delete;
        PhysicsLibrary( PhysicsLibrary& ) = delete;
        void operator=( PhysicsLibrary& ) = delete;

        private:

        static PhysXAllocatorCallback s_allocatorCallback;
        static PhysXErrorCallback     s_errorCallback;

        static physx::PxFoundation* s_foundation;
        static physx::PxPvd*        s_visualDebugger;
        static physx::PxPhysics*    s_physics;

        static physx::PxScene*    s_scene;
        static physx::PxMaterial* s_defaultMaterial;

        static bool s_initialized;
    };
}

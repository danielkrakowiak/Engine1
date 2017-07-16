#include "PhysicsLibrary.h"

#include "Windows.h"

#include <thread>

#include "StringUtil.h"

using namespace Engine1;
using namespace physx;

PhysXAllocatorCallback PhysicsLibrary::s_allocatorCallback;
PhysXErrorCallback     PhysicsLibrary::s_errorCallback;

physx::PxFoundation* PhysicsLibrary::s_foundation      = nullptr;
physx::PxPvd*        PhysicsLibrary::s_visualDebugger  = nullptr;
physx::PxPhysics*    PhysicsLibrary::s_physics         = nullptr;
physx::PxScene*      PhysicsLibrary::s_scene           = nullptr;
physx::PxMaterial*   PhysicsLibrary::s_defaultMaterial = nullptr;

bool PhysicsLibrary::s_initialized = PhysicsLibrary::initialize();

bool PhysicsLibrary::initialize()
{
    s_foundation = PxCreateFoundation( PX_FOUNDATION_VERSION, s_allocatorCallback, s_errorCallback );
    assert( s_foundation );
    if ( !s_foundation )
        throw std::exception( "PhysicsLibrary::PhysicsLibrary: Failed to create PhysX foundation." );

    s_visualDebugger = PxCreatePvd( *s_foundation );
    assert( s_visualDebugger );
    if ( !s_visualDebugger )
        throw std::exception( "PhysicsLibrary::PhysicsLibrary: Failed to create PhysX visual debugger." );

    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate( "localhost", 5425, 10 );
    s_visualDebugger->connect( *transport, PxPvdInstrumentationFlag::eALL );

    const bool recordMemoryAllocations = false;

    physx::PxTolerancesScale scale;
    scale.length = 1.0f;    // Usual size of an object is 1m.
    scale.mass   = 1000.0f; // Weight of 1 cubic meter in kg.
    scale.speed  = 10.0f;   // 10 m/s - speed of a falling object after 1 sec.

    s_physics = PxCreatePhysics( 
        PX_PHYSICS_VERSION, *s_foundation, scale, recordMemoryAllocations, s_visualDebugger 
    );
    assert( s_physics );
    if ( !s_physics )
        throw std::exception( "PhysicsLibrary::PhysicsLibrary: Failed to create PhysX core physics object." );

    if ( !PxInitExtensions( *s_physics, s_visualDebugger ) )
    {
        assert( false );
        throw std::exception( "PhysicsLibrary::PhysicsLibrary: Failed to initialize PhysX extensions." );
    }

    auto* defaultCPUDispather = PxDefaultCpuDispatcherCreate( std::thread::hardware_concurrency() );

    PxSceneDesc sceneDesc( scale );
    sceneDesc.gravity         = PxVec3( 0.0f, -9.81f, 0.0f );
    sceneDesc.cpuDispatcher   = defaultCPUDispather;
    sceneDesc.filterShader    = PxDefaultSimulationFilterShader;

    s_scene = s_physics->createScene( sceneDesc );

    auto* pvdClient = s_scene->getScenePvdClient();
    if ( pvdClient ) {
        pvdClient->setScenePvdFlag( PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true );
        pvdClient->setScenePvdFlag( PxPvdSceneFlag::eTRANSMIT_CONTACTS, true );
        pvdClient->setScenePvdFlag( PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true );
    }

    s_defaultMaterial = s_physics->createMaterial( 0.5f, 0.5f, 0.6f );

    return true;
}

void PhysicsLibrary::deinitialize()
{
    if (s_physics)
    {
        s_physics->release();
        s_physics = nullptr;
    }

    if (s_visualDebugger)
    {
        auto* transport = s_visualDebugger->getTransport();

        s_visualDebugger->release();
        s_visualDebugger = nullptr;

        if (transport)
        {
            transport->release();
            transport = nullptr;
        }
    }

    if (s_foundation)
    {
        s_foundation->release();
        s_foundation = nullptr;
    }
}

physx::PxFoundation& PhysicsLibrary::getFoundation()
{
    if ( s_foundation )
        return *s_foundation;
    else
        throw std::exception( "PhysicsLibrary::getFoundation - Physics Library is not initialized (or is corrupted)." );
}

physx::PxPvd& PhysicsLibrary::getVisualDebugger()
{
    if ( s_visualDebugger )
        return *s_visualDebugger;
    else
        throw std::exception( "PhysicsLibrary::getVisualDebugger - Physics Library is not initialized (or is corrupted)." );
}

physx::PxPhysics& PhysicsLibrary::getPhysics()
{
    if ( s_physics )
        return *s_physics;
    else
        throw std::exception( "PhysicsLibrary::getPhysics - Physics Library is not initialized (or is corrupted)." );
}

physx::PxScene& PhysicsLibrary::getScene()
{
    if ( s_scene )
        return *s_scene;
    else
        throw std::exception( "PhysicsLibrary::getScene - Physics Library is not initialized (or is corrupted)." );
}

physx::PxMaterial& PhysicsLibrary::getDefaultMaterial()
{
    if ( s_defaultMaterial )
        return *s_defaultMaterial;
    else
        throw std::exception( "PhysicsLibrary::getDefaultMaterial - Physics Library is not initialized (or is corrupted)." );
}

PhysXErrorCallback::PhysXErrorCallback()
{}

PhysXErrorCallback::~PhysXErrorCallback()
{}

void PhysXErrorCallback::reportError( PxErrorCode::Enum code, const char* message, const char* file, int line )
{
    std::string errorCode;

    switch ( code ) {
        case PxErrorCode::eNO_ERROR:
            errorCode = "no error";
            break;
        case PxErrorCode::eINVALID_PARAMETER:
            errorCode = "invalid parameter";
            break;
        case PxErrorCode::eINVALID_OPERATION:
            errorCode = "invalid operation";
            break;
        case PxErrorCode::eOUT_OF_MEMORY:
            errorCode = "out of memory";
            break;
        case PxErrorCode::eDEBUG_INFO:
            errorCode = "info";
            break;
        case PxErrorCode::eDEBUG_WARNING:
            errorCode = "warning";
            break;
        case PxErrorCode::ePERF_WARNING:
            errorCode = "performance warning";
            break;
        case PxErrorCode::eABORT:
            errorCode = "abort";
            break;
        case PxErrorCode::eINTERNAL_ERROR:
            errorCode = "internal error";
            break;
        case PxErrorCode::eMASK_ALL:
            errorCode = "unknown error";
            break;
    }

    OutputDebugStringW( 
        StringUtil::widen( 
            "[PhysX] " + 
            std::string( file ) + 
            std::string(" (") + 
            std::to_string(line) + "): " + 
            errorCode + " - " + 
            message + 
            std::string("\n") 
        ).c_str() 
    );

    assert( code != PxErrorCode::eABORT );
}

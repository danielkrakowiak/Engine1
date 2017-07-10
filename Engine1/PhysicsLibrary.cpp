#include "PhysicsLibrary.h"

#include "Windows.h"

#include "StringUtil.h"

using namespace Engine1;
using namespace physx;

PhysicsLibrary::PhysicsLibrary()
{
    m_foundation = PxCreateFoundation( PX_FOUNDATION_VERSION, m_allocatorCallback, m_errorCallback );
    assert( m_foundation );
    if ( !m_foundation )
        throw std::exception( "PhysicsLibrary::PhysicsLibrary: Failed to create PhysX foundation." );

    m_visualDebugger = PxCreatePvd( *m_foundation );
    assert( m_visualDebugger );
    if ( !m_visualDebugger )
        throw std::exception( "PhysicsLibrary::PhysicsLibrary: Failed to create PhysX visual debugger." );

    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate( "localhost", 5425, 10 );
    m_visualDebugger->connect( *transport, PxPvdInstrumentationFlag::eALL );

    const bool recordMemoryAllocations = false;

    physx::PxTolerancesScale scale;
    scale.length = 1.0f;    // Usual size of an object is 1m.
    scale.mass   = 1000.0f; // Weight of 1 cubic meter in kg.
    scale.speed  = 10.0f;   // 10 m/s - speed of a falling object after 1 sec.

    m_physics = PxCreatePhysics( 
        PX_PHYSICS_VERSION, *m_foundation, scale, recordMemoryAllocations, m_visualDebugger 
    );
    assert( m_physics );
    if ( !m_physics )
        throw std::exception( "PhysicsLibrary::PhysicsLibrary: Failed to create PhysX core physics object." );
}

PhysicsLibrary::~PhysicsLibrary()
{
    if (m_physics)
    {
        m_physics->release();
        m_physics = nullptr;
    }

    if (m_visualDebugger)
    {
        m_visualDebugger->release();
        m_visualDebugger = nullptr;
    }

    if (m_foundation)
    {
        m_foundation->release();
        m_foundation = nullptr;
    }
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

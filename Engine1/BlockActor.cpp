#include "BlockActor.h"

#include <algorithm>

#include "BlockModel.h"

#include "float44.h"

using namespace Engine1;
using namespace physx;

BlockActor::BlockActor( std::shared_ptr<BlockModel> model, const float43& pose, const bool createPhysics_ ) :
    m_model( model ),
    m_pose( pose )
{
    if ( createPhysics_ )
        createDynamicPhysics();
}

BlockActor::BlockActor( const BlockActor& other ) :
    m_model( other.m_model ),
    m_pose( other.m_pose )
{
    if ( other.m_physics )
    {
        createPhysics( *other.m_physics );
    }
}

BlockActor::~BlockActor()
{}

Actor::Type BlockActor::getType() const
{
    return Type::BlockActor;
}

const float43& BlockActor::getPose() const
{
    return m_pose;
}

float43& BlockActor::getPose( )
{
    if ( m_physics )
    {
        auto transform = m_physics->getGlobalPose();
        m_pose = float43( transform );
    }
        
    return m_pose;
}

std::shared_ptr<const BlockModel> BlockActor::getModel( ) const
{
    return m_model;
}

std::shared_ptr<BlockModel> BlockActor::getModel( )
{
    return m_model;
}

void BlockActor::setPose( const float43& pose )
{
    this->m_pose = pose;

    if ( m_physics )
    {
        float44 pose44( pose );
        PxTransform transform( pose44 );
        m_physics->setGlobalPose( transform );
    }
}

void BlockActor::setModel( std::shared_ptr<BlockModel> model )
{
    this->m_model = model;
}

bool BlockActor::hasPhysics() const
{
    return m_physics != nullptr;
}

void BlockActor::createDynamicPhysics()
{
    PxTransform transform( 
        m_pose.getTranslation(), 
        PxQuat( m_pose.getOrientation() ) 
    );

    float3 dimensions( 1.0f, 1.0f, 1.0f );
    if ( m_model && m_model->getMesh() )
        dimensions = m_model->getMesh()->getBoundingBox().getDimensions();

    const float minSize = 0.02f;
    dimensions.x = std::max( minSize, dimensions.x );
    dimensions.y = std::max( minSize, dimensions.y );
    dimensions.z = std::max( minSize, dimensions.z );

    const float maxDimension = std::max( dimensions.x, std::max( dimensions.y, dimensions.z ));

    auto* shape = PhysicsLibrary::getPhysics().createShape( 
        PxSphereGeometry( maxDimension / 2.0f ), 
        //PxBoxGeometry( dimensions / 2.0f ), 
        PhysicsLibrary::getDefaultMaterial() 
    );

    const float density = 1000.0f;

    PxRigidActor* physicsActor = PxCreateDynamic(
        PhysicsLibrary::getPhysics(),
        transform,
        *shape,
        density
    );

    PhysicsLibrary::getScene().addActor( *physicsActor );

    m_physics = std::unique_ptr< PxRigidActor, std::function< void( PxRigidActor* ) > >
        ( physicsActor, []( PxRigidActor* p ) { p->release(); } );
}

void BlockActor::createKinematicPhysics()
{
    PxTransform transform(
        m_pose.getTranslation(),
        PxQuat( m_pose.getOrientation() )
    );

    float3 dimensions( 1.0f, 1.0f, 1.0f );
    if ( m_model && m_model->getMesh() )
        dimensions = m_model->getMesh()->getBoundingBox().getDimensions();

    const float minSize = 0.02f;
    dimensions.x = std::max( minSize, dimensions.x );
    dimensions.y = std::max( minSize, dimensions.y );
    dimensions.z = std::max( minSize, dimensions.z );

    auto* shape = PhysicsLibrary::getPhysics().createShape(
        PxBoxGeometry( dimensions / 2.0f ),
        PhysicsLibrary::getDefaultMaterial()
    );

    const float density = 1000.0f;

    PxRigidActor* physicsActor = PxCreateKinematic(
        PhysicsLibrary::getPhysics(),
        transform,
        *shape,
        density
    );

    PhysicsLibrary::getScene().addActor( *physicsActor );

    m_physics = std::unique_ptr< PxRigidActor, std::function< void( PxRigidActor* ) > >
        ( physicsActor, []( PxRigidActor* p ) { p->release(); } );
}

void BlockActor::createPhysics( const PxRigidActor& otherPhysics )
{
    PxShape* otherShape = nullptr;

    PxU32 shapeCount = otherPhysics.getNbShapes();
    if (shapeCount == 1)
        otherPhysics.getShapes(&otherShape, 1, 0);

    auto* shape = PhysicsLibrary::getPhysics().createShape(
        otherShape->getGeometry().any(),
        PhysicsLibrary::getDefaultMaterial()
    );

    if (!shape)
        throw std::exception( "BlockActor::createPhysics - cannot copy physical actor because it doesn't have a shape." );

    const float density = 1000.0f;

    PxRigidActor* physicsActor = PxCreateDynamic(
        PhysicsLibrary::getPhysics(),
        otherPhysics.getGlobalPose(),
        *shape,
        density
    );

    PhysicsLibrary::getScene().addActor( *physicsActor );

    m_physics = std::unique_ptr< PxRigidActor, std::function< void( PxRigidActor* ) > >
        ( physicsActor, []( PxRigidActor* p ) { p->release(); } );
}

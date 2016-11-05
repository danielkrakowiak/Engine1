#include "Model.h"

#include <tuple>

using namespace Engine1;

std::string Model::textureTypeToString( Model::TextureType type )
{
    switch ( type )
    {
        case Model::TextureType::Alpha:           return "Alpha";
        case Model::TextureType::Emissive:        return "Emissive";
        case Model::TextureType::Albedo:          return "Albedo";
        case Model::TextureType::Metalness:       return "Metalness";
        case Model::TextureType::Roughness:       return "Roughness";
        case Model::TextureType::Normal:          return "Normal";
        case Model::TextureType::RefractiveIndex: return "RefractiveIndex";
    }

    return "-";
}

Model::Model( ) 
{}

Model::Model( const Model& obj ) :
m_alphaTextures( obj.m_alphaTextures ),
m_emissiveTextures( obj.m_emissiveTextures ),
m_albedoTextures( obj.m_albedoTextures ),
m_metalnessTextures( obj.m_metalnessTextures ),
m_roughnessTextures( obj.m_roughnessTextures ),
m_normalTextures( obj.m_normalTextures ),
m_refractiveIndexTextures( obj.m_refractiveIndexTextures )
{}

Model::~Model( ) 
{}

void Model::addTexture( const TextureType type, std::shared_ptr< Asset > texture, const int texcoordIndex, const float4 colorMultiplier )
{
    if ( type == Model::TextureType::Alpha || 
         type == Model::TextureType::Metalness || 
         type == Model::TextureType::Roughness || 
         type == Model::TextureType::RefractiveIndex )
    {
            const auto& typedTexture = std::dynamic_pointer_cast< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > >( texture );

            if ( !typedTexture )
                throw std::exception( "Model::addTexture - given texture has incorrect PixelType (should be unsiged char) or binding (should be TexBind::ShaderResource) to be used as requested type." );
    
            ModelTexture2D< unsigned char > modelTexture( typedTexture, texcoordIndex, colorMultiplier );

            if ( type == Model::TextureType::Alpha )
                m_alphaTextures.push_back( modelTexture );
            else if ( type == Model::TextureType::Metalness )
                m_metalnessTextures.push_back( modelTexture );
            else if ( type == Model::TextureType::Roughness )
                m_roughnessTextures.push_back( modelTexture );
            else if ( type == Model::TextureType::RefractiveIndex )
                m_refractiveIndexTextures.push_back( modelTexture );
    }
    else
    {
        const auto& typedTexture = std::dynamic_pointer_cast<Texture2DSpecBind< TexBind::ShaderResource, uchar4 >>( texture );

        if ( !typedTexture )
            throw std::exception( "Model::addTexture - given texture has incorrect PixelType (should be uchar4) or binding (should be TexBind::ShaderResource) to be used as requested type." );

        ModelTexture2D< uchar4 > modelTexture( typedTexture, texcoordIndex, colorMultiplier );

        if ( type == Model::TextureType::Emissive )
            m_emissiveTextures.push_back( modelTexture );
        else if ( type == Model::TextureType::Albedo )
            m_albedoTextures.push_back( modelTexture );
        if ( type == Model::TextureType::Normal )
            m_normalTextures.push_back( modelTexture );
    }
}

void Model::addAlphaTexture( ModelTexture2D< unsigned char >& texture )
{
	m_alphaTextures.push_back( texture );
}

void Model::addEmissiveTexture( ModelTexture2D< uchar4 >& texture )
{
	m_emissiveTextures.push_back( texture );
}

void Model::addAlbedoTexture( ModelTexture2D< uchar4 >& texture )
{
	m_albedoTextures.push_back( texture );
}

void Model::addMetalnessTexture( ModelTexture2D< unsigned char >& texture )
{
    m_metalnessTextures.push_back( texture );
}

void Model::addRoughnessTexture( ModelTexture2D< unsigned char >& texture )
{
	m_roughnessTextures.push_back( texture );
}

void Model::addNormalTexture( ModelTexture2D< uchar4 >& texture )
{
	m_normalTextures.push_back( texture );
}

void Model::addRefractiveIndexTexture( ModelTexture2D< unsigned char >& texture )
{
	m_refractiveIndexTextures.push_back( texture );
}

void Model::removeAllAlphaTextures()
{
    m_alphaTextures.clear();
}

void Model::removeAllEmissiveTextures()
{
    m_emissiveTextures.clear();
}

void Model::removeAllAlbedoTextures()
{
    m_albedoTextures.clear();
}

void Model::removeAllMetalnessTextures()
{
    m_metalnessTextures.clear();
}

void Model::removeAllRoughnessTextures()
{
    m_roughnessTextures.clear();
}

void Model::removeAllNormalTextures()
{
    m_normalTextures.clear();
}

void Model::removeAllRefractiveIndexTextures()
{
    m_refractiveIndexTextures.clear();
}

int Model::getTextureCount( const TextureType type ) const
{
    switch ( type ) {
        case TextureType::Alpha:
            return (int)m_alphaTextures.size();
        case TextureType::Emissive:
            return (int)m_emissiveTextures.size();
        case TextureType::Albedo:
            return (int)m_albedoTextures.size();
        case TextureType::Metalness:
            return (int)m_metalnessTextures.size();
        case TextureType::Roughness:
            return (int)m_roughnessTextures.size();
        case TextureType::Normal:
            return (int)m_normalTextures.size();
        case TextureType::RefractiveIndex:
            return (int)m_refractiveIndexTextures.size();
    }

    throw std::exception( "Model::getTextureCount - there is no such texture type." );
}

int Model::getAlphaTexturesCount( ) const {
	return (int)m_alphaTextures.size();
}

int Model::getEmissiveTexturesCount() const
{
    return (int)m_emissiveTextures.size();
}

int Model::getAlbedoTexturesCount() const
{
    return (int)m_albedoTextures.size();
}

int Model::getMetalnessTexturesCount() const
{
    return (int)m_metalnessTextures.size();
}

int Model::getRoughnessTexturesCount() const
{
    return (int)m_roughnessTextures.size();
}

int Model::getNormalTexturesCount() const
{
    return (int)m_normalTextures.size();
}

int Model::getRefractiveIndexTexturesCount() const
{
    return (int)m_refractiveIndexTextures.size();
}

std::vector< std::tuple< std::shared_ptr< Asset >, int > > Model::getTextures( const TextureType type ) const
{
    std::vector< std::tuple< std::shared_ptr< Asset >, int > > textures;

    switch ( type ) {
        case TextureType::Alpha:
            for ( const auto& modelTexture : m_alphaTextures )
                textures.push_back( std::make_tuple( modelTexture.getTexture(), modelTexture.getTexcoordIndex() ) );

            return textures;
        case TextureType::Emissive:
            for ( const auto& modelTexture : m_emissiveTextures )
                textures.push_back( std::make_tuple( modelTexture.getTexture(), modelTexture.getTexcoordIndex() ) );

            return textures;
        case TextureType::Albedo:
            for ( const auto& modelTexture : m_albedoTextures )
                textures.push_back( std::make_tuple( modelTexture.getTexture(), modelTexture.getTexcoordIndex() ) );

            return textures;
        case TextureType::Metalness:
            for ( const auto& modelTexture : m_metalnessTextures )
                textures.push_back( std::make_tuple( modelTexture.getTexture(), modelTexture.getTexcoordIndex() ) );

            return textures;
        case TextureType::Roughness:
            for ( const auto& modelTexture : m_roughnessTextures )
                textures.push_back( std::make_tuple( modelTexture.getTexture(), modelTexture.getTexcoordIndex() ) );

            return textures;
        case TextureType::Normal:
            for ( const auto& modelTexture : m_normalTextures )
                textures.push_back( std::make_tuple( modelTexture.getTexture(), modelTexture.getTexcoordIndex() ) );

            return textures;
        case TextureType::RefractiveIndex:
            for ( const auto& modelTexture : m_refractiveIndexTextures )
                textures.push_back( std::make_tuple( modelTexture.getTexture(), modelTexture.getTexcoordIndex() ) );

            return textures;
    }

    throw std::exception( "Model::getTextures - there is no such texture type." );
}

std::vector< ModelTexture2D< unsigned char > > Model::getAlphaTextures( ) const 
{
	return std::vector< ModelTexture2D< unsigned char > >( m_alphaTextures.begin( ), m_alphaTextures.end( ) );
}

std::vector< ModelTexture2D< uchar4 > > Model::getEmissiveTextures() const
{
    return std::vector< ModelTexture2D< uchar4 > >( m_emissiveTextures.begin(), m_emissiveTextures.end() );
}

std::vector< ModelTexture2D< uchar4 > > Model::getAlbedoTextures() const
{
    return std::vector< ModelTexture2D< uchar4 > >( m_albedoTextures.begin(), m_albedoTextures.end() );
}

std::vector< ModelTexture2D< unsigned char > > Model::getMetalnessTextures() const
{
    return std::vector< ModelTexture2D< unsigned char > >( m_metalnessTextures.begin(), m_metalnessTextures.end() );
}

std::vector< ModelTexture2D< unsigned char > > Model::getRoughnessTextures() const
{
    return std::vector< ModelTexture2D< unsigned char > >( m_roughnessTextures.begin(), m_roughnessTextures.end() );
}

std::vector< ModelTexture2D< uchar4 > > Model::getNormalTextures() const
{
    return std::vector< ModelTexture2D< uchar4 > >( m_normalTextures.begin(), m_normalTextures.end() );
}

std::vector< ModelTexture2D< unsigned char > > Model::getRefractiveIndexTextures() const
{
    return std::vector< ModelTexture2D< unsigned char > >( m_refractiveIndexTextures.begin(), m_refractiveIndexTextures.end() );
}

std::tuple< std::shared_ptr< Asset >, int > Model::getTexture( const TextureType type, int index ) const
{
    switch ( type ) 
    {
        case TextureType::Alpha: 
        {
            const auto& modelTexture = m_alphaTextures.at( index );
            return std::make_tuple( modelTexture.getTexture(), modelTexture.getTexcoordIndex() );
        }
        case TextureType::Emissive:
        {
            const auto& modelTexture = m_emissiveTextures.at( index );
            return std::make_tuple( modelTexture.getTexture(), modelTexture.getTexcoordIndex() );
        }
        case TextureType::Albedo:
        {
            const auto& modelTexture = m_albedoTextures.at( index );
            return std::make_tuple( modelTexture.getTexture(), modelTexture.getTexcoordIndex() );
        }
        case TextureType::Metalness:
        {
            const auto& modelTexture = m_metalnessTextures.at( index );
            return std::make_tuple( modelTexture.getTexture(), modelTexture.getTexcoordIndex() );
        }
        case TextureType::Roughness:
        {
            const auto& modelTexture = m_roughnessTextures.at( index );
            return std::make_tuple( modelTexture.getTexture(), modelTexture.getTexcoordIndex() );
        }
        case TextureType::Normal:
        {
            const auto& modelTexture = m_normalTextures.at( index );
            return std::make_tuple( modelTexture.getTexture(), modelTexture.getTexcoordIndex() );
        }
        case TextureType::RefractiveIndex:
        {
            const auto& modelTexture = m_refractiveIndexTextures.at( index );
            return std::make_tuple( modelTexture.getTexture(), modelTexture.getTexcoordIndex() );
        }
    }

    throw std::exception( "Model::getTexture - there is no such texture type." );
}

ModelTexture2D< unsigned char > Model::getAlphaTexture( int index ) const 
{
	if ( index >= (int)m_alphaTextures.size() ) throw std::exception( "Model::getAlphaTexture: Trying to access texture at non-existing index" );

	return m_alphaTextures.at( index );
}

ModelTexture2D< uchar4 > Model::getEmissiveTexture( int index ) const 
{
	if ( index >= (int)m_emissiveTextures.size() ) throw std::exception( "Model::getEmissionTexture: Trying to access texture at non-existing index" );

	return m_emissiveTextures.at( index );
}

ModelTexture2D< uchar4 > Model::getAlbedoTexture( int index ) const 
{
	if ( index >= (int)m_albedoTextures.size() ) throw std::exception( "Model::getAlbedoTexture: Trying to access texture at non-existing index" );

	return m_albedoTextures.at( index );
}

ModelTexture2D< unsigned char > Model::getMetalnessTexture( int index ) const 
{
	if ( index >= (int)m_metalnessTextures.size() ) throw std::exception( "Model::getMetalnessTexture: Trying to access texture at non-existing index" );

	return m_metalnessTextures.at( index );
}

ModelTexture2D< unsigned char > Model::getRoughnessTexture( int index ) const 
{
	if ( index >= (int)m_roughnessTextures.size( ) ) throw std::exception( "Model::getRoughnessTexture: Trying to access texture at non-existing index" );

	return m_roughnessTextures.at( index );
}

ModelTexture2D< uchar4 > Model::getNormalTexture( int index ) const 
{
	if ( index >= (int)m_normalTextures.size( ) ) throw std::exception( "Model::getNormalTexture: Trying to access texture at non-existing index" );

	return m_normalTextures.at( index );
}

ModelTexture2D< unsigned char > Model::getRefractiveIndexTexture( int index ) const 
{
	if ( index >= (int)m_refractiveIndexTextures.size( ) ) throw std::exception( "Model::getIndexOfRefractionTexture: Trying to access texture at non-existing index" );

	return m_refractiveIndexTextures.at( index );
}


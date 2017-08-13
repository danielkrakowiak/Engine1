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

Model::TextureType Model::textureFileNameToType( std::string fileName )
{
    auto suffixIndex    = fileName.rfind("_");
    auto extensionIndex = fileName.rfind(".");
    if ( suffixIndex != std::string::npos )
    {
        auto fileNameSuffix = fileName.substr( suffixIndex, extensionIndex - suffixIndex );

        return textureNameSuffixToType( fileNameSuffix );
    }

    throw std::exception( std::string("Model::textureFileNameToType - texture name has no suffix (" + fileName + ").").c_str() );
}

Model::TextureType Model::textureNameSuffixToType( std::string nameSuffix )
{
    if (nameSuffix.compare("_AL") == 0)       return Model::TextureType::Alpha;
    else if (nameSuffix.compare("_E") == 0)   return Model::TextureType::Emissive;
    else if (nameSuffix.compare("_A") == 0)   return Model::TextureType::Albedo;
    else if (nameSuffix.compare("_M") == 0)   return Model::TextureType::Metalness;
    else if (nameSuffix.compare("_R") == 0)   return Model::TextureType::Roughness;
    else if (nameSuffix.compare("_N") == 0)   return Model::TextureType::Normal;
    else if (nameSuffix.compare("_I") == 0)   return Model::TextureType::RefractiveIndex;

    throw std::exception( (std::string( "Model::textureNameSuffixToType - unrecognized texture name suffix (" ) + nameSuffix + ").").c_str() );
}

std::string Model::textureTypeToNameSuffix( TextureType type )
{
    switch ( type )
    {
        case Model::TextureType::Alpha:           return "_AL";
        case Model::TextureType::Emissive:        return "_E";
        case Model::TextureType::Albedo:          return "_A";
        case Model::TextureType::Metalness:       return "_M";
        case Model::TextureType::Roughness:       return "_R";
        case Model::TextureType::Normal:          return "_N";
        case Model::TextureType::RefractiveIndex: return "_I";
    }

    assert( false ); // Probably a new enum was added, but not added here.
    return "";
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

void Model::removeAllTextures( const TextureType type )
{
    switch ( type ) {
        case TextureType::Alpha:
            m_alphaTextures.clear();
            return;
        case TextureType::Emissive:
            m_emissiveTextures.clear();
            return;
        case TextureType::Albedo:
            m_albedoTextures.clear();
            return;
        case TextureType::Metalness:
            m_metalnessTextures.clear();
            return;
        case TextureType::Roughness:
            m_roughnessTextures.clear();
            return;
        case TextureType::Normal:
            m_normalTextures.clear();
            return;
        case TextureType::RefractiveIndex:
            m_refractiveIndexTextures.clear();
            return;
    }

    assert( false );
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

    assert( false );
    return 0;
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

    assert( false );
    return textures;
}

const std::vector< ModelTexture2D< unsigned char > >& Model::getAlphaTextures( ) const 
{
	return m_alphaTextures;
}

std::vector< ModelTexture2D< unsigned char > >& Model::getAlphaTextures()
{
    return m_alphaTextures;
}

const std::vector< ModelTexture2D< uchar4 > >& Model::getEmissiveTextures() const
{
    return m_emissiveTextures;
}

std::vector< ModelTexture2D< uchar4 > >& Model::getEmissiveTextures()
{
    return m_emissiveTextures;
}

const std::vector< ModelTexture2D< uchar4 > >& Model::getAlbedoTextures() const
{
    return m_albedoTextures;
}

std::vector< ModelTexture2D< uchar4 > >& Model::getAlbedoTextures()
{
    return m_albedoTextures;
}

const std::vector< ModelTexture2D< unsigned char > >& Model::getMetalnessTextures() const
{
    return m_metalnessTextures;
}

std::vector< ModelTexture2D< unsigned char > >& Model::getMetalnessTextures()
{
    return m_metalnessTextures;
}

const std::vector< ModelTexture2D< unsigned char > >& Model::getRoughnessTextures() const
{
    return m_roughnessTextures;
}

std::vector< ModelTexture2D< unsigned char > >& Model::getRoughnessTextures()
{
    return m_roughnessTextures;
}

const std::vector< ModelTexture2D< uchar4 > >& Model::getNormalTextures() const
{
    return m_normalTextures;
}

std::vector< ModelTexture2D< uchar4 > >& Model::getNormalTextures()
{
    return m_normalTextures;
}

const std::vector< ModelTexture2D< unsigned char > >& Model::getRefractiveIndexTextures() const
{
    return m_refractiveIndexTextures;
}

std::vector< ModelTexture2D< unsigned char > >& Model::getRefractiveIndexTextures()
{
    return m_refractiveIndexTextures;
}

std::shared_ptr< Asset > Model::getTexture( const TextureType type, int index ) const
{
    switch ( type ) 
    {
        case TextureType::Alpha: 
        {
            const auto& modelTexture = m_alphaTextures.at( index );
            return modelTexture.getTexture();
        }
        case TextureType::Emissive:
        {
            const auto& modelTexture = m_emissiveTextures.at( index );
            return modelTexture.getTexture();
        }
        case TextureType::Albedo:
        {
            const auto& modelTexture = m_albedoTextures.at( index );
            return modelTexture.getTexture();
        }
        case TextureType::Metalness:
        {
            const auto& modelTexture = m_metalnessTextures.at( index );
            return modelTexture.getTexture();
        }
        case TextureType::Roughness:
        {
            const auto& modelTexture = m_roughnessTextures.at( index );
            return modelTexture.getTexture();
        }
        case TextureType::Normal:
        {
            const auto& modelTexture = m_normalTextures.at( index );
            return modelTexture.getTexture();
        }
        case TextureType::RefractiveIndex:
        {
            const auto& modelTexture = m_refractiveIndexTextures.at( index );
            return modelTexture.getTexture();
        }
    }

    assert( false );
    return nullptr;
}

float4 Model::getTextureColorMultiplier( const TextureType type, int index ) const
{
    switch ( type ) 
    {
        case TextureType::Alpha: 
        {
            const auto& modelTexture = m_alphaTextures.at( index );
            return modelTexture.getColorMultiplier();
        }
        case TextureType::Emissive:
        {
            const auto& modelTexture = m_emissiveTextures.at( index );
            return modelTexture.getColorMultiplier();
        }
        case TextureType::Albedo:
        {
            const auto& modelTexture = m_albedoTextures.at( index );
            return modelTexture.getColorMultiplier();
        }
        case TextureType::Metalness:
        {
            const auto& modelTexture = m_metalnessTextures.at( index );
            return modelTexture.getColorMultiplier();
        }
        case TextureType::Roughness:
        {
            const auto& modelTexture = m_roughnessTextures.at( index );
            return modelTexture.getColorMultiplier();
        }
        case TextureType::Normal:
        {
            const auto& modelTexture = m_normalTextures.at( index );
            return modelTexture.getColorMultiplier();
        }
        case TextureType::RefractiveIndex:
        {
            const auto& modelTexture = m_refractiveIndexTextures.at( index );
            return modelTexture.getColorMultiplier();
        }
    }
}
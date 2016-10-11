#include "BlockModel.h"

#include <d3d11.h>

#include "BinaryFile.h"
#include "BlockModelParser.h"

using namespace Engine1;

std::shared_ptr<BlockModel> BlockModel::createFromFile( const BlockModelFileInfo& fileInfo, const bool loadRecurrently, ID3D11Device& device )
{
	return createFromFile( fileInfo.getPath(), fileInfo.getFormat(), loadRecurrently, device );
}

std::shared_ptr<BlockModel> BlockModel::createFromFile( const std::string& path, const BlockModelFileInfo::Format format, const bool loadRecurrently, ID3D11Device& device )
{
	std::shared_ptr< std::vector<char> > fileData = BinaryFile::load( path );

	std::shared_ptr<BlockModel> model = createFromMemory( fileData->cbegin(), format, loadRecurrently, device );

    model->getFileInfo( ).setPath( path );
    model->getFileInfo( ).setFormat( format );

    return model;
}

std::shared_ptr<BlockModel> BlockModel::createFromMemory( std::vector<char>::const_iterator dataIt, const BlockModelFileInfo::Format format, const bool loadRecurrently, ID3D11Device& device )
{
	if ( BlockModelFileInfo::Format::BLOCKMODEL == format ) {
		return BlockModelParser::parseBinary( dataIt, loadRecurrently, device );
	}

	throw std::exception( "BlockModel::createFromMemory() - incorrect 'format' argument." );
}

BlockModel::BlockModel( ) 
: m_mesh( nullptr ) 
{}

BlockModel::BlockModel( const BlockModel& obj ) :
m_fileInfo( obj.m_fileInfo ),
m_mesh( obj.m_mesh ),
m_alphaTextures( obj.m_alphaTextures ),
m_emissiveTextures( obj.m_emissiveTextures ),
m_albedoTextures( obj.m_albedoTextures ),
m_metalnessTextures( obj.m_metalnessTextures ),
m_roughnessTextures( obj.m_roughnessTextures ),
m_normalTextures( obj.m_normalTextures ),
m_refractiveIndexTextures( obj.m_refractiveIndexTextures )
{}

BlockModel::~BlockModel( ) 
{}

Asset::Type BlockModel::getType() const
{
	return Asset::Type::BlockModel;
}

std::vector< std::shared_ptr< const Asset > > BlockModel::getSubAssets( ) const
{
	std::vector< std::shared_ptr< const Asset > > subAssets;

	if ( m_mesh )
		subAssets.push_back( m_mesh );

    for ( const ModelTexture2D< unsigned char >& texture : m_alphaTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< uchar4 >& texture : m_emissiveTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< uchar4 >& texture : m_albedoTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : m_metalnessTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : m_roughnessTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< uchar4 >& texture : m_normalTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : m_refractiveIndexTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	return subAssets;
}

std::vector< std::shared_ptr< Asset > > BlockModel::getSubAssets()
{
	std::vector< std::shared_ptr<Asset> > subAssets;

	if ( m_mesh )
		subAssets.push_back( m_mesh );

    for ( const ModelTexture2D< unsigned char >& texture : m_alphaTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< uchar4 >& texture : m_emissiveTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< uchar4 >& texture : m_albedoTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : m_metalnessTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : m_roughnessTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< uchar4 >& texture : m_normalTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : m_refractiveIndexTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	return subAssets;
}

void BlockModel::swapSubAsset( std::shared_ptr<Asset> oldAsset, std::shared_ptr<Asset> newAsset )
{
	if ( !oldAsset || !newAsset )
		throw std::exception( "BlockModel::swapSubAsset - nullptr passed." );

    std::shared_ptr< BlockMesh > newMesh = std::dynamic_pointer_cast< BlockMesh >( newAsset );

    if ( newMesh )
    {
	    if ( m_mesh == oldAsset ) 
        {
		    m_mesh = newMesh;
		    return;
        }

        throw std::exception( "BlockModel::swapSubAsset - tried to swap assets of different types." );
    }

    typedef Texture2DSpecBind< TexBind::ShaderResource, unsigned char > TextureU1;
    std::shared_ptr< TextureU1 > newTextureU1 = std::dynamic_pointer_cast< TextureU1 >( newAsset );

    if ( newTextureU1 )
    {
        for ( ModelTexture2D< unsigned char >& texture : m_alphaTextures )
        {
		    if ( texture.getTexture() == oldAsset ) {
                texture.setTexture( newTextureU1 );
                return;
            }
        }

        for ( ModelTexture2D< unsigned char >& texture : m_metalnessTextures )
        {
		    if ( texture.getTexture() == oldAsset ) {
                texture.setTexture( newTextureU1 );
                return;
            }
        }

	    for ( ModelTexture2D< unsigned char >& texture : m_roughnessTextures )
        {
		    if ( texture.getTexture() == oldAsset ) {
                texture.setTexture( newTextureU1 );
                return;
            }
        }

        for ( ModelTexture2D< unsigned char >& texture : m_refractiveIndexTextures )
        {
		    if ( texture.getTexture() == oldAsset ) {
                texture.setTexture( newTextureU1 );
                return;
            }
        }

        throw std::exception( "BlockModel::swapSubAsset - tried to swap assets of different types." );
    }

    typedef Texture2DSpecBind< TexBind::ShaderResource, uchar4 > TextureU4;
    std::shared_ptr< TextureU4 > newTextureU4 = std::dynamic_pointer_cast< TextureU4 >( newAsset );

    if ( newTextureU4 )
    {
        for ( ModelTexture2D< uchar4 >& texture : m_emissiveTextures )
        {
		    if ( texture.getTexture() == oldAsset ) {
                texture.setTexture( newTextureU4 );
                return;
            }
        }

	    for ( ModelTexture2D< uchar4 >& texture : m_albedoTextures )
        {
		    if ( texture.getTexture() == oldAsset ) {
                texture.setTexture( newTextureU4 );
                return;
            }
        }

        for ( ModelTexture2D< uchar4 >& texture : m_normalTextures ) 
        {
		    if ( texture.getTexture() == oldAsset ) {
                texture.setTexture( newTextureU4 );
                return;
            }
        }

        throw std::exception( "BlockModel::swapSubAsset - tried to swap assets of different types." );
    }

    throw std::exception( "BlockModel::swapSubAsset - tried to swap assets of different types." );
}

void BlockModel::setFileInfo( const BlockModelFileInfo& fileInfo )
{
	this->m_fileInfo = fileInfo;
}

const BlockModelFileInfo& BlockModel::getFileInfo() const
{
	return m_fileInfo;
}

BlockModelFileInfo& BlockModel::getFileInfo()
{
	return m_fileInfo;
}

void BlockModel::saveToFile( const std::string& path ) const
{
	std::vector<char> data;

	BlockModelParser::writeBinary( data, *this );

	BinaryFile::save( path, data );
}

void BlockModel::saveToMemory( std::vector<char>& data ) const
{
    BlockModelParser::writeBinary( data, *this );
}

void BlockModel::loadCpuToGpu( ID3D11Device& device, ID3D11DeviceContext& deviceContext )
{
	if ( m_mesh )
		m_mesh->loadCpuToGpu( device );

    for ( const ModelTexture2D< unsigned char >& texture : m_alphaTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext );

    for ( const ModelTexture2D< uchar4 >& texture : m_emissiveTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext );

	for ( const ModelTexture2D< uchar4 >& texture : m_albedoTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext );

	for ( const ModelTexture2D< unsigned char >& texture : m_metalnessTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext );

	for ( const ModelTexture2D< unsigned char >& texture : m_roughnessTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext );

	for ( const ModelTexture2D< uchar4 >& texture : m_normalTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext );

	for ( const ModelTexture2D< unsigned char >& texture : m_refractiveIndexTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext );
}

void BlockModel::loadGpuToCpu()
{
    throw std::exception( "BlockMesh::loadGpuToCpu - unimplemented method." );
	// #TODO: implement.

	/*if ( mesh )
		mesh->loadGpuToCpu();

	std::vector<ModelTexture2D> textures = getAllTextures();
	for ( ModelTexture2D& texture : textures ) {
		if ( texture.getTexture() )
			texture.getTexture()->loadGpuToCpu();
	}*/
}

void BlockModel::unloadFromCpu()
{
	if ( m_mesh )
		m_mesh->unloadFromCpu();

    for ( const ModelTexture2D< unsigned char >& texture : m_alphaTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromCpu();

    for ( const ModelTexture2D< uchar4 >& texture : m_emissiveTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromCpu();

	for ( const ModelTexture2D< uchar4 >& texture : m_albedoTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromCpu();

	for ( const ModelTexture2D< unsigned char >& texture : m_metalnessTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromCpu();

	for ( const ModelTexture2D< unsigned char >& texture : m_roughnessTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromCpu();

	for ( const ModelTexture2D< uchar4 >& texture : m_normalTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromCpu();

	for ( const ModelTexture2D< unsigned char >& texture : m_refractiveIndexTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromCpu();
}

void BlockModel::unloadFromGpu()
{
	if ( m_mesh )
		m_mesh->unloadFromGpu();

    for ( const ModelTexture2D< unsigned char >& texture : m_alphaTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromGpu();

	for ( const ModelTexture2D< uchar4 >& texture : m_emissiveTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromGpu();

	for ( const ModelTexture2D< uchar4 >& texture : m_albedoTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromGpu();

	for ( const ModelTexture2D< unsigned char >& texture : m_metalnessTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromGpu();

	for ( const ModelTexture2D< unsigned char >& texture : m_roughnessTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromGpu();

	for ( const ModelTexture2D< uchar4 >& texture : m_normalTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromGpu();

	for ( const ModelTexture2D< unsigned char >& texture : m_refractiveIndexTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromGpu();
}

bool BlockModel::isInCpuMemory() const
{
	if ( !m_mesh || !m_mesh->isInCpuMemory() )
		return false;

    for ( const ModelTexture2D< unsigned char >& texture : m_alphaTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInCpuMemory() )
            return false;

    for ( const ModelTexture2D< uchar4 >& texture : m_emissiveTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInCpuMemory() )
            return false;

	for ( const ModelTexture2D< uchar4 >& texture : m_albedoTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInCpuMemory() )
            return false;

	for ( const ModelTexture2D< unsigned char >& texture : m_metalnessTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInCpuMemory() )
            return false;

	for ( const ModelTexture2D< unsigned char >& texture : m_roughnessTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInCpuMemory() )
            return false;

	for ( const ModelTexture2D< uchar4 >& texture : m_normalTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInCpuMemory() )
            return false;

	for ( const ModelTexture2D< unsigned char >& texture : m_refractiveIndexTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInCpuMemory() )
            return false;

	return true;
}

bool BlockModel::isInGpuMemory() const
{
	if ( !m_mesh || !m_mesh->isInGpuMemory() )
		return false;

    for ( const ModelTexture2D< unsigned char >& texture : m_alphaTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInGpuMemory() )
            return false;

    for ( const ModelTexture2D< uchar4 >& texture : m_emissiveTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInGpuMemory() )
            return false;

	for ( const ModelTexture2D< uchar4 >& texture : m_albedoTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInGpuMemory() )
            return false;

	for ( const ModelTexture2D< unsigned char >& texture : m_metalnessTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInGpuMemory() )
            return false;

	for ( const ModelTexture2D< unsigned char >& texture : m_roughnessTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInGpuMemory() )
            return false;

	for ( const ModelTexture2D< uchar4 >& texture : m_normalTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInGpuMemory() )
            return false;

	for ( const ModelTexture2D< unsigned char >& texture : m_refractiveIndexTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInGpuMemory() )
            return false;

	return true;
}

void BlockModel::setMesh( std::shared_ptr<BlockMesh> mesh ) {
	this->m_mesh = mesh;
}

std::shared_ptr<const BlockMesh> BlockModel::getMesh() const {
	return std::static_pointer_cast<const BlockMesh>( m_mesh );
}

std::shared_ptr<BlockMesh> BlockModel::getMesh( ) {
	return m_mesh;
}

void BlockModel::addAlphaTexture( ModelTexture2D< unsigned char >& texture )
{
	m_alphaTextures.push_back( texture );
}

void BlockModel::addEmissiveTexture( ModelTexture2D< uchar4 >& texture )
{
	m_emissiveTextures.push_back( texture );
}

void BlockModel::addAlbedoTexture( ModelTexture2D< uchar4 >& texture )
{
	m_albedoTextures.push_back( texture );
}

void BlockModel::addMetalnessTexture( ModelTexture2D< unsigned char >& texture )
{
    m_metalnessTextures.push_back( texture );
}

void BlockModel::addRoughnessTexture( ModelTexture2D< unsigned char >& texture )
{
	m_roughnessTextures.push_back( texture );
}

void BlockModel::addNormalTexture( ModelTexture2D< uchar4 >& texture )
{
	m_normalTextures.push_back( texture );
}

void BlockModel::addRefractiveIndexTexture( ModelTexture2D< unsigned char >& texture )
{
	m_refractiveIndexTextures.push_back( texture );
}

void BlockModel::removeAllAlphaTextures()
{
    m_alphaTextures.clear();
}

void BlockModel::removeAllEmissiveTextures()
{
    m_emissiveTextures.clear();
}

void BlockModel::removeAllAlbedoTextures()
{
    m_albedoTextures.clear();
}

void BlockModel::removeAllMetalnessTextures()
{
    m_metalnessTextures.clear();
}

void BlockModel::removeAllRoughnessTextures()
{
    m_roughnessTextures.clear();
}

void BlockModel::removeAllNormalTextures()
{
    m_normalTextures.clear();
}

void BlockModel::removeAllRefractiveIndexTextures()
{
    m_refractiveIndexTextures.clear();
}

int BlockModel::getAlphaTexturesCount( ) const {
	return (int)m_alphaTextures.size();
}

std::vector< ModelTexture2D< unsigned char > > BlockModel::getAlphaTextures( ) const {
	return std::vector< ModelTexture2D< unsigned char > >( m_alphaTextures.begin( ), m_alphaTextures.end( ) );
}

ModelTexture2D< unsigned char > BlockModel::getAlphaTexture( int index ) const {
	if ( index >= (int)m_alphaTextures.size() ) throw std::exception( "BlockModel::getAlphaTexture: Trying to access texture at non-existing index" );

	return m_alphaTextures.at( index );
}

int BlockModel::getEmissiveTexturesCount( ) const {
	return (int)m_emissiveTextures.size();
}

std::vector< ModelTexture2D< uchar4 > > BlockModel::getEmissiveTextures( ) const {
	return std::vector< ModelTexture2D< uchar4 > >( m_emissiveTextures.begin( ), m_emissiveTextures.end( ) );
}

ModelTexture2D< uchar4 > BlockModel::getEmissiveTexture( int index ) const {
	if ( index >= (int)m_emissiveTextures.size() ) throw std::exception( "BlockModel::getEmissionTexture: Trying to access texture at non-existing index" );

	return m_emissiveTextures.at( index );
}

int BlockModel::getAlbedoTexturesCount( ) const {
	return (int)m_albedoTextures.size();
}

std::vector< ModelTexture2D< uchar4 > > BlockModel::getAlbedoTextures( ) const {
	return std::vector< ModelTexture2D< uchar4 > >( m_albedoTextures.begin( ), m_albedoTextures.end( ) );
}

ModelTexture2D< uchar4 > BlockModel::getAlbedoTexture( int index ) const {
	if ( index >= (int)m_albedoTextures.size() ) throw std::exception( "BlockModel::getAlbedoTexture: Trying to access texture at non-existing index" );

	return m_albedoTextures.at( index );
}

int BlockModel::getMetalnessTexturesCount( ) const {
	return (int)m_metalnessTextures.size();
}

std::vector< ModelTexture2D< unsigned char > > BlockModel::getMetalnessTextures( ) const {
	return std::vector< ModelTexture2D< unsigned char > >( m_metalnessTextures.begin( ), m_metalnessTextures.end( ) );
}

ModelTexture2D< unsigned char > BlockModel::getMetalnessTexture( int index ) const {
	if ( index >= (int)m_metalnessTextures.size() ) throw std::exception( "BlockModel::getMetalnessTexture: Trying to access texture at non-existing index" );

	return m_metalnessTextures.at( index );
}

int BlockModel::getRoughnessTexturesCount( ) const {
	return (int)m_roughnessTextures.size();
}

std::vector< ModelTexture2D< unsigned char > > BlockModel::getRoughnessTextures( ) const {
	return std::vector< ModelTexture2D< unsigned char > >( m_roughnessTextures.begin( ), m_roughnessTextures.end( ) );
}

ModelTexture2D< unsigned char > BlockModel::getRoughnessTexture( int index ) const {
	if ( index >= (int)m_roughnessTextures.size( ) ) throw std::exception( "BlockModel::getRoughnessTexture: Trying to access texture at non-existing index" );

	return m_roughnessTextures.at( index );
}

int BlockModel::getNormalTexturesCount( ) const {
	return (int)m_normalTextures.size();
}

std::vector< ModelTexture2D< uchar4 > > BlockModel::getNormalTextures( ) const {
	return std::vector< ModelTexture2D< uchar4 > >( m_normalTextures.begin( ), m_normalTextures.end( ) );
}

ModelTexture2D< uchar4 > BlockModel::getNormalTexture( int index ) const {
	if ( index >= (int)m_normalTextures.size( ) ) throw std::exception( "BlockModel::getNormalTexture: Trying to access texture at non-existing index" );

	return m_normalTextures.at( index );
}

int BlockModel::getRefractiveIndexTexturesCount( ) const {
	return (int)m_refractiveIndexTextures.size();
}

std::vector< ModelTexture2D< unsigned char > > BlockModel::getRefractiveIndexTextures( ) const {
	return std::vector< ModelTexture2D< unsigned char > >( m_refractiveIndexTextures.begin( ), m_refractiveIndexTextures.end( ) );
}

ModelTexture2D< unsigned char > BlockModel::getRefractiveIndexTexture( int index ) const {
	if ( index >= (int)m_refractiveIndexTextures.size( ) ) throw std::exception( "BlockModel::getIndexOfRefractionTexture: Trying to access texture at non-existing index" );

	return m_refractiveIndexTextures.at( index );
}

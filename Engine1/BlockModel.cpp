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
: mesh( nullptr ) 
{}

BlockModel::BlockModel( const BlockModel& obj ) :
fileInfo( obj.fileInfo ),
mesh( obj.mesh ),
alphaTextures( obj.alphaTextures ),
emissionTextures( obj.emissionTextures ),
albedoTextures( obj.albedoTextures ),
metalnessTextures( obj.metalnessTextures ),
roughnessTextures( obj.roughnessTextures ),
normalTextures( obj.normalTextures ),
indexOfRefractionTextures( obj.indexOfRefractionTextures )
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

	if ( mesh )
		subAssets.push_back( mesh );

    for ( const ModelTexture2D< unsigned char >& texture : alphaTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< uchar4 >& texture : emissionTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< uchar4 >& texture : albedoTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : metalnessTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : roughnessTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< uchar4 >& texture : normalTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : indexOfRefractionTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	return subAssets;
}

std::vector< std::shared_ptr< Asset > > BlockModel::getSubAssets()
{
	std::vector< std::shared_ptr<Asset> > subAssets;

	if ( mesh )
		subAssets.push_back( mesh );

    for ( const ModelTexture2D< unsigned char >& texture : alphaTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< uchar4 >& texture : emissionTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< uchar4 >& texture : albedoTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : metalnessTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : roughnessTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< uchar4 >& texture : normalTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : indexOfRefractionTextures )
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
	    if ( mesh == oldAsset ) 
        {
		    mesh = newMesh;
		    return;
        }

        throw std::exception( "BlockModel::swapSubAsset - tried to swap assets of different types." );
    }

    typedef Texture2DSpecBind< TexBind::ShaderResource, unsigned char > TextureU1;
    std::shared_ptr< TextureU1 > newTextureU1 = std::dynamic_pointer_cast< TextureU1 >( newAsset );

    if ( newTextureU1 )
    {
        for ( ModelTexture2D< unsigned char >& texture : alphaTextures )
        {
		    if ( texture.getTexture() == oldAsset ) {
                texture.setTexture( newTextureU1 );
                return;
            }
        }

        for ( ModelTexture2D< unsigned char >& texture : metalnessTextures )
        {
		    if ( texture.getTexture() == oldAsset ) {
                texture.setTexture( newTextureU1 );
                return;
            }
        }

	    for ( ModelTexture2D< unsigned char >& texture : roughnessTextures )
        {
		    if ( texture.getTexture() == oldAsset ) {
                texture.setTexture( newTextureU1 );
                return;
            }
        }

        for ( ModelTexture2D< unsigned char >& texture : indexOfRefractionTextures )
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
        for ( ModelTexture2D< uchar4 >& texture : emissionTextures )
        {
		    if ( texture.getTexture() == oldAsset ) {
                texture.setTexture( newTextureU4 );
                return;
            }
        }

	    for ( ModelTexture2D< uchar4 >& texture : albedoTextures )
        {
		    if ( texture.getTexture() == oldAsset ) {
                texture.setTexture( newTextureU4 );
                return;
            }
        }

        for ( ModelTexture2D< uchar4 >& texture : normalTextures ) 
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
	this->fileInfo = fileInfo;
}

const BlockModelFileInfo& BlockModel::getFileInfo() const
{
	return fileInfo;
}

BlockModelFileInfo& BlockModel::getFileInfo()
{
	return fileInfo;
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
	if ( mesh )
		mesh->loadCpuToGpu( device );

    for ( const ModelTexture2D< unsigned char >& texture : alphaTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext );

    for ( const ModelTexture2D< uchar4 >& texture : emissionTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext );

	for ( const ModelTexture2D< uchar4 >& texture : albedoTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext );

	for ( const ModelTexture2D< unsigned char >& texture : metalnessTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext );

	for ( const ModelTexture2D< unsigned char >& texture : roughnessTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext );

	for ( const ModelTexture2D< uchar4 >& texture : normalTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext );

	for ( const ModelTexture2D< unsigned char >& texture : indexOfRefractionTextures )
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
	if ( mesh )
		mesh->unloadFromCpu();

    for ( const ModelTexture2D< unsigned char >& texture : alphaTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromCpu();

    for ( const ModelTexture2D< uchar4 >& texture : emissionTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromCpu();

	for ( const ModelTexture2D< uchar4 >& texture : albedoTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromCpu();

	for ( const ModelTexture2D< unsigned char >& texture : metalnessTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromCpu();

	for ( const ModelTexture2D< unsigned char >& texture : roughnessTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromCpu();

	for ( const ModelTexture2D< uchar4 >& texture : normalTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromCpu();

	for ( const ModelTexture2D< unsigned char >& texture : indexOfRefractionTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromCpu();
}

void BlockModel::unloadFromGpu()
{
	if ( mesh )
		mesh->unloadFromGpu();

    for ( const ModelTexture2D< unsigned char >& texture : alphaTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromGpu();

	for ( const ModelTexture2D< uchar4 >& texture : emissionTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromGpu();

	for ( const ModelTexture2D< uchar4 >& texture : albedoTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromGpu();

	for ( const ModelTexture2D< unsigned char >& texture : metalnessTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromGpu();

	for ( const ModelTexture2D< unsigned char >& texture : roughnessTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromGpu();

	for ( const ModelTexture2D< uchar4 >& texture : normalTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromGpu();

	for ( const ModelTexture2D< unsigned char >& texture : indexOfRefractionTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromGpu();
}

bool BlockModel::isInCpuMemory() const
{
	if ( !mesh || !mesh->isInCpuMemory() )
		return false;

    for ( const ModelTexture2D< unsigned char >& texture : alphaTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInCpuMemory() )
            return false;

    for ( const ModelTexture2D< uchar4 >& texture : emissionTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInCpuMemory() )
            return false;

	for ( const ModelTexture2D< uchar4 >& texture : albedoTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInCpuMemory() )
            return false;

	for ( const ModelTexture2D< unsigned char >& texture : metalnessTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInCpuMemory() )
            return false;

	for ( const ModelTexture2D< unsigned char >& texture : roughnessTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInCpuMemory() )
            return false;

	for ( const ModelTexture2D< uchar4 >& texture : normalTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInCpuMemory() )
            return false;

	for ( const ModelTexture2D< unsigned char >& texture : indexOfRefractionTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInCpuMemory() )
            return false;

	return true;
}

bool BlockModel::isInGpuMemory() const
{
	if ( !mesh || !mesh->isInGpuMemory() )
		return false;

    for ( const ModelTexture2D< unsigned char >& texture : alphaTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInGpuMemory() )
            return false;

    for ( const ModelTexture2D< uchar4 >& texture : emissionTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInGpuMemory() )
            return false;

	for ( const ModelTexture2D< uchar4 >& texture : albedoTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInGpuMemory() )
            return false;

	for ( const ModelTexture2D< unsigned char >& texture : metalnessTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInGpuMemory() )
            return false;

	for ( const ModelTexture2D< unsigned char >& texture : roughnessTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInGpuMemory() )
            return false;

	for ( const ModelTexture2D< uchar4 >& texture : normalTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInGpuMemory() )
            return false;

	for ( const ModelTexture2D< unsigned char >& texture : indexOfRefractionTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInGpuMemory() )
            return false;

	return true;
}

void BlockModel::setMesh( std::shared_ptr<BlockMesh> mesh ) {
	this->mesh = mesh;
}

std::shared_ptr<const BlockMesh> BlockModel::getMesh() const {
	return std::static_pointer_cast<const BlockMesh>( mesh );
}

std::shared_ptr<BlockMesh> BlockModel::getMesh( ) {
	return mesh;
}

void BlockModel::addAlphaTexture( ModelTexture2D< unsigned char >& texture )
{
	alphaTextures.push_back( texture );
}

void BlockModel::addEmissionTexture( ModelTexture2D< uchar4 >& texture )
{
	emissionTextures.push_back( texture );
}

void BlockModel::addAlbedoTexture( ModelTexture2D< uchar4 >& texture )
{
	albedoTextures.push_back( texture );
}

void BlockModel::addMetalnessTexture( ModelTexture2D< unsigned char >& texture )
{
    metalnessTextures.push_back( texture );
}

void BlockModel::addRoughnessTexture( ModelTexture2D< unsigned char >& texture )
{
	roughnessTextures.push_back( texture );
}

void BlockModel::addNormalTexture( ModelTexture2D< uchar4 >& texture )
{
	normalTextures.push_back( texture );
}

void BlockModel::addIndexOfRefractionTexture( ModelTexture2D< unsigned char >& texture )
{
	indexOfRefractionTextures.push_back( texture );
}

void BlockModel::removeAllAlphaTextures()
{
    alphaTextures.clear();
}

void BlockModel::removeAllEmissionTextures()
{
    emissionTextures.clear();
}

void BlockModel::removeAllAlbedoTextures()
{
    albedoTextures.clear();
}

void BlockModel::removeAllMetalnessTextures()
{
    metalnessTextures.clear();
}

void BlockModel::removeAllRoughnessTextures()
{
    roughnessTextures.clear();
}

void BlockModel::removeAllNormalTextures()
{
    normalTextures.clear();
}

void BlockModel::removeAllIndexOfRefractionTextures()
{
    indexOfRefractionTextures.clear();
}

int BlockModel::getAlphaTexturesCount( ) const {
	return (int)alphaTextures.size();
}

std::vector< ModelTexture2D< unsigned char > > BlockModel::getAlphaTextures( ) const {
	return std::vector< ModelTexture2D< unsigned char > >( alphaTextures.begin( ), alphaTextures.end( ) );
}

ModelTexture2D< unsigned char > BlockModel::getAlphaTexture( int index ) const {
	if ( index >= (int)alphaTextures.size() ) throw std::exception( "BlockModel::getAlphaTexture: Trying to access texture at non-existing index" );

	return alphaTextures.at( index );
}

int BlockModel::getEmissionTexturesCount( ) const {
	return (int)emissionTextures.size();
}

std::vector< ModelTexture2D< uchar4 > > BlockModel::getEmissionTextures( ) const {
	return std::vector< ModelTexture2D< uchar4 > >( emissionTextures.begin( ), emissionTextures.end( ) );
}

ModelTexture2D< uchar4 > BlockModel::getEmissionTexture( int index ) const {
	if ( index >= (int)emissionTextures.size() ) throw std::exception( "BlockModel::getEmissionTexture: Trying to access texture at non-existing index" );

	return emissionTextures.at( index );
}

int BlockModel::getAlbedoTexturesCount( ) const {
	return (int)albedoTextures.size();
}

std::vector< ModelTexture2D< uchar4 > > BlockModel::getAlbedoTextures( ) const {
	return std::vector< ModelTexture2D< uchar4 > >( albedoTextures.begin( ), albedoTextures.end( ) );
}

ModelTexture2D< uchar4 > BlockModel::getAlbedoTexture( int index ) const {
	if ( index >= (int)albedoTextures.size() ) throw std::exception( "BlockModel::getAlbedoTexture: Trying to access texture at non-existing index" );

	return albedoTextures.at( index );
}

int BlockModel::getMetalnessTexturesCount( ) const {
	return (int)metalnessTextures.size();
}

std::vector< ModelTexture2D< unsigned char > > BlockModel::getMetalnessTextures( ) const {
	return std::vector< ModelTexture2D< unsigned char > >( metalnessTextures.begin( ), metalnessTextures.end( ) );
}

ModelTexture2D< unsigned char > BlockModel::getMetalnessTexture( int index ) const {
	if ( index >= (int)metalnessTextures.size() ) throw std::exception( "BlockModel::getMetalnessTexture: Trying to access texture at non-existing index" );

	return metalnessTextures.at( index );
}

int BlockModel::getRoughnessTexturesCount( ) const {
	return (int)roughnessTextures.size();
}

std::vector< ModelTexture2D< unsigned char > > BlockModel::getRoughnessTextures( ) const {
	return std::vector< ModelTexture2D< unsigned char > >( roughnessTextures.begin( ), roughnessTextures.end( ) );
}

ModelTexture2D< unsigned char > BlockModel::getRoughnessTexture( int index ) const {
	if ( index >= (int)roughnessTextures.size( ) ) throw std::exception( "BlockModel::getRoughnessTexture: Trying to access texture at non-existing index" );

	return roughnessTextures.at( index );
}

int BlockModel::getNormalTexturesCount( ) const {
	return (int)normalTextures.size();
}

std::vector< ModelTexture2D< uchar4 > > BlockModel::getNormalTextures( ) const {
	return std::vector< ModelTexture2D< uchar4 > >( normalTextures.begin( ), normalTextures.end( ) );
}

ModelTexture2D< uchar4 > BlockModel::getNormalTexture( int index ) const {
	if ( index >= (int)normalTextures.size( ) ) throw std::exception( "BlockModel::getNormalTexture: Trying to access texture at non-existing index" );

	return normalTextures.at( index );
}

int BlockModel::getIndexOfRefractionTexturesCount( ) const {
	return (int)indexOfRefractionTextures.size();
}

std::vector< ModelTexture2D< unsigned char > > BlockModel::getIndexOfRefractionTextures( ) const {
	return std::vector< ModelTexture2D< unsigned char > >( indexOfRefractionTextures.begin( ), indexOfRefractionTextures.end( ) );
}

ModelTexture2D< unsigned char > BlockModel::getIndexOfRefractionTexture( int index ) const {
	if ( index >= (int)indexOfRefractionTextures.size( ) ) throw std::exception( "BlockModel::getIndexOfRefractionTexture: Trying to access texture at non-existing index" );

	return indexOfRefractionTextures.at( index );
}

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <wrl.h>

#include "FreeImagePlus.h"

#include "int2.h"

#include "BasicAsset.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;

class Texture2D {
	friend class Texture2DParser;

	public:

	enum class FileFormat : char 
	{
		BMP,
		DDS,
		JPEG,
		PNG,
		RAW,
		TIFF,
		TGA
	};

	static std::shared_ptr<Texture2D> createFromFile( const std::string& path, const FileFormat format );
	static std::shared_ptr<Texture2D> createFromMemory( std::vector<unsigned char>& fileData, const FileFormat format );
	
	static std::shared_ptr<Texture2D> createFromFileInfoBinary( std::vector<unsigned char>::const_iterator& dataIt, const bool load );

	public:

	void writeFileInfoBinary( std::vector<unsigned char>& data ) const;

	Texture2D();
	~Texture2D();

	virtual std::string getFilePath() const;
	virtual FileFormat getFileFormat() const;

	virtual void loadCpuToGpu( ID3D11Device& device );
	virtual void loadGpuToCpu( );
	virtual void unloadFromCpu( );
	virtual void unloadFromGpu( );
	virtual bool isInCpuMemory( ) const;
	virtual bool isInGpuMemory( ) const;

	virtual void generateMipMapsOnCpu( );
	virtual void generateMipMapsOnGpu( ID3D11DeviceContext& deviceContext );
	virtual void removeMipMapsOnCpu( );
	virtual bool hasMipMapsOnCpu( )      const;
	virtual bool hasMipMapsOnGpu( )     const;
	virtual int  getMipMapCountOnCpu( )  const;
	virtual int  getMipMapCountOnGpu( ) const;

	std::vector<unsigned char>&       getData( unsigned int mipMapLevel = 0 );
	const std::vector<unsigned char>& getData( unsigned int mipMapLevel = 0 ) const;

	virtual ID3D11Texture2D*          getTexture() const;
	virtual ID3D11ShaderResourceView* getShaderResource() const;

	virtual int getBytesPerPixel( ) const;
	virtual int getWidth( unsigned int mipMapLevel = 0 ) const;
	virtual int getHeight( unsigned int mipMapLevel = 0 ) const;
	virtual int getSize( unsigned int mipMapLevel = 0 ) const;
	virtual int getLineSize( unsigned int mipMapLevel = 0 ) const;

	protected:

	std::string filePath;
	FileFormat fileFormat;

	int bytesPerPixel;
	int width;
	int height;
	int size; // In bytes. Including padding at the end of each line (if present).
	bool mipmapsOnGpu;

	std::vector< std::vector<unsigned char> > dataMipMaps;

	Microsoft::WRL::ComPtr<ID3D11Texture2D>          texture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResource;

	int getMaxMipMapCount();

	// Copying textures in not allowed.
	Texture2D(const Texture2D& ) = delete;
	Texture2D& operator=( const Texture2D& ) = delete;
};


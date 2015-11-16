#pragma once

#include <string>
#include <vector>
#include <memory>

class FileInfo;

class Asset {

	public:

	enum class Type : char
	{
		BlockMesh         = 0,
		SkeletonMesh      = 1,
		Texture2D         = 2,
		BlockModel        = 3,
		SkeletonModel     = 4,
		SkeletonAnimation = 5
	};

	virtual Type                                        getType() const = 0;
	virtual const FileInfo&                             getFileInfo() const = 0;
	virtual FileInfo&                                   getFileInfo() = 0;
	virtual std::vector< std::shared_ptr<const Asset> > getSubAssets() const = 0;
	virtual std::vector< std::shared_ptr<Asset> >       getSubAssets() = 0;
	virtual void                                        swapSubAsset( std::shared_ptr<Asset> oldAsset, std::shared_ptr<Asset> newAsset ) = 0;
};


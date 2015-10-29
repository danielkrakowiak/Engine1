#pragma once

#include <string>

enum AssetType {
	BlockMeshAsset,
	RiggedMeshAsset,
	Texture2DAsset,
	SkeletonAnimationAsset
};

enum AssetFileFormat {
	NONE,
	BMP,
	DDS,
	JPEG,
	PNG,
	RAW,
	TIFF,
	TGA,
	OBJ,
	DAE,
	XAF
};

class BasicAsset {
public:
	virtual std::string getPath() = 0;
	virtual void loadFile( ) = 0;
	virtual void unloadFile( ) = 0;
	virtual void load( ) = 0;
	virtual void unload( ) = 0;
	virtual AssetType getAssetType( ) = 0;
};


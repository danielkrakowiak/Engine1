#pragma once

#include <string>
#include <memory>

#include "Asset.h"

class FileInfo
{
	public:

	enum class FileType : char
	{
		Textual = 0,
		Binary  = 1
	};

	virtual std::string               getPath() const = 0;
	virtual Asset::Type               getAssetType() const = 0;
	virtual FileType                  getFileType() const = 0;
	virtual std::shared_ptr<FileInfo> clone() const = 0;
};


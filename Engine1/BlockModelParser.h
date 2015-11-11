#pragma once

#include <string>
#include <vector>
#include <memory>

class BlockModel;

class BlockModelParser
{
	friend class BlockModel;

	private:
	static std::shared_ptr<BlockModel> parseBinary( const std::vector<char>& data, const bool loadRecurrently );
	static void                        writeBinary( std::vector<char>& data, const BlockModel& model );

	static std::string fileTypeIdentifier;
};


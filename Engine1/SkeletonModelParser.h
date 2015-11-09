#pragma once

#include <string>
#include <vector>
#include <memory>

class SkeletonModel;

class SkeletonModelParser
{
	friend class SkeletonModel;

	private:
	static std::shared_ptr<SkeletonModel> parseBinary( const std::vector<unsigned char>& data, const bool loadRecurrently );
	static void                           writeBinary( std::vector<unsigned char>& data, const SkeletonModel& model );

	static std::string fileTypeIdentifier;
};


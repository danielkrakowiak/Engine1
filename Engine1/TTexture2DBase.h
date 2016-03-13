#pragma once

namespace Engine1
{
    class Texture2DFileInfo;

    class TTexture2DBase
    {
        public:

        virtual void                     setFileInfo( const Texture2DFileInfo& fileInfo ) = 0;
        virtual const Texture2DFileInfo& getFileInfo() const = 0;
        virtual Texture2DFileInfo&       getFileInfo() = 0;

        virtual bool isInCpuMemory() const = 0;
        virtual bool isInGpuMemory() const = 0;

        virtual int getMipMapCountOnCpu()  const = 0;
        virtual int getMipMapCountOnGpu() const = 0;
        virtual int getBytesPerPixel() const = 0;
        virtual int getWidth( unsigned int mipMapLevel = 0 ) const = 0;
        virtual int getHeight( unsigned int mipMapLevel = 0 ) const = 0;
        virtual int getSize( unsigned int mipMapLevel = 0 ) const = 0;
        virtual int getLineSize( unsigned int mipMapLevel = 0 ) const = 0;

        protected:

        TTexture2DBase() {};
        ~TTexture2DBase() {};
    };
}


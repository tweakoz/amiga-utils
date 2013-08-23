#include "amigadrive.h"
#include "amigadumpfile.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

namespace amigadrive
{
	ADFIO::ADFIO()
	{
		;
	}

	void ADFIO::initDriver(UI *messenger, const char *devName, bool readOnly)
	{
		assert(messenger);
		assert(m_drvArch == DRV_32);

		m_messenger = messenger;

		if (readOnly)
		{
			m_filePtr = fopen(devName, "r");
			if (m_filePtr == nullptr)
			{
				throw Exception(m_messenger, "NativeDevice: unable to open readonly");
			}
		}
		else
		{
			m_filePtr = fopen(devName, "r+");
			if (m_filePtr == nullptr)
			{
				throw Exception(m_messenger, "NativeDevice: unable to open readwrite");
			}
		}
	}

	ADFIO::~ADFIO()
	{
		if (m_filePtr != nullptr)
		{
			fclose(m_filePtr);
			m_filePtr = nullptr;
		}
	}

	bool ADFIO::writeBlock(Block* writeBuffer, u64 blockNum)
	{
		s64 r;
		lseek(fileno(m_filePtr), BLOCKSIZE * blockNum, SEEK_SET);
		r = write(fileno(m_filePtr), writeBuffer, sizeof(Block));
		return (r == 512 ? true : false);
	}

	bool ADFIO::readBlock(Block* readBuffer, u64 blockNum)
	{
		s64 r;
		lseek(fileno(m_filePtr), BLOCKSIZE * blockNum, SEEK_SET);
		r = read(fileno(m_filePtr), readBuffer, sizeof(Block));
		return (r == 512 ? true : false);
	}
}

#ifndef AMIGADUMPFILE_H_INCLUDED
#define AMIGADUMPFILE_H_INCLUDED

#include <stdio.h>

namespace amigadrive
{
	/*!
	* 	This class implements basic file io for dealing with Amiga drive dump files.
	*/
	class ADFIO: public DeviceIO
	{
		protected:
			FILE *m_filePtr;

			/*!
			*	Initialises the driver. This function is called internally.
			*
			*	\param messenger - a UI object handling user alerts.
			*	\paran devName - the name of the dump file.
			*	\param readOnly - true if the dump file is to be opened read-only.
			*/
			virtual void initDriver(UI *messenger, const char *devName, bool readOnly);

			/*!
			* 	Writes a 512 byte block to the indicated zero-based block address.
			*/
			virtual bool writeBlock(Block* writeBuffer, u64 blockNum);

			/*!
			* 	Reads a 512 byte block from the indicated zero-based block address.
			*/
			virtual bool readBlock(Block* readBuffer, u64 blockNum);

		public:
			ADFIO();
			~ADFIO();

	};
}

#endif // AMIGADUMPFILE_H_INCLUDED

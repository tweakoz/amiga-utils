#ifndef AMIGADRIVE_H_INCLUDED
#define AMIGADRIVE_H_INCLUDED

#include "amigaui.h"
#include "exception.h"
#include "amigatypes.h"
#include "amigastruct.h"
#include "amigautils.h"

/*! \mainpage AmigaDrive - a library for working with Amiga devices and device images.
 *
 * \section intro_sec Introduction
 *
 * I starting writing this library on a rainy day in February of 2013. I'd been trying to
 * debug a broken install NetBSD on WinUAE, and I needed a way of accessing the
 *	drive partitions such that I could extract one and work on it under an Intel install
 *	of the same OS. I developed the code under CodeBlocks, my favourite
 *	Linux-based IDE to date. The host system was the 64-bit KDE variant of Ubuntu Precise
 *	Pangolin, but the code was written with an eye on portability and should
 *	port to any system with a reasonable C++ compiler.
 *
 *	The design of the library was inspired and influenced by Laurent Clevy's ADFLib -
 * 	initially I'd planned to use ADFLib but it lacked support for hard drive images. I felt that adding the
 *	necessary facilities would compromise its design, and it seemed like a good opportunity
 *	to learn a little more about the Linux C++ programming experience, so I decided to roll my own.
 *
 *	This is an early revision of the library - my ultimate goal is to provide a portable, versatile and
 *	powerful too for working with Amiga storage media.
  *
 */

#define BLOCKSIZE 512

namespace amigadrive
{
	typedef enum {DD_DISKETTE, HD_DISKETTE, HARD_DRIVE} DriveType;
	typedef enum {DRV_32, DRV_64} DriveArch;

	class Device;
	class Volume;
	class DeviceIO;

	/*!
	* 	A Volume class models an Amiga partition. The Device class keeps a list of Volumes, one per Amiga partition.
	*/
	class Volume
	{
		friend class Device;
		protected:

			DeviceIO *m_io;
			UI *m_messenger;
			bool m_ro;
			Volume *m_nextVol;
			Volume *m_prevVol;
			stringStore *m_strings;
			struct partitionBlock *m_partBlock;
			// struct amigaPartGeometry *m_partGeom;

		public:
			Volume(DeviceIO *io, UI *messenger, bool ro, struct rigidDiskBlock *rdb, u32 block);
			~Volume();

			/*!
			*	Returns a C string containing the name of the volume. Null on error.
			*/
			const char *volName(void);

			/*!
			*	Return the start block of the volume or -1 on error.
			*/
			s64 volStartBlock(void);

			/*!
			*	Return the volume's block count or -1 on error.
			*/
			s64 volBlockCount(void);

			/*!
			*	Return the number of bytes per block.
			*/
			s64 volBytesPerBlock(void);

			/*!
			*	Return the volume type
			*/
			char *volType(void);
	};

	/*!
	*	The DeviceIO class is an abstract base class which defines the interface for
	*	all input/out operations on a storage medium.
	*/
	class DeviceIO
	{
		friend class Device;
		friend class Volume;
		protected:
			DriveArch m_drvArch;
			u64 m_sectorCount;
			UI *m_messenger;

		public:
			DeviceIO() {;};
			virtual ~DeviceIO() {;};

		protected:
			/*!
			*	Initialises the IO driver with a user interface object, the name of the fire/device and whether
			*	it should be opened read-only.
			*/
			virtual void initDriver(UI *messenger, const char *devName, bool readOnly) = 0;

			/*!
			* Reads the given 512 byte sector in the sector buffer. Returns -1 on error, otherwise the number of bytes read.
			*
			* \param readBuffer - A pointer to a 512 byte array.
			* \param blockNumber - zero-based Block number to be read.
			*/
			virtual bool readBlock(Block *readBuffer, u64 blockNumber) = 0;

			/*!
			* Writes the given 512 byte sector to the device.
			*
			* \param writeBuffer - A pointer to a 512 byte array.
			* \param blockNumber - zero-based Block number to be written.
			*/
			virtual bool writeBlock(Block *writeBuffer, u64 blockNumber) = 0;
	};

	/*!
	*	The device class is intended to represent an Amiga persistent storage device
	*	such as a hard drive, zip drive, CD drive, or, quelle horreur, a
	* 	floppy disk drive.
	*
	*	For partitioned devices, it will maintain a list of volumes, one for each
	*	partition. For devices lacking a partition table, it will maintain a single
	*	volume.
	*/
	class Device
	{
		public:
		/*!
		*   Opens given device/file and read its configuration.
		*/
			Device(DeviceIO *io = nullptr, UI *messenger = nullptr, const char *devName = nullptr, bool readOnly = true);
			~Device();

			/*!
			* Returns the size of the drive in blocks.
			*/
			u64 blockCount(void);

			/*!
			* Returns the given 512 byte block in the supplied buffer.
			* Returns true on a successfull read, otherwise it returns false.
			*/
			bool readBlock(Block readBuffer, u64 blockNumber);

			/*!
			* Writes the given 512 byte sector to the device. Returns true on a successful write.
			*/
			bool writeBlock(Block writeBuffer, u64 blockNumber);

			/*!
			*	Returns the number of volumes.
			*/
			int volumeCount(void);

			/*!
			*	Pick a partition based on partition number. The first partition is partition number 1, and so on.
			*/
			Volume *volumeNumber(int partitionNumber);

			/*!
			* Returns a pointer to the first volume on the disk.
			*/
			Volume *getFirstVolume(void);

			/*!
			* Returns a pointer to the next volume on the disk.
			*/
			Volume *getNextVolume(void);

			/*!
			* Copy the given slice of device out to the named output file.
			*/
			bool blockCopyOut(const char *outFile, s64 start, s64 count);

			/*!
			* Copy the given input file starting at the given block
			* for an optional count of blocks. If no count is given,
			* copies the entire file.
			*/
			bool blockCopyIn(const char *outFile, s64 start, s64 count=-1);

			/*!
			*	Copies the given partition out to a named file.
			*	Partition numbers start at 1.
			*/
			bool partCopyOut(const char *outFile, int partition);

			/*!
			*	Copies the named partition file into a partition.
			* 	The file must be the exact size of the destionation
			*	partition.
			*
			*	Partition numbers start at 1.
			*/
			bool partCopyIn(const char *outFile, int partition);

			/*!
			*  Displays what we know about this disk or disk image.
			*/
			void About(void);

		protected:

		private:
			bool m_ro;
			DeviceIO *m_io;
			UI *m_messenger;
			Volume *m_volPtr;
			Volume *m_firstVol;
			DriveType m_drvType;
			stringStore *m_strings;

			struct rigidDiskBlock *m_rdb;
			struct bootcodeBlock *m_bootcode;

			struct rigidDiskBlock *getRDB(void);
			struct bootcodeBlock *getBootCode(void);
			void printPartAmiga(void);

			bool isWriteable(const char *filename);
			bool isReadable(const char *filename);
			bool isPresent(const char *filename);
	};
};
#endif // AMIGADRIVE_H_INCLUDED

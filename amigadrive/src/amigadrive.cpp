#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "amigadrive.h"
#include "amigastruct.h"
#include "endianness.h"

#define AMIGA_BLOCK_LIMIT 16
namespace amigadrive
{
	bool g_littleEndian;

	struct blockHeader
	{
		u32 id;
		u32 summedLongs;
		s32 chkSum;
	};

	/*
	 * Sum a block. The checksum of a block must end up at zero
	 * to be valid. The chk_sum field is selected so that adding
	 * it yields zero.
	 */
	int sumBlock(struct blockHeader *header)
	{
		u32 summedLongs = fe32(header->summedLongs);
		s32 *block = (s32 *)header;
		u32 i;
		s32 sum = 0;

		for (i = 0; i < summedLongs; i++)
		{
			sum += (s32)fe32(*block++);
		}

		return (sum != 0);
	}

	/*
	 * Search for the Rigid Disk Block. The rigid disk block is required
	 * to be within the first 16 blocks of a drive, needs to have
	 * the ID AMIGA_ID_RDISK ('RDSK') and needs to have a valid
	 * sum-to-zero checksum
	 */
	struct rigidDiskBlock *Device::getRDB(void)
	{
		struct rigidDiskBlock *rdb;
		Block blockBuffer;
		bool res;
		int i;

		rdb = new struct rigidDiskBlock;

		memset(rdb, 0, sizeof(struct rigidDiskBlock));

		for (i=0; i< AMIGA_BLOCK_LIMIT; i++)
		{
			res = m_io->readBlock(&blockBuffer, i);

			if (res)
			{
				struct rigidDiskBlock *trdb = (struct rigidDiskBlock *)blockBuffer;
				// m_messenger->textInfo("Checking %08x against %08x\n",fe32(trdb->id), AMIGA_ID_RDISK);
				if (fe32(trdb->id) == AMIGA_ID_RDISK)
				{
					// m_messenger->textInfo("Rigid disk block suspect at %d, checking checksum\n",i);
					if (sumBlock((struct blockHeader *)blockBuffer) == 0)
					{
						// m_messenger->textInfo("FOUND");
						memcpy(rdb, trdb, sizeof(struct rigidDiskBlock));
						return rdb;
					}
				}
			}
		}

		if (rdb)
			delete rdb;

		// m_messenger->textInfo("Done scanning, no RDB found");
		return nullptr;
	}

	struct bootcodeBlock *Device::getBootCode(void)
	{
		struct bootcodeBlock *bootcode;
		Block blockBuffer;
		bool res;
		int i;

		bootcode = new struct bootcodeBlock;
		memset(bootcode, 0, sizeof(struct bootcodeBlock));

		// m_messenger->textInfo("Scanning for BOOT from 0 to %d\n", AMIGA_BLOCK_LIMIT);
		for (i = 0; i < AMIGA_BLOCK_LIMIT; i++)
		{
			res = m_io->readBlock(&blockBuffer, i);
			if (res)
			{
				struct bootcodeBlock *boot = (struct bootcodeBlock *)&blockBuffer;
				if (fe32(boot->id) == AMIGA_ID_BOOT)
				{
					// m_messenger->textInfo("BOOT block at %d, checking checksum\n", i);
					if (sumBlock((struct blockHeader *)blockBuffer) == 0)
					{
						// m_messenger->textInfo("Found valid bootcode block\n");
						memcpy(bootcode, boot, sizeof(struct bootcodeBlock));
						return bootcode;
					}
				}
			}
		}

		if (bootcode)
			delete bootcode;

		// m_messenger->textInfo("No boot code found on disk\n");
		return nullptr;
	}

	/*
	 * Print a BCPL String. BCPL strings start with a byte with the length
	 * of the string, and don't contain a terminating null character
	 */
	static void bstrPrint(UI *msgr, char *string)
	{
		u8 len = *string++;
		char buffer[256];
		int i;

		i = 0;
		while (len)
		{
			buffer[i++] = *string++;
			len--;
		}

		buffer[i] = 0;
		msgr->textInfo("%-10s", buffer);
	}

	static void printDiskType(UI *msgr, u32 diskType)
	{
		char buffer[6];
		buffer[0] = (diskType & 0xFF000000)>>24;
		buffer[1] = (diskType & 0x00FF0000)>>16;
		buffer[2] = (diskType & 0x0000FF00)>>8;
		buffer[3] = '\\';
		buffer[4] = (diskType & 0x000000FF) + '0';
		buffer[5] = 0;
		msgr->textInfo("%s", buffer);
	}

	/*
	 * Print the info contained within the given partition block
	 */
	static void printPartInfo(UI* msgr, struct partitionBlock *p)
	{
		struct amigaPartGeometry *g;

		g = (struct amigaPartGeometry *)&(p->environment);

		bstrPrint(msgr, p->driveName);
		msgr->textInfo("%6d\t%6d\t",
			   fe32(g->lowCyl) * fe32(g->blockPerTrack) * fe32(g->surfaces) ,
			   (fe32(g->highCyl) - fe32(g->lowCyl) + 1) * fe32(g->blockPerTrack) * fe32(g->surfaces) - 1);
		printDiskType(msgr, fe32(g->dosType));
		msgr->textInfo("\t%5d\n", fe32(g->bootPriority));
	}


	void Device::printPartAmiga (void)
	{
		struct rigidDiskBlock *rdb = m_rdb;
		struct bootcodeBlock *boot;
		struct partitionBlock *p;
		Block blockBuffer;
		u32 block;
		int i = 1;

		if (!rdb)
		{
			m_messenger->textInfo("printPartAmiga: no rdb found\n");
			return;
		}

		m_messenger->textInfo("printPartAmiga: Scanning partition list\n");

		block = fe32(rdb->partitionList);
		m_messenger->textInfo("printPartAmiga: partition list at 0x%x\n", block);

		m_messenger->textInfo("Summary:  DiskBlockSize: %d\n"
			   "          Cylinders    : %d\n"
			   "          Sectors/Track: %d\n"
			   "          Heads        : %d\n\n",
			   fe32(rdb->blockBytes), fe32(rdb->cylinders), fe32(rdb->sectors),
			   fe32(rdb->heads));

		m_messenger->textInfo("                 First   Num. \n"
			   "Nr.  Part. Name  Block   Block  Type        Boot Priority\n");

		while (block != 0xFFFFFFFF)
		{
			bool res;

			m_messenger->textInfo("Trying to load block #0x%X\n", block);

			res = m_io->readBlock(&blockBuffer, block);
			if (res)
			{
				p = (struct partitionBlock *)blockBuffer;
				if (fe32(p->id) == AMIGA_ID_PART)
				{
					m_messenger->textInfo("PART block suspect at 0x%x, checking checksum\n",block);
					if (sumBlock((struct blockHeader *)p) == 0)
					{
						m_messenger->textInfo("%-4d ", i);
						i++;
						printPartInfo(m_messenger, p);
						block = fe32(p->next);
					}
				}
				else block = 0xFFFFFFFF;
			}
			else block = 0xFFFFFFFF;
		}

		boot = m_bootcode;
		if (boot)
		{
			m_messenger->textInfo("Disk is bootable\n");
		}
	}

	/*
	 * Copy a bcpl stribg to a c string
	 */
	static char *bcplStringCopy(stringStore *L, char *F)
	{
		u8 len = *F++;
		char *T;
		char *B;

		B = T = L->makeString(len+1);

		while (len)
		{
			*T++ = *F++;
			len--;
		}
		*T = 0;

		return B;
	}

	const char *Volume::volName(void)
	{
		if (m_partBlock)
			return bcplStringCopy(m_strings, m_partBlock->driveName);
		return nullptr;
	}

	s64 Volume::volStartBlock(void)
	{
		struct amigaPartGeometry *g = (struct amigaPartGeometry *)&(m_partBlock->environment);

		if (g)
			return fe32(g->lowCyl) * fe32(g->blockPerTrack) * fe32(g->surfaces);
		return -1;
	}

	s64 Volume::volBlockCount(void)
	{
		struct amigaPartGeometry *g = (struct amigaPartGeometry *)&(m_partBlock->environment);

		if (g)
			return (fe32(g->highCyl) - fe32(g->lowCyl) + 1) * fe32(g->blockPerTrack) * fe32(g->surfaces) - 1;
		return -1;
	}

	s64 Volume::volBytesPerBlock(void)
	{
		struct amigaPartGeometry *g = (struct amigaPartGeometry *)&(m_partBlock->environment);

		if (g)
			return g->sizeBlocks;
		return -1;
	}

	static char *strDiskType(stringStore *s, u32 diskType)
	{
		char *buffer = s->makeString(6);
		char *b = buffer;

		*b++ = (diskType & 0xFF000000)>>24;
		*b++ = (diskType & 0x00FF0000)>>16;
		*b++ = (diskType & 0x0000FF00)>>8;
		*b++ = '\\';
		*b++ = (diskType & 0x000000FF) + '0';
		*b = 0;

		return buffer;
	}

	char *Volume::volType(void)
	{
		struct amigaPartGeometry *g = (struct amigaPartGeometry *)&(m_partBlock->environment);

		if (g)
			return strDiskType(m_strings, fe32(g->dosType));
		return nullptr;
	}

	Volume::Volume(DeviceIO *io, UI *messenger, bool ro, struct rigidDiskBlock *rdb, u32 block)
	{
		struct partitionBlock *p;
		m_messenger = messenger;
		m_partBlock = nullptr;
		// m_partGeom = nullptr;
		m_nextVol = nullptr;
		Block blockBuffer;
		m_ro = ro;
		m_io = io;

		m_strings = new stringStore();

		if (block != 0xFFFFFFFF)
		{
			bool res;

			res = m_io->readBlock(&blockBuffer, block);
			if (res)
			{
				p = (struct partitionBlock *)blockBuffer;
				if (fe32(p->id) == AMIGA_ID_PART)
				{
					if (sumBlock((struct blockHeader *)p) == 0)
					{
						// m_messenger->textInfo("Creating a new volume structure to describe partition\n");

						m_partBlock = new struct partitionBlock;

						if (!m_partBlock)
							throw 0xFFFFFFFF;

						memcpy(m_partBlock, p, sizeof(struct partitionBlock));
						block = fe32(p->next);

						try
						{
							m_nextVol = new Volume(io, messenger, ro, rdb, block);
						}
						catch(u32 E)
						{
							if (m_nextVol)
							{
								delete m_nextVol;
								m_nextVol = nullptr;
							}
						}
						return;
					}
				}
			}
		}
		throw (u32)0xFFFFFFFF;
	}

	Volume::~Volume()
	{
		if (m_nextVol)
		{
			delete m_nextVol;
			m_nextVol = nullptr;
		}

		if (m_partBlock)
		{
			delete m_partBlock;
			m_partBlock = nullptr;
		}

		if (m_strings)
		{
			delete m_strings;
			m_strings = nullptr;
		}
	}

	Volume *Device::getFirstVolume(void)
	{
		m_volPtr = m_firstVol->m_nextVol;
		return m_firstVol;
	}

	Volume *Device::getNextVolume(void)
	{
		Volume *V = m_volPtr;
		m_volPtr = V->m_nextVol;
		return V;
	}

	int Device::volumeCount(void)
	{
		Volume *V;
		int I;

		if (m_rdb)
		{
			for (I=0, V=m_firstVol; V; V=V->m_nextVol)
				I++;
			return I;
		}
		return 0;
	}

	Volume *Device::volumeNumber(int partition)
	{
		Volume *V; int I;

		if (partition < 1 || partition > volumeCount())
		{
			char *s = m_strings->makeString(32);
			snprintf(s, 32, "Partition number should range between 1 and %d inclusive\n.", volumeCount());
			throw Exception(m_messenger, (const char *)s);
		}

		for (I = 1, V=m_firstVol; V; V=V->m_nextVol, I++)
			if (I == partition)
				return V;

		return nullptr;
	}

	Device::Device(DeviceIO *io, UI *messenger, const char *devName, bool readOnly)
	{
		g_littleEndian = isLittleEndian();

		m_strings = new stringStore();

		assert(messenger);
		assert(io);
		m_rdb = nullptr;
		m_bootcode = nullptr;
		m_firstVol = nullptr;
		m_messenger = messenger;
		m_io = io;
		m_ro = readOnly;
		m_io->initDriver(messenger, devName, readOnly);

		// look for a rigid disk block - returns null if not found
		m_rdb = getRDB();
		if (m_rdb)
		{
			u32 block;

			m_drvType = HARD_DRIVE;
			block = fe32(m_rdb->partitionList);
			try
			{
				m_firstVol = new Volume(m_io, m_messenger, m_ro, m_rdb, block);
			}
			catch(u32 E)
			{
				if (m_firstVol)
				{
					delete m_firstVol;
					m_firstVol = nullptr;
				}
			}
		}

		// look for bootcode - returns null if not found
		m_bootcode = getBootCode();

		// printPartAmiga();
	}

	Device::~Device()
	{
		m_io = nullptr;
		m_messenger = nullptr;
		if (m_bootcode)
		{
			delete m_bootcode;
			m_bootcode = nullptr;
		}

		if (m_rdb)
		{
			delete m_rdb;
			m_rdb = nullptr;
		}

		if (m_strings)
		{
			delete m_strings;
			m_strings = nullptr;
		}
	}

	bool Device::blockCopyOut(const char *outfile, s64 begin, s64 size)
	{
		s64 onepercent = size / 100;
		Block copyBuffer;
		s64 blocks = size;
		s64 i = begin;
		FILE *o;

		if (isPresent(outfile))
			if (!isWriteable(outfile))
				return false;

		o=fopen(outfile, "w");

		while (blocks--)
		{
			m_io->readBlock(&copyBuffer, i++);
			if (fwrite(&copyBuffer, sizeof(copyBuffer), 1, o) != 1)
			{
				fclose(o);
				return false;
			}
			m_messenger->progressBar((size - blocks) / onepercent);
		}
		fclose(o);
		return true;
	}

	bool Device::blockCopyIn(const char *infile, s64 begin, s64 size)
	{
		s64 onepercent = size / 100;
		Block copyBuffer;
		s64 blocks = size;
		s64 i = begin;
		FILE *in;

		if (isPresent(infile))
			if (!isReadable(infile))
				return false;

		in=fopen(infile, "r");

		while (blocks--)
		{
			if (fread(&copyBuffer, sizeof(copyBuffer), 1, in) != 1)
			{
				fclose(in);
				return false;
			}
			m_io->writeBlock(&copyBuffer, i++);
			m_messenger->progressBar((size - blocks) / onepercent);
		}
		fclose(in);
		return true;
	}

	bool Device::partCopyOut(const char *outfile, int partition)
	{
		s64 onepercent;
		Block copyBuffer;
		s64 blocks, size;
		Volume *V;
		s64 i;
		FILE *o;

		if (isPresent(outfile))
			if (!isWriteable(outfile))
			{
				m_messenger->textError("Can't write to [%s]\n", outfile);
				return false;
			}

		V = volumeNumber(partition);

		if (!V)
		{
			return false;
		}

		size = blocks = V->volBlockCount();
		i = V->volStartBlock();
		onepercent = blocks / 100;

		o=fopen(outfile, "w");

		while (blocks--)
		{
			m_io->readBlock(&copyBuffer, i++);
			if (fwrite(&copyBuffer, sizeof(copyBuffer), 1, o) != 1)
			{
				fclose(o);
				return false;
			}
			m_messenger->progressBar((size - blocks) / onepercent);
		}
		fclose(o);
		return true;
	}

	// Checks whether the given file exist/is writeable/is a regular file
	bool Device::isReadable(const char *fileName)
	{
		struct stat s;
		int rc, e;

		assert(fileName);

		if (*fileName == '\0')
		{
			m_messenger->textError("Filename was zero length\n");
		}

		rc = stat(fileName, &s);

		if (rc != 0)
		{
			e = errno;

			m_messenger->textError("Couldn't stat file [%s] - error return was %s\n", strerror(e));
			return false;
		}

		if (!S_ISREG(s.st_mode))
		{
			m_messenger->textError("[%s] isn't a regular file\n", fileName);
			return false;
		}

		if (!(S_IRUSR & s.st_mode))
		{
			m_messenger->textError("[%s] isn't readable\n", fileName);
			return false;
		}
		return true;
	}

	// Checks whether the given file exist/is writeable/is a regular file
	bool Device::isWriteable(const char *fileName)
	{
		struct stat s;
		int rc, e;

		assert(fileName);

		if (*fileName == '\0')
		{
			m_messenger->textError("Filename was zero length\n");
		}

		rc = stat(fileName, &s);

		if (rc != 0)
		{
			e = errno;

			m_messenger->textError("Couldn't stat file [%s] - error return was %s\n", strerror(e));
			return false;
		}

		if (!S_ISREG(s.st_mode))
		{
			m_messenger->textError("[%s] isn't a regular file\n", fileName);
			return false;
		}

		if (!(S_IWUSR & s.st_mode))
		{
			m_messenger->textError("[%s] isn't readable\n", fileName);
			return false;
		}
		return true;
	}

	// Checks whether the given file exist/is writeable/is a regular file
	bool Device::isPresent(const char *fileName)
	{
		struct stat s;
		int rc, e;

		assert(fileName);

		if (*fileName == '\0')
		{
			m_messenger->textError("Filename was zero length\n");
		}

		rc = stat(fileName, &s);

		if (rc != 0)
		{
			e = errno;

			m_messenger->textError("Couldn't stat file [%s] - error return was %s\n", fileName, strerror(e));
			return false;
		}

		if (S_ISREG(s.st_mode))
		{
			return true;
		}

		return false;
	}

	void Device::About(void)
	{
		if (m_rdb)
		{
			m_messenger->textInfo("Device has:\n\ta rigid disk block\n");
			m_messenger->textInfo("\tblock size %d\n", fe32(m_rdb->blockBytes));
			m_messenger->textInfo("\tphysical C/H/S %d, %d, %d\n", fe32(m_rdb->cylinders), fe32(m_rdb->heads), fe32(m_rdb->sectors));
			m_messenger->textInfo("\t%d partitions\n", volumeCount());
			{
				Volume *V; int I;

				for (I=1, V = m_firstVol; V; I++, V = V->m_nextVol)
					m_messenger->textInfo("\t\t%d. %s partion, start [%ld], count [%ld], type [%s]...\n", I, V->volName(), V->volStartBlock(), V->volBlockCount(), V->volType());
			}
			// m_messenger->textInfo("\tblock count %ld\n", ((((u64)fixEndian32(m_rdb->rdbBlocksHi)) << 32) | ((u64)fixEndian32(m_rdb->rdbBlocksLo))));
		}
	}
}

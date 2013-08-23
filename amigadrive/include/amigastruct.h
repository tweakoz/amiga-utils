#ifndef AMIGASTRUCT_H_INCLUDED
#define AMIGASTRUCT_H_INCLUDED
/*
 * The disk structures were copied from code written by
 *
 * Hans-Joerg Frieden, Hyperion Entertainment
 * Hans-JoergF@hyperion-entertainment.com
 */
#include "amigatypes.h"

namespace amigadrive
{
	/*!
	 * Amiga disks have a very open structure. The head for the partition table information
	 * is stored somewhere within the first 16 blocks on disk, and is called the
	 * "RigidDiskBlock". Note that all 16 and 32 bit values are stored big endian, as per
	 * the Motorola 68K cpu. Intel x86 architecture cpus are little endian.
	 */
	struct rigidDiskBlock
	{
		u32 id;
		u32 summedLongs;
		s32 chkSum;
		u32 hostid;
		u32 blockBytes;
		u32 flags;
		u32 badBlockList;
		u32 partitionList;
		u32 fileSysHeaderList;
		u32 driveInit;
		u32 bootCodeBlock;
		u32 reserved_1[5];

		/* Physical drive geometry */
		u32 cylinders;
		u32 sectors;
		u32 heads;
		u32 interleave;
		u32 park;
		u32 reserved_2[3];
		u32 writePreComp;
		u32 reducedWrite;
		u32 stepRate;
		u32 reserved_3[5];

		/* logical drive geometry */
		u32 rdbBlocksLo;
		u32 rdbBlocksHi;
		u32 loCylinder;
		u32 hiCylinder;
		u32 cylBlocks;
		u32 autoParkSeconds;
		u32 highRDSKblock;
		u32 reserved_4;

		char diskVendor[8];
		char diskProduct[16];
		char diskRevision[4];
		char controllerVendor[8];
		char controllerProduct[16];
		char controllerRevision[4];

		u32 reserved_5[10];
	};

	/*!
	 * Each partition on this drive is defined by such a block.
	 */
	struct partitionBlock
	{
		u32 id;
		u32 summedLongs;
		s32 chkSum;
		u32 hostid;
		u32 next;
		u32 flags;
		u32 reserved_1[2];
		u32 devFlags;
		char driveName[32];
		u32 reserved_2[15];
		u32 environment[17];
		u32 reserved_3[15];
	};

	/*!
	*	Header for boot loader code
	*/
	struct bootcodeBlock
	{
		u32   id;
		u32   summedLongs;
		s32   chkSum;
		u32   hostid;
		u32   next;
		u32   loadData[123];
	};

	#define AMIGA_ID_RDISK                  0x5244534B
	#define AMIGA_ID_PART                   0x50415254
	#define AMIGA_ID_BOOT                   0x424f4f54

	/*!
	 * The environment array in the partition block
	 * describes the partition. This is a template
	 * struture describing that data area.
	 *
	 * struct amigaPartGeometry *g = (struct amigaPartGeometry)&(m_partBlock->environment)
	 */
	struct amigaPartGeometry
	{
		u32 tableSize;
		u32 sizeBlocks;
		u32 unused1;
		u32 surfaces;

		u32 sectorPerBlock;
		u32 blockPerTrack;
		u32 reserved;
		u32 prealloc;

		u32 interleave;
		u32 lowCyl;
		u32 highCyl;
		u32 numBuffers;

		u32 bufMemType;
		u32 maxTransfer;
		u32 mask;
		s32 bootPriority;

		u32 dosType;
		u32 baud;
		u32 control;
		u32 bootBlocks;
	};
}

#endif // AMIGASTRUCT_H_INCLUDED

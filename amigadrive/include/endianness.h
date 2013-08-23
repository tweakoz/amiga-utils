#ifndef ENDIANNESS_H_INCLUDED
#define ENDIANNESS_H_INCLUDED

namespace amigadrive
{
	/*!
	*	returns true if the CPU is little-endian
	*/
	bool isLittleEndian(void);

	/*!
	*	Swaps endianness for the 32 bit paramer, but only if the cpu is little-endian.
	*/
	u32 fe32(u32 x);

	/*!
	*	Swaps endianness for the 16 bit parameter, but only if the cpu is little-endian.
	*/
	u16 fe16(u16 x);
};

#endif // ENDIANNESS_H_INCLUDED

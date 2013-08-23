#include "amigatypes.h"
#include "endianness.h"

namespace amigadrive
{
	extern bool g_littleEndian;

	struct startendian32
	{
		u8 a;
		u8 b;
		u8 c;
		u8 d;
	};

	struct swapendian32
	{
		u8 d;
		u8 c;
		u8 b;
		u8 a;
	};

	union endian32
	{
		swapendian32 w;
		startendian32 t;
		u32 u;
	};

	/*!
	* (f)ix(e)ndian32 - swaps from 32-bit big-endian to little-endian and back, but only if the cpu is little-endian.
	*/
	u32 fe32(u32 x)
	{
		if (g_littleEndian)
		{
			endian32 i, j;

			i.u = x;
			j.w.a = i.t.a;
			j.w.b = i.t.b;
			j.w.c = i.t.c;
			j.w.d = i.t.d;

			return j.u;
		}
		return x;
	}

	struct startendian16
	{
		u8 a;
		u8 b;
	};

	struct swapendian16
	{
		u8 b;
		u8 a;
	};

	union endian16
	{
		swapendian16 w;
		startendian16 t;
		u16 u;
	};

	/*!
	* (f)ix(e)ndian16 - Swaps from 16-bit big-endian to little-endian and back, but only if the cpu is little-endian.
	*/
	u16 fe16(u16 x)
	{
		if (g_littleEndian)
		{
			endian16 i, j;

			i.u = x;
			j.w.a = i.t.a;
			j.w.b = i.t.b;

			return j.u;
		}
		return x;
	}

	/*!
	*	Returns true if little-endian.
	*/
	bool isLittleEndian(void)
	{
		struct bigendian
		{
			u8 hi;
			u8 lo;
		};

		union
		{
			struct bigendian b;
			s16 n;
		} u;

		u.n = 0x0201;

		if (u.b.hi == 2)
			return false;
		return true;
	}
};

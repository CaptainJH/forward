#include "Types.h"

namespace
{
	void TypesCheck()
	{
		using namespace forward;

		static_assert(sizeof(f32) == 4, "f32 size is incorrect");

		static_assert(sizeof(i64) == 8, "i64 size is incorrect");
		static_assert(sizeof(i32) == 4, "i32 size is incorrect");
		static_assert(sizeof(i16) == 2, "i16 size is incorrect");
		static_assert(sizeof(i8) == 1, "i8 size is incorrect");

		static_assert(sizeof(u64) == 8, "u64 size is incorrect");
		static_assert(sizeof(u32) == 4, "u32 size is incorrect");
		static_assert(sizeof(u16) == 2, "u16 size is incorrect");
		static_assert(sizeof(u8) == 1, "u8 size is incorrect");
	}
}
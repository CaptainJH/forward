#include <string>
#include <vector>

#ifdef _WINDOWS
#include <concurrent_vector.h>
#include <concurrent_queue.h>
#endif

#include <ImathVec.h>
#include <ImathMatrix.h>


namespace forward
{
	typedef char i8;
	typedef short i16;
	typedef int i32;
	typedef long long i64;
	typedef unsigned char u8;
	typedef unsigned short u16;
	typedef unsigned int u32;
	typedef unsigned long long u64;

	typedef std::atomic_uint64_t	atomic_u64;

	typedef float f32;
	typedef double f64;

	typedef std::wstring WString;
	typedef std::string String;
	template<class T> using Vector = std::vector<T>;

	typedef Imath::Vec3<forward::f32> float2;
	typedef Imath::Vec3<forward::f32> float3;
	typedef Imath::Vec4<forward::f32> float4;
	typedef Imath::Matrix33<forward::f32> float3x3;
	typedef Imath::Matrix44<forward::f32> float4x4;


#ifdef _WINDOWS
	template<class T> using Concurrent_Vector = Concurrency::concurrent_vector<T>;
	template<class T> using Concurrent_Queue = Concurrency::concurrent_queue<T>;

#endif


}
#include <string>
#include <vector>

#ifdef _WINDOWS
#include <concurrent_vector.h>
#include <concurrent_queue.h>
#endif


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


#ifdef _WINDOWS
	template<class T> using Concurrent_Vector = Concurrency::concurrent_vector<T>;
	template<class T> using Concurrent_Queue = Concurrency::concurrent_queue<T>;

#endif


}
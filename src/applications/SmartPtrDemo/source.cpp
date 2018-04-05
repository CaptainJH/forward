#include "PCH.h"
#include "SmartPtrs.h"
#include <memory>
#include <iostream>
#include <type_traits>

using namespace forward;

struct A : public forward::intrusive_ref_counter
{
	A(int in)
	{
		a = in;
	}
	int a;
};

struct B : public A
{
	B(int in)
		: A(in)
	{
		a = in + 1;
	}
	int a;
};

i32 main()
{
	std::cout << sizeof(shared_ptr<B>) << std::endl;
	std::cout << sizeof(weak_ptr<B>) << std::endl;

	shared_ptr<B> pB = make_shared<B>(5);
	{
		shared_ptr<B> pB2(pB.get());
	}
	std::cout << pB->a << std::endl; // doesn't work for std::shared_ptr!

	shared_ptr<A> pA(pB); // OK!
	//shared_ptr<B> pB2(pA); // cannot compile

	// cannot compile
	//shared_ptr<int> pfloat = make_shared<int>(5);

	//i32* raw_ptr = new int(5);
	//weak_ptr<int> wp(raw_ptr); // cannot compile!


	return 0;
}
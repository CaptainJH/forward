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
	//bool b2a = std::is_convertible<B*, A*>::value;
	//std::cout << b2a << std::endl;

	//shared_ptr<B> pint = make_shared<B>(5);
	//shared_ptr<A> pa(pint);
	//shared_ptr<float> pfloat(pint);

	std::shared_ptr<A> pint = std::make_shared<A>(5);
	std::shared_ptr<B> pfloat(pint);


	return 0;
}
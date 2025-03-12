#pragma once
#include <assert.h>
#include <type_traits>
#include <unordered_map>
#include <algorithm>
#include "Types.h"

namespace forward
{
	class intrusive_ref_counter;

	class weak_ptr_base
	{
	protected:
		intrusive_ref_counter* px = nullptr;

		void register_weak_ptr();
		void deregister_weak_ptr();

		friend class intrusive_ref_counter;
	};

	class intrusive_ref_counter
	{
		friend class weak_ptr_base;

	private:
		//! Reference counter type
		typedef u32 counter_type;
		//! Reference counter
		mutable counter_type m_ref_counter;

	public:
		/*!
		* Default constructor
		*
		* \post <tt>use_count() == 0</tt>
		*/
		intrusive_ref_counter()
			: m_ref_counter(0)
		{
		}

		/*!
		* Copy constructor
		*
		* \post <tt>use_count() == 0</tt>
		*/
		intrusive_ref_counter(intrusive_ref_counter const&)
			: m_ref_counter(0)
		{
		}

		/*!
		* Assignment
		*
		* \post The reference counter is not modified after assignment
		*/
		intrusive_ref_counter& operator= (intrusive_ref_counter const&) { return *this; }

		/*!
		* \return The reference counter
		*/
		u32 use_count() const
		{
			return (m_ref_counter);
		}

	protected:
		/*!
		* Destructor
		*/
		virtual ~intrusive_ref_counter()
		{
			auto it = m_sWeakPtrTable.find(this);
			if (it != m_sWeakPtrTable.end())
			{
				std::for_each(it->second.begin(), it->second.end(), [](auto& weakp) {
					if (weakp) weakp->px = nullptr;
					});
				/// JHQ: 
				/// this is a very tricky bug. the instrusive_ref_counter object should always
				/// remove its entry in m_sWeakPtrTable when it is destroyed. Because later on, 
				/// a new object might be created with the same address, and this will cause serious
				/// problem when that new object get destroyed.
				m_sWeakPtrTable.erase(this);
			}
		}


		static std::unordered_map<intrusive_ref_counter*, std::vector<weak_ptr_base*>> m_sWeakPtrTable;

	public:
		static void intrusive_ptr_add_ref(const intrusive_ref_counter* p)
		{
			++p->m_ref_counter;
		}
		static void intrusive_ptr_release(const intrusive_ref_counter* p)
		{
			if (--p->m_ref_counter == 0)
				delete (p);
		}
	};

	//
	//  shared_ptr
	//
	//  A smart pointer that uses intrusive reference counting.
	//
	//  Relies on unqualified calls to
	//  
	//      void intrusive_ptr_add_ref(T * p);
	//      void intrusive_ptr_release(T * p);
	//
	//          (p != nullptr)
	//
	//  The object is responsible for destroying itself.
	//

	template<class T>
	class shared_ptr
	{
	private:
		typedef shared_ptr this_type;

	public:
		typedef T element_type;

		shared_ptr() 
			: px(nullptr)
		{
		}

		shared_ptr(T * p, bool add_ref = true) 
			: px(p)
		{
			if (px != nullptr && add_ref) 
				intrusive_ref_counter::intrusive_ptr_add_ref(px);
		}

		template<class U, typename std::enable_if<std::is_convertible<U*, T*>::value, size_t>::type = 0>
		shared_ptr(shared_ptr<U> const& rhs)
			: px((T*)rhs.get())
		{
			if (px != nullptr)
			{
				intrusive_ref_counter::intrusive_ptr_add_ref(px);
			}
		}

		shared_ptr(shared_ptr const & rhs) 
			: px(rhs.px)
		{
			if (px != nullptr) 
				intrusive_ref_counter::intrusive_ptr_add_ref(px);
		}

		~shared_ptr()
		{
			if (px != nullptr) 
				intrusive_ref_counter::intrusive_ptr_release(px);
		}

		// Move support
		shared_ptr(shared_ptr && rhs) 
			: px(rhs.px)
		{
			rhs.px = nullptr;
		}

		shared_ptr & operator=(shared_ptr && rhs)
		{
			this_type(static_cast< shared_ptr && >(rhs)).swap(*this);
			return *this;
		}

		shared_ptr & operator=(shared_ptr const & rhs)
		{
			this_type(rhs).swap(*this);
			return *this;
		}

		shared_ptr & operator=(T * rhs)
		{
			this_type(rhs).swap(*this);
			return *this;
		}

		void reset()
		{
			this_type().swap(*this);
		}

		void reset(T * rhs)
		{
			this_type(rhs).swap(*this);
		}

		void reset(T * rhs, bool add_ref)
		{
			this_type(rhs, add_ref).swap(*this);
		}

		T * get() const
		{
			return px;
		}

		T * detach()
		{
			T * ret = px;
			px = nullptr;
			return ret;
		}

		T & operator*() const
		{
			assert(px != nullptr);
			return *px;
		}

		T * operator->() const
		{
			assert(px != nullptr);
			return px;
		}

		explicit operator bool() const
		{
			return px != nullptr;
		}

		void swap(shared_ptr & rhs)
		{
			T * tmp = px;
			px = rhs.px;
			rhs.px = tmp;
		}

	private:
		T * px;
	};

	template<class _Ty, class... _Types>
	shared_ptr<_Ty> make_shared(_Types&&... _Args)
	{	
		// make a shared_ptr
        const auto _Rx = new _Ty(std::forward<_Types>(_Args)...);

		shared_ptr<_Ty> _Ret(_Rx);
		return (_Ret);
	}

	template<class T> 
	class weak_ptr : public weak_ptr_base
	{
	private:
		typedef weak_ptr<T> this_type;

	public:

		typedef T element_type;

		~weak_ptr()
		{
			if (px)
			{
				deregister_weak_ptr();
			}
		}

		weak_ptr() 
		{
			px = nullptr;
		}

		//  generated copy constructor, assignment, destructor are fine...

		weak_ptr(weak_ptr const & r) 
		{
			px = r.px;
			if (px)
			{
				register_weak_ptr();
			}
		}

		weak_ptr(T* p)
		{
			px = p;
			if (px)
			{
				register_weak_ptr();
			}
		}

		weak_ptr & operator=(weak_ptr const & r)
		{
			if (px)
			{
                deregister_weak_ptr();
			}
			px = r.px;
			if (px)
			{
				register_weak_ptr();
			}
			return *this;
		}

		// for better efficiency in the T == Y case
		weak_ptr(weak_ptr && r)
		{
			px = r.px;
			if (px)
			{
				register_weak_ptr();
				r.deregister_weak_ptr();
			}
			r.px = nullptr;
		}

		// for better efficiency in the T == Y case
		weak_ptr & operator=(weak_ptr && r)
		{
			this_type(static_cast< weak_ptr && >(r)).swap(*this);
			return *this;
		}

		shared_ptr<T> lock()
		{
			return forward::shared_ptr<T>(getTypedPtr());
		}

		template<class U, typename std::enable_if<std::is_convertible<U*, T*>::value, size_t>::type = 0>
		shared_ptr<U> lock_down()
		{
			auto ptr = dynamic_cast<U*>(px);
			if (ptr)
			{
				return shared_ptr<U>(ptr);
			}
			else
			{
				return nullptr;
			}
		}

		bool expired() const
		{
			return px == nullptr;
		}

		void reset()
		{
			this_type().swap(*this);
		}

		void reset(T* p)
		{
			this_type(p).swap(*this);
		}

		void swap(this_type & other)
		{
			if (px)
			{
				deregister_weak_ptr();
			}
			if (other.px)
			{
				other.deregister_weak_ptr();
			}
			std::swap(px, other.px);
			if (px)
			{
				register_weak_ptr();
			}
			if (other.px)
			{
				other.register_weak_ptr();
			}
		}

	private:
		T * getTypedPtr()
		{
			return static_cast<T*>(px);
		}
	};  // weak_ptr

}

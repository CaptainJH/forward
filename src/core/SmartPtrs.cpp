#include "SmartPtrs.h"
#include <algorithm>

using namespace forward;


void weak_ptr_base::register_weak_ptr()
{
	intrusive_ref_counter::m_sWeakPtrTable[px].push_back(this);
}

void weak_ptr_base::deregister_weak_ptr()
{
	auto it = intrusive_ref_counter::m_sWeakPtrTable.find(px);
	assert(it != intrusive_ref_counter::m_sWeakPtrTable.end());

	auto it_weak = std::find(it->second.begin(), it->second.end(), this);
	assert(it_weak != it->second.end());
	it->second.erase(it_weak);
}

std::map<intrusive_ref_counter*, std::vector<weak_ptr_base*>> intrusive_ref_counter::m_sWeakPtrTable;
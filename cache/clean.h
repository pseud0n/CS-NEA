#ifdef DO_CACHE_DECL

#ifndef CACHE_CLEAN_H
#define CACHE_CLEAN_H

for (UL::InternalObject<UL::Aliases::NumT>* cached_num_ptr : UL::Cache::numbers)
	delete cached_num_ptr;
#define DEL_BOOL(ptr) delete (UL::InternalObject<UL::Aliases::BoolT>*)ptr;
DEL_BOOL(UL::Cache::bool_true)
DEL_BOOL(UL::Cache::bool_false)
#undef DEL_BOOL

#endif

#endif
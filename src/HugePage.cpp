#include <cerrno>		// errno
#include <cmath>		// abs, ceil, floor, isnan, log, log10, log2, pow, round, sqrt
#include <cstring>		// strlen, strncmp, strncpy, memset, memcpy, memmove, strerror
// #include <hugetlbfs.h> 还没弄懂怎么链接 libhugetlbfs
#include <iostream>
#include <sys/mman.h>	// shm_open, PROT_READ, PROT_WRITE, MAP_PRIVATE, MAP_ANON

#include "leonutils/Exceptions.hpp"
#include "leonutils/HugePage.hpp"

namespace leon_utl {

HugePage_t::~HugePage_t() {
	if( _shm_p != nullptr )
		release();
};

constexpr uintptr_t HUGE_PAGE_SIZE = 2048 * 1024;

ssize_t HugePage_t::make( ssize_t b_ ) {
	/*	_1page = gethugepagesize(); 还没弄懂怎么链接 libhugetlbfs
		std::cerr << "HugePage Size:" << _1page; */
	_1page = HUGE_PAGE_SIZE;

	if( _shm_p != nullptr || _bytes != 0 )
		throw bad_usage( "重复申请的HugePage!!!" );

	b_ = std::ceil( static_cast<double>( b_ ) / _1page );
	b_ *= _1page;

	/*
		a. m_mask: 操作共享内存的权限 (PROT_READ | PROT_WRITE)
		b. map_flags:
			HUGEPAGE 必须是 MAP_HUGETLB | MAP_ANON | MAP_PRIVATE
	 */
	auto m_mask = PROT_READ | PROT_WRITE;
	int map_flags = MAP_HUGETLB | MAP_ANON | MAP_PRIVATE;

	_shm_p = mmap( NULL, b_, m_mask, map_flags, -1, 0 );
	if( _shm_p == MAP_FAILED )
		throw std::runtime_error( "mmap错误:" + str_t( std::strerror( errno ) ) );
	if( reinterpret_cast<uintptr_t>( _shm_p ) % HUGE_PAGE_SIZE != 0 )
		std::cerr << "HugePage居然不对齐2M边界:"
				  << reinterpret_cast<uintptr_t>( _shm_p );

	return _bytes = b_;
};

void HugePage_t::release() {
	if( _shm_p == nullptr )
		throw bad_usage( "重复释放shm!!!" );

	if( munmap( _shm_p, _bytes ) != 0 )
		std::cerr << "munmap错误:\"" << str_t( std::strerror( errno ) ) << "\".";

	_shm_p = nullptr;
	_bytes = 0;
};

};	// namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;

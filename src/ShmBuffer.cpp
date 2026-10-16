#include <cmath>		// ceil
#include <cerrno>		// errno
#include <cstring>		// strlen, strncmp, strncpy, memset, memcpy, memmove, strerror
#include <fcntl.h>		// O_CREAT, O_TRUNC, O_RDWR, S_IRUSR, S_IWUSR
#include <filesystem>
#include <iostream>
#include <sys/mman.h>	// shm_open, PROT_READ, PROT_WRITE, MAP_PRIVATE, MAP_ANON
#include <unistd.h>		// syscall, ftruncate, open, close, getpagesize

#include "leonutils/Exceptions.hpp"
#include "leonutils/ShmBuffer.hpp"

namespace leon_utl {

namespace fs = std::filesystem;
using path_t = fs::path;
using fs::exists;

ShmBuffer_t::~ShmBuffer_t() {
	if( _shm_p != nullptr )
		unplug();
};

void* ShmBuffer_t::_mk_plug( str_cr n_, size_t b_, bool tr_,
							 int f_mask_, mode_t u_mask_, int m_mask_ ) {
	_1page = getpagesize();

	if( _shm_p != nullptr || _bytes != 0 || !_shm_n.empty() )
		throw bad_usage( "用于\"" + n_ + "\"的ShmBuffer对象已作为\"" + _shm_n + "\"之用!!!" );

	auto shm_fd = shm_open( n_.c_str(), f_mask_, u_mask_ );
	if( shm_fd < 0 )
		throw std::runtime_error( "shm_open错误:" + str_t( std::strerror( errno ) ) );

	if( tr_ && ftruncate( shm_fd, b_ ) != 0 )
		throw std::runtime_error( "ftruncate错误:" + str_t( std::strerror( errno ) ) );

	auto shm_pt = mmap( NULL, b_, m_mask_, MAP_SHARED_VALIDATE, shm_fd, 0 );
	if( shm_pt == MAP_FAILED )
		throw std::runtime_error( "mmap错误:" + str_t( std::strerror( errno ) ) );

	if( close( shm_fd ) != 0 )
		throw std::runtime_error( "close( shm_fd )错误:" +
								  str_t( std::strerror( errno ) ) );

	if( ( reinterpret_cast<uintptr_t>( shm_pt ) & 63 ) != 0 )
		throw std::runtime_error( n_ + ":地址未从64字节整倍数开始!" );

	return shm_pt;
};

size_t ShmBuffer_t::make( str_cr n_, size_t b_, bool wr_, bool log_ ) {

	// 把原有 shm 删除
	shm_unlink( n_.c_str() );
	path_t shm_path { "/dev/shm/" + n_ };
	if( fs::exists( shm_path ) )
		std::cerr << "shm_unlink之后依然存在:" << shm_path << ",shm尺寸可能错乱!" << std::endl;

	/* 1.权限分三类:
		a. f_mask: 当前打开的fd对应的操作权限 (O_RDONLY | O_RDWR | O_CREAT | O_TRUNC)
			仅当有写权(O_RDWR)时才能 ftruncate 文件, 所以创建者必有 O_RDWR.
		b. u_mask: 在文件系统内的权限 (S_IRUSR | S_IWUSR)
			统一保持为(S_IRUSR | S_IWUSR), 因为这是针对某个 Linux 用户的权限. 只要进程
			由某个 user 运行, 即便是创建者也不能修改.
		c. m_mask: 操作共享内存的权限 (PROT_READ | PROT_WRITE)
	 */
	auto f_mask = O_RDWR | O_CREAT | O_TRUNC;
	auto u_mask = S_IRUSR | S_IWUSR;
	auto m_mask = PROT_READ;
	if( wr_ ) { m_mask |= PROT_WRITE; }

	_shm_p = _mk_plug( n_, b_, true, f_mask, u_mask, m_mask );
	_shm_n = n_;

	if( fs::exists( shm_path ) ) {
		_bytes = fs::file_size( shm_path );
		if( _bytes != b_ )
			std::cerr << "实际创建的shm大小:" << _bytes << "不等于期望值:" << b_ << std::endl;
		// else std::cerr << "成功创建shm:" << shm_path << ",文件尺寸:" << _bytes;
	} else {
		_shm_n.clear(); _shm_p = nullptr; _bytes = 0;
		throw std::runtime_error( shm_path.native() + ":shm创建完了居然不存在?" );
	}

	_pages = std::ceil( static_cast<float>( _bytes ) / _1page );
	if( log_ )
		std::cout << "shm:'" << _shm_n << "' created:" << _bytes << "bytes in "
				  << _pages << "pages." << std::endl;
	return _bytes;
};

size_t ShmBuffer_t::plug( str_cr n_, bool wr_, bool log_ ) {

	// 原有 shm 必须已经存在, 否则报错
	path_t shm_path { "/dev/shm/" + n_ };
	size_t rs {};
	if( fs::exists( shm_path ) ) {
		rs = fs::file_size( shm_path );
		// std::cerr << "将要对接shm:" << n_ << ",文件尺寸:" << rs;
	} else
		throw std::runtime_error( shm_path.native() + ":shm不存在?" );

	/* 1.权限分三类:
		a. f_mask: 当前打开的fd对应的操作权限 (O_RDONLY | O_RDWR | O_CREAT | O_TRUNC)
			仅当有写权(O_RDWR)时才能 ftruncate 文件,否则创建者也不能.
		b. u_mask: 在文件系统内的权限 (S_IRUSR | S_IWUSR)
			统一保持为(S_IRUSR | S_IWUSR), 因为这是针对某个 Linux 用户的权限. 只要进程
			由某个 user 运行, 即便是创建者也不能修改.
		c. m_mask: 操作共享内存的权限 (PROT_READ | PROT_WRITE)
	 */
	auto f_mask = O_RDONLY;
	auto u_mask = S_IRUSR | S_IWUSR;
	auto m_mask = PROT_READ;
	if( wr_ ) { f_mask |= O_RDWR; m_mask |= PROT_WRITE; }

	_shm_p = _mk_plug( n_, rs, false, f_mask, u_mask, m_mask );
	_shm_n = n_;
	_bytes = rs;

	_pages = std::ceil( static_cast<float>( _bytes ) / _1page );
	if( log_ )
		std::cout << "shm:'" << _shm_n << "' plugged:" << _bytes << "bytes in "
				  << _pages << "pages." << std::endl;
	return _bytes;
};

size_t ShmBuffer_t::makeOrPlug( str_cr n_, size_t b_, bool wr_, bool log_ ) {

	// 同名 shm 如果存在, 就plug, 否则make
	path_t shm_path { "/dev/shm/" + n_ };

	if( fs::exists( shm_path ) )
		return plug( n_, wr_, log_ );
	else
		return make( n_, b_, wr_, log_ );
};

void ShmBuffer_t::unplug( bool rm_, bool log_ ) {

	if( _shm_p == nullptr )
		throw bad_usage( "重复释放shm!!!" );

	if( munmap( _shm_p, _bytes ) != 0 )
		std::cerr << "munmap错误:\"" << str_t( std::strerror( errno ) )
				  << "\",shm_name:" << _shm_n << std::endl;
	else if( log_ )
		std::cout << "shm:'" << _shm_n << "' release:" << _bytes << "bytes" << std::endl;

	if( rm_ )
		delOsFile();

	_shm_n.clear();
	_shm_p = nullptr;
	_bytes = 0;
};

void ShmBuffer_t::delOsFile() const {

	if( _shm_n.empty() )
		return;

	path_t shm_path { "/dev/shm/" + _shm_n };

	if( ! fs::exists( shm_path ) ) {
//		std::cerr << "准备删除:\"" << shm_path << "\"时,它已经不存在了.";
		return;
	}

	if( shm_unlink( _shm_n.c_str() ) != 0 )
		std::cerr << "shm_unlink错误:\"" << std::strerror( errno )
				  << "\",shm_name:" << _shm_n << std::endl;
};

size_t ShmBuffer_t::tagCold( const void* end_p_, size_t beg_ ) {
	return _madvise( __func__, MADV_DONTNEED, beg_, _page_no( end_p_ ), _coold );
};

size_t ShmBuffer_t::tagPage( int64_t beg_, int64_t end_ ) {
	return _madvise( __func__, MADV_DONTNEED, beg_, end_, _coold );
};

size_t ShmBuffer_t::swapOut( const void* end_p_, size_t beg_ ) {
	return _madvise( __func__, MADV_PAGEOUT, beg_, _page_no( end_p_ ), _swapd );
};

size_t ShmBuffer_t::pageOut( int64_t beg_, int64_t end_ ) {
	return _madvise( __func__, MADV_PAGEOUT, beg_, end_, _swapd );
};

size_t ShmBuffer_t::_page_no( const void* addr_ ) const {

	/* 尽管64位地址很难遇到最高位为1, 但不能完全排除. 所以地址运算有时需要用有符号运算,
		有时又需要用无符号运算, 必须处处小心. */
	uintptr_t uptr_addr = reinterpret_cast<uintptr_t>( addr_ );
	uintptr_t uptr_base = reinterpret_cast<uintptr_t>( _shm_p );

	// end_p_ 不应该小于 _shm_p, 否则就是收到了错误的参数
	if( uptr_addr < uptr_base ) {
		std::cerr << _shm_n << '.' << __func__ << ":收到的地址("
				  << addr_ << ")还小于基址(" << _shm_p << "),已拒绝!";
		return 0;
	}

	// 变为页序号
	return ( uptr_addr - uptr_base ) / _1page;
};

size_t ShmBuffer_t::_madvise( char_cp fn_, int op_, int64_t beg_, int64_t end_,
							  size_t& last_end_ ) {

	// 跳过开始多少页(beg_为负表示:倒数第几页)
	int64_t signed_pages = _pages;
	beg_ = beg_ >= 0 ? beg_ : signed_pages + beg_;
	beg_ = std::min( signed_pages - 1, std::max<int64_t>( 0, beg_ ) );

	// 跳过末尾多少页(end_为负表示:倒数第几页)
	end_ = end_ > 0 ? end_ : signed_pages + end_;
	end_ = std::min( signed_pages, std::max<int64_t>( 0, end_ ) );

	// 因为 end_ 有可能小于 last_end_, 所以结果要有符号
	int64_t s_last_end = last_end_;
	int64_t more_pages = end_ - s_last_end;	// 本次标记范围新增多少页
	int64_t cool_pages = end_ - beg_;		// 本次总共需要标记多少页

	// 为减少系统调用次数(很耗时), 如果新增(或总共)需要标记的页数少于 64, 就不做.
	if( more_pages < 64 || cool_pages < 64 )
		return 0;

	// 需要标记(换出)的字节数
	auto to_be = cool_pages * _1page;

	// 需要标记的起始地址
	auto tag_head = reinterpret_cast<uintptr_t>( _shm_p ) + _1page * beg_;

	if( madvise( reinterpret_cast<void*>( tag_head ), to_be, op_ ) != 0 ) {
		auto e = errno;
		std::cerr << _shm_n << '.' << fn_ << ":错误:\"" << std::strerror( e )
				  << "\"." << std::endl;
		return 0;
	}

	last_end_ = end_;
	return to_be;
};

};	// namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;

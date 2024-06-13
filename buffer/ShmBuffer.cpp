#include <cerrno>		// errno
#include <cstring>		// strlen, strncmp, strncpy, memset, memcpy, memmove, strerror
#include <fcntl.h>		// O_CREAT, O_TRUNC, O_RDWR, S_IRUSR, S_IWUSR
#include <filesystem>
#include <iostream>
#include <sys/mman.h>	// shm_open, PROT_READ, PROT_WRITE, MAP_PRIVATE, MAP_ANON
#include <unistd.h>		// syscall, ftruncate, open, close

#include "ShmBuffer.hpp"
#include "except/Exceptions.hpp"

namespace leon_utl {

namespace fs = std::filesystem;
using path_t = fs::path;
using fs::exists;

ShmBuffer_t::~ShmBuffer_t() {
	if( _shm_p != nullptr )
		unplug();
};

void* CreateOrPlug( const str_t& n_, size_t b_, bool cr_,
					int f_mask_, mode_t u_mask_, int m_mask_ ) {

	auto shm_fd = shm_open( n_.c_str(), f_mask_, u_mask_ );
	if( shm_fd < 0 )
		throw std::runtime_error( "shm_open错误:" + str_t( std::strerror( errno ) ) );

	if( cr_ && ftruncate( shm_fd, b_ ) != 0 )
		throw std::runtime_error( "ftruncate错误:" + str_t( std::strerror( errno ) ) );

	auto shm_pt = mmap( NULL, b_, m_mask_, MAP_SHARED_VALIDATE, shm_fd, 0 );
	if( shm_pt == MAP_FAILED )
		throw std::runtime_error( "mmap错误:" + str_t( std::strerror( errno ) ) );

	if( close( shm_fd ) != 0 )
		throw std::runtime_error( "close( shm_fd )错误:" +
								  str_t( std::strerror( errno ) ) );

	if( ( reinterpret_cast<intptr_t>( shm_pt ) & 63 ) != 0 )
		throw std::runtime_error( n_ + ":地址未从64字节整倍数开始!" );

	return shm_pt;
};

size_t ShmBuffer_t::make( const str_t& n_, size_t b_ ) {
	if( _shm_p != nullptr || _bytes != 0 || !_shm_n.empty() )
		throw bad_usage( "用于申请shm:\"" + n_ +
						 "\"的ShmBuffer_t对象已作为\"" + _shm_n + "\"之用!!!" );

	// 把原有 shm 删除
	shm_unlink( n_.c_str() );
	path_t shm_path { "/dev/shm/" + n_ };
	if( fs::exists( shm_path ) )
		std::cerr << "shm_unlink之后依然存在:" << shm_path << ",shm尺寸可能错乱!";

	/* 1.权限分三类:
		a. f_mask: 当前打开的fd对应的操作权限 (O_RDONLY | O_RDWR | O_CREAT | O_TRUNC)
			仅当有写权(O_RDWR)时才能 ftruncate 文件,否则创建者也不能.
		b. u_mask: 在文件系统内的权限 (S_IRUSR | S_IWUSR)
			统一保持为(S_IRUSR | S_IWUSR), 因为这是针对某个 Linux 用户的权限. 只要进程
			由某个 user 运行, 即便是创建者也不能修改.
		c. m_mask: 操作共享内存的权限 (PROT_READ | PROT_WRITE)
	 */
	auto f_mask = O_RDWR | O_CREAT | O_TRUNC;
	auto u_mask = S_IRUSR | S_IWUSR;
	auto m_mask = PROT_READ | PROT_WRITE;
	_shm_p = CreateOrPlug( n_, b_, true, f_mask, u_mask, m_mask );
	_shm_n = n_;

	if( fs::exists( shm_path ) ) {
		_bytes = fs::file_size( shm_path );
		if( _bytes != b_ )
			std::cerr << "实际创建的shm大小:" << _bytes << "不等于期望值:" << b_;
		// else std::cerr << "成功创建shm:" << shm_path << ",文件尺寸:" << _bytes;
	} else {
		_shm_n.clear(); _shm_p = nullptr; _bytes = 0;
		throw std::runtime_error( shm_path.native() + ":shm创建完了居然不存在?" );
	}

	return _bytes;
};

size_t ShmBuffer_t::plug( const str_t& n_, bool wr_ ) {
	if( _shm_p != nullptr || _bytes != 0 || !_shm_n.empty() )
		throw bad_usage( "用于申请shm:\"" + n_ +
						 "\"的ShmBuffer_t对象已作为\"" + _shm_n + "\"之用!!!" );

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
	if( wr_ ) {
		f_mask |= O_RDWR;
		m_mask |= PROT_WRITE;
	}
	_shm_p = CreateOrPlug( n_, rs, false, f_mask, u_mask, m_mask );
	_shm_n = n_;
	_bytes = rs;
	return _bytes;
};

void ShmBuffer_t::unplug( bool rm_ ) {
	if( _shm_p == nullptr )
		throw bad_usage( "重复释放shm!!!" );

	path_t shm_path { "/dev/shm/" + _shm_n };
	// std::cerr << shm_path << "munmap前:" << ( exists( shm_path ) ? "存在" : "不存在" );

	if( munmap( _shm_p, _bytes ) != 0 )
		std::cerr << "munmap错误:\"" << str_t( std::strerror( errno ) )
				  << "\",shm_name:" << _shm_n;
	// std::cerr << shm_path << ":munmap后:" << ( exists( shm_path ) ? "存在" : "不存在" );

	if( rm_ ) {
		if( ! fs::exists( shm_path ) )
			std::cerr << "准备删除:" << shm_path << "时,它已经不存在了.";
		else if( shm_unlink( _shm_n.c_str() ) != 0 )
			std::cerr << "shm_unlink错误:\"" << std::strerror( errno )
					  << "\",shm_name:" << _shm_n;
	}

	_shm_n.clear();
	_shm_p = nullptr;
	_bytes = 0;
};

};	// namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;

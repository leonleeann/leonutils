#pragma once

namespace leon_ext {

using str_t = std::string;

/*******************************************************************************
 * 一个 SharedBuffer 封装一块共享内存,将申请、释放等琐碎操作集中起来.
 * 主要用于:
 *		a.Tick行情共享
 *		b.日线行情共享
 *		c.跨进程原子变量、消息队列
 */
class ShmBuffer_t {
//==== 对象接口 =================================================================
public:
	ShmBuffer_t( ShmBuffer_t&& ) = delete;
	ShmBuffer_t( const ShmBuffer_t& ) = delete;
	ShmBuffer_t& operator=( ShmBuffer_t&& ) = delete;
	ShmBuffer_t& operator=( const ShmBuffer_t& ) = delete;
	ShmBuffer_t() = default;
	~ShmBuffer_t();

	//---- 公开例程 ---------------------------------------------
	// 创建底层SHM, 返回真实创建的字节数
	size_t make( const str_t& name, size_t bytes );

	// 对接底层SHM, 返回真实的字节数
	size_t plug( const str_t& name, bool writable );

	void unplug( bool remove_shm_file = false );

	const str_t&	name()	const { return _shm_n; };
	size_t			bytes()	const { return _bytes; };
	void*			get()	const { return _shm_p; };

//==== 内部实现 =================================================================
private:
	//---- 内部例程 ---------------------------------------------

	//---- 内部变量 ---------------------------------------------
	str_t	_shm_n {};
	void*	_shm_p {};
	size_t	_bytes {};
};

};	// namespace leon_ext

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;

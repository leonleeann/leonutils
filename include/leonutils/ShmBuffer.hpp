#pragma once

using str_t = std::string;

namespace leon_utl {

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
	size_t make( const str_t& name, size_t bytes, bool writable );

	// 对接底层SHM, 返回真实的字节数
	size_t plug( const str_t& name, bool writable );

	void unplug( bool remove_shm_file = false );

	const str_t&	name()		const { return _shm_n; };
	size_t			bytes()		const { return _bytes; };
	void*			get()		const { return _shm_p; };
	ssize_t			pageSize()	const { return _1page; };
	ssize_t			swapout()	const;

	// 可以提前删除底层OS文件, 以免/dev/shm下文件太多, 被 OS 限制了
	str_t			osFile()	const { return "/dev/shm/" + _shm_n; };
	void			delOsFile()	const;

//==== 内部实现 =================================================================
private:
	//---- 内部例程 ---------------------------------------------

	//---- 内部变量 ---------------------------------------------
	str_t	_shm_n {};
	void*	_shm_p {};
	size_t	_bytes {};
	ssize_t	_1page {};	// 单个页面大小(字节)
};

};	// namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;

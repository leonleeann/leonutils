#pragma once

using char_cp = char const*;
using str_t = std::string;	using str_cp = const str_t*; using str_cr = const str_t&;

namespace leon_utl {

/*******************************************************************************
一个 SharedBuffer 封装一块共享内存,将申请、释放等琐碎操作集中起来.
主要用于:
	a.Tick行情共享
	b.日线行情共享
	c.跨进程原子变量、消息队列
*******************************************************************************/

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
	// 强制创建SHM(先无脑删除原有再创建), 返回真实创建的字节数
	size_t	make( str_cr name, size_t bytes, bool writable, bool log = false );

	// 对接底层SHM, 返回真实的字节数
	size_t	plug( str_cr name, bool writable, bool log = false );

	// 创建或对接SHM(存在就对接, 否则才创建), 返回SHM真实字节数
	size_t	makeOrPlug( str_cr name, size_t bytes, bool writable, bool log = false );

	void	unplug( bool remove_shm_file = false, bool log = false );

	str_cr	name()		const { return _shm_n; };
	size_t	bytes()		const { return _bytes; };
	void*	get()		const { return _shm_p; };
	int		pages()		const { return _pages; };
	int		pageSize()	const { return _1page; };

	/* 在小内存平台上分配大 SHM, 可能需要用到交换空间. 但 OS 不会主动换出 SHM 到交换空间.
	所以需要如下两个操作:
		1. 每个进程都来标记冷数据(MADV_DONTNEED), 让冷页不再有PTE(Page Table Entry);
		2. 每个进程都主动要求换出(MADV_PAGEOUT), 才可能得到换出.
	当然还需要 OS 其它配置, 比如 vm.swappiness, memory.reclaim 配合完成.
	*/
	// 标记冷数据(系统调用昂贵, 本对象会记录上次标记范围, 如果新增热数据太少就跳过)
	void	tagcold(
		const void*	cool_before,	// 将此地址之前的页标记为冷数据
		int			skip_header = 1	// 不标记最初几页(保持热度)
	);
	// 显示要求换出(必须要标记过的冷页)
	ssize_t	swapout(
		int beg_page,		// 从第几页开始(跳过几页.	负数表示:从倒数第几页开始)
		int end_page = -1	// 到第几页(不含本页.		负数表示:至倒数第几页,0表示全部)
	);

	// 可以提前删除底层OS文件, 以免/dev/shm下文件太多, 被 OS 限制了
	str_t	osFile()	const { return "/dev/shm/" + _shm_n; };
	void	delOsFile()	const;

//==== 内部实现 =================================================================
private:
	//---- 内部例程 ---------------------------------------------
	void* _mk_plug( str_cr name, size_t bytes, bool resize,
					int file_mask, mode_t user_mask, int memory_mask );
	ssize_t _madvise(
		char_cp	func_name,
		int		directive,		// MADV_PAGEOUT, MADV_DONTNEED
		int		beg_page,		// 从第几页开始(跳过几页.	负数表示:从倒数第几页开始)
		int		end_page = -1	// 到第几页(不含本页.		负数表示:至倒数第几页,0表示全部)
	);

	//---- 内部变量 ---------------------------------------------
	str_t		_shm_n {};	// 名字
	void*		_shm_p {};	// 基址
	size_t		_bytes {};	// 大小
	int			_coolp {};	// 上次标记为冷的页序号, 用于再次标记时计算新增冷页数
	int			_pages {};	// 共占多少页
	int			_1page {};	// 单页大小(字节)
};

};	// namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;

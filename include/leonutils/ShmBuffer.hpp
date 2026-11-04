#pragma once
#include <string>

using char_cp = char const*;
using str_t = std::string;	using str_cp = const str_t*; using str_cr = const str_t&;

namespace leon_utl {

/*******************************************************************************
一个 ShmBuffer 封装一块共享内存, 将申请、释放等琐碎操作集中起来.
主要用于:
	a.Tick行情共享
	b.日线行情共享
	c.跨进程原子变量、消息队列
注意:
	1. 适配 Linux-6.1 以上;
	2. 本对象自己不解决多线程同步问题!!!
*******************************************************************************/

class ShmBuffer_t {
//==== 对象接口 =================================================================
public:
	ShmBuffer_t( ShmBuffer_t&& ) = delete;
	ShmBuffer_t( const ShmBuffer_t& ) = delete;
	ShmBuffer_t& operator=( ShmBuffer_t&& ) = delete;
	ShmBuffer_t& operator=( const ShmBuffer_t& ) = delete;
	ShmBuffer_t();
	~ShmBuffer_t();

	str_cr	name()		const;
	size_t	bytes()		const;
	void*	get()		const;
	size_t	pages()		const;
	size_t	pageSize()	const;

//==== 创建/销毁 操作 ============================================================
	// 强制创建SHM(先无脑删除原有再创建), 返回真实创建的字节数
	size_t	make( str_cr name, size_t bytes, bool writable, bool log = false );

	// 对接底层SHM, 返回真实的字节数
	size_t	plug( str_cr name, bool writable, bool log = false );

	// 创建或对接SHM(存在就对接, 否则才创建), 返回SHM真实字节数
	size_t	makeOrPlug( str_cr name, size_t bytes, bool writable, bool log = false );

	void	unplug( bool remove_shm_file = false, bool log = false );

	// 可以提前删除底层OS文件, 以免/dev/shm下文件太多, 被 OS 限制了
	str_t	osFile()	const;
	void	delOsFile()	const;

//==== 换入/换出 操作 ============================================================
	// 整块换入
	void	swapIn();

	/* 在小内存平台上分配大 SHM, 可能需要用到交换空间. 但 OS 不会主动换出 SHM 到交换空间.
	所以需要如下两个操作:
		1. 每个进程都来标记冷数据(MADV_DONTNEED), 让冷页不再有PTE(Page Table Entry);
		2. 每个进程都主动要求换出(MADV_PAGEOUT), 才可能得到换出.
	当然还需要 OS 其它配置, 比如 vm.swappiness, memory.reclaim 配合完成.
	*/
	// 标记冷数据(系统调用昂贵, 本对象会记录上次标记范围, 如果新增热数据太少就跳过)
	size_t	tagCold(
		const void*	cool_before,	// 将此地址之前的页标冷
		size_t		skip_header,	// 不标记最初几页(保持热度)
		bool		force			// 强制标冷, 不管已标记过的范围, 注意频繁调用比较耗时!
	);
	size_t	tagPage(
		int64_t		beg_page,		// 从第几页开始(跳过几页.	负数表示:从倒数第几页开始)
		int64_t		end_page,		// 到第几页(不含本页.		负数表示:至倒数第几页,0表示全部)
		bool		force			// 强制标冷, 不管已标记过的范围, 注意频繁调用比较耗时!
	);

	// 显示要求换出(只能是标记过的冷页. 这也只是确保换出条件,实际能否换出只能OS决定)
	size_t	swapOut(
		const void*	swap_before,	// 将此地址之前的页换出
		size_t		skip_header,	// 不换出最初几页(保持驻留)
		bool		force			// 强制换出, 不管已换出过的范围, 注意频繁调用比较耗时!
	);
	size_t	pageOut(
		int64_t		beg_page,		// 从第几页开始(跳过几页.	负数表示:从倒数第几页开始)
		int64_t		end_page,		// 到第几页(不含本页.		负数表示:至倒数第几页,0表示全部)
		bool		force			// 强制换出, 不管已换出过的范围, 注意频繁调用比较耗时!
	);
	// 清除上述两个(标冷,换出)范围记忆
	void	clearTagRanges();

	/* 将整块 SHM 标记为巨页
		linux-6.1 才有 MADV_COLLAPSE, 编辑本文件时所用系统库(头文件)尚不支持. 别删, 升级gcc后会改回来
		force:	false(MADV_HUGEPAGE):异步,暗示性;
				true (MADV_COLLAPSE):同步,明确要求,并且会导致全部换入. */
	void	tagHuge();

//==== 内部实现 =================================================================
private:
	struct Imp_t;
	Imp_t* _imp;
};

};	// namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;

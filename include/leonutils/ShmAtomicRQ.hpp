#pragma once
#include <atomic>
#include <bit>

#include "leonutils/MemoryOrder.hpp"
#include "leonutils/ShmBuffer.hpp"

namespace leon_utl {

/* 一个基于shm共享内存上原子变量,可跨进程使用的无锁循环队列
	特点:
		1.循环队列,整个生命周期不会追加/释放内存
		2.无锁
		3.多生产者/多消费者(MPMC)
		4.基于共享内存,可多进程使用,也可多线程使用
		5.线程安全,一个本对象可为多个线程使用

	注意:
		1.载荷数据必须是 POD,因为跨进程. 构造器/析构器也没有意义!
		2.载荷数据大小必须固定128字节, 因为本对象不能使用变长载荷!
*/
class ShmAtmRQ_t final {
//====== 类接口 =================================================================
public:
	friend struct TestShmAtmRQ_F;

	using SIZE_TYPE = int64_t;
	using ATOM_SIZE = std::atomic_int64_t;

//====== 内部结构 ===============================================================
	struct alignas( 64 ) Node_t {
		// SIZE_TYPE不论用int32还是int64, gcc都会分配64位, 干脆都用64位
		ATOM_SIZE	h_tag;

		// 强制112字节, 把 t_tag 挤到下一个 cache line
		char		goods[ 128 - sizeof( ATOM_SIZE ) * 2 ];

		// 确保它和 h_tag 分别在不同的 cache line
		ATOM_SIZE	t_tag;
	};
	static_assert( sizeof( Node_t ) == 128 );
	static constexpr SIZE_TYPE LOAD_SIZE = sizeof( Node_t::goods );
	static_assert( LOAD_SIZE == 112 );

	// shm 内存首部的控制区, 主要是队首、 队尾变量, 以及容量
	struct Meta_t {
		alignas( 64 ) ATOM_SIZE	_head;	// 消费者修改
		SIZE_TYPE const			_capa;	// 只应由创建者初始化
		SIZE_TYPE const			_mask;	// 只应由创建者初始化

		// 本对象的任何一个使用者都可以设置的标志,其它使用者都能看到,包括跨进程
		ai64_t					_flag;
		alignas( 64 ) ATOM_SIZE	_tail;	// 生产者修改
	};

//====== 容量限制 ===============================================================
	// 最大共享内存占用(单位:字节)
	static constexpr SIZE_TYPE MAX_SHM_USAGE = 0x40000000;	// 1G Bytes
	// 最少容量(单位:元素个数)
	static constexpr SIZE_TYPE LEAST_ELEMNTS = 4;
	// 最多容量(单位:元素个数,必须是2的整数次幂, 因为_mask必须是全1)
#if( __GNUC__ >= 10 )
	static constexpr SIZE_TYPE MOST_ELEMENTS =
		std::bit_floor( MAX_SHM_USAGE / sizeof( Node_t ) );
#else
	static constexpr SIZE_TYPE MOST_ELEMENTS =
		std::floor2( MAX_SHM_USAGE / sizeof( Node_t ) );
#endif

//====== 对象接口 ===============================================================
	ShmAtmRQ_t( ShmAtmRQ_t&& ) = delete;
	ShmAtmRQ_t( const ShmAtmRQ_t& ) = delete;
	ShmAtmRQ_t& operator=( ShmAtmRQ_t&& ) = delete;
	ShmAtmRQ_t& operator=( const ShmAtmRQ_t& ) = delete;
	explicit ShmAtmRQ_t() = default;
	~ShmAtmRQ_t();

	// 作为"拥有者"创建本对象,删除重建底层SHM,并且设定容量、初始化公共部分
	void make( SIZE_TYPE capacity, const str_t& name );

	// 作为"访问者"创建本对象,只是对接底层SHM,并根据公共部分设定自身规格
	void plug( const str_t& name );

	// 所有生产者、消费者都对接好之后,可以调用本函数删除对应的底层OS文件,以免/dev/shm下文件太多
	str_t	osFile() const { return _buff.osFile(); };
	void delOsFile() const { _buff.delOsFile(); };

	const str_t& name() const { return _buff.name(); };
	SIZE_TYPE	capa() const { return _capa; };
	SIZE_TYPE	head() const { return _meta->_head.load( mo_acquire ); };
	SIZE_TYPE	tail() const { return _meta->_tail.load( mo_acquire ); };
	SIZE_TYPE	size() const {
		SIZE_TYPE tail = _meta->_tail.load( mo_relaxed );
		SIZE_TYPE head = _meta->_head.load( mo_acquire );
		return tail - head;
	};
	int64_t		err_cnt() const { return _errs.load( mo_acquire ); };
	bool		full() const {
		SIZE_TYPE tail = _meta->_tail.load( mo_relaxed );
		SIZE_TYPE head = _meta->_head.load( mo_acquire );
		return tail - head >= _capa;
	};
	bool		empty() const {
		SIZE_TYPE tail = _meta->_tail.load( mo_relaxed );
		SIZE_TYPE head = _meta->_head.load( mo_acquire );
		return tail == head;
	};

	// 因为跨进程要求 T 必须是POD的,所以这里是"移入"还是"复制"都无所谓了
	bool		enque( const void* );
	bool		deque( void* );
	void		clear();
	// fork之后的父子进程都是所有者, 得让一边有办法放弃所有权
	void		releaseOwnership() { const_cast<bool&>( _ownr ) = false; };

	// 附赠的flag,便于跨进程简单沟通.本对象行为与此完全无关,只是用浪费的shm空间提供个小玩意
	int64_t		get_flag() const;		// 查看 SHM 上的数值
	void		set_flag( int64_t );	// 将一个数值设置到 SHM,供其它进程查看

//====== 内部实现 ===============================================================
private:
	// 让容量刚好是2的整数次幂, 因为_mask必须是全1
	static SIZE_TYPE _AlignCapa( SIZE_TYPE want_capa );

	// 为了方便测试,把这几个属性也暴露出来
	uintptr_t	_base_addr() const { return reinterpret_cast<uintptr_t>( _base ); };
	uintptr_t	_head_node() const { return reinterpret_cast<uintptr_t>( _base + _meta->_head.load() ); };
	uintptr_t	_tail_node() const { return reinterpret_cast<uintptr_t>( _base + _meta->_tail.load() ); };
	SIZE_TYPE	_ring_mask() const { return _mask; };

	//---- 成员变量 -------------
	Meta_t* const	_meta {};	// 控制区,同样的物理内存,在不同进程有不同地址值
	Node_t* const	_base {};	// 数据区,同样的物理内存,在不同进程有不同地址值
	SIZE_TYPE const	_capa {};	// 和_meta内部属性相同,复制到外面简化编码
	SIZE_TYPE const	_mask {};	// 和_meta内部属性相同,复制到外面简化编码
	ShmBuffer_t		_buff {};
	ai64_t			_errs {};	// 本对象操作(出入队)失败计数, 成功一次会清除
	bool const		_ownr {};	// 为true就是拥有者,否则就是访问者
};

};	// namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;

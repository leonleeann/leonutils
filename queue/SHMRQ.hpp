#pragma once
#include <string>

/* 一个基于共享内存可跨进程使用的无锁环状队列
 * 特点:
 *   1.无锁
 *   2.环状队列,整个生命周期不会追加/释放内存
 *   3.多生产者/多消费者(MPMC)
 *   4.不论多少个生产者、消费者,本对象的拥有者(owner)只能有一个
 *   5.基于共享内存,可多进程使用,也可多线程使用
 *   6.曾经使用无符号的 size_t 作为容量类型是 STL 的历史错误,所以改用 int64_t
 */
namespace leon_utl {

template <typename T>
class SHMRQ_t {
	friend struct TestSHMRQ_WITH_ELEMENT_F;

public:
	// 最大内存占用(单位:字节)
	static constexpr int64_t MAX_MEM_USAGE = 0x40000000;  // 1G Bytes

	// 最少容量(单位:元素个数)
	static constexpr int64_t LEAST_ELEMNTS = 4;

	// 由consumer来构造,会创建共享内存,反之producer只会访问
	explicit SHMRQ_t( const std::string& shm_name, int64_t capacity, bool owning );
	~SHMRQ_t();

	const std::string& name() const;

	// 队列容量(单位:元素个数)
	int64_t capa() const;

	// 当前占用(单位:元素个数)
	int64_t size() const;

	// 复制入队
	// bool enque( const T& data );
	// 移动入队
	// bool enque( T&& data );
	// 通用入队(不再区分复制还是移动)
	template<typename U>
	bool enque( U&& data );

	// 移动出队
	bool deque( T& data );

protected:
	struct Imp_t;
	Imp_t* _imp;
};

};  // namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;

#pragma once

using str_t = std::string;

namespace leon_utl {

class HugePage_t {
//==== 对象接口 =================================================================
public:
	HugePage_t( HugePage_t&& ) = delete;
	HugePage_t( const HugePage_t& ) = delete;
	HugePage_t& operator=( HugePage_t&& ) = delete;
	HugePage_t& operator=( const HugePage_t& ) = delete;
	HugePage_t() = default;
	~HugePage_t();

	//---- 公开例程 ---------------------------------------------
	size_t	make( size_t bytes );
	void	release();
	size_t	bytes()		const { return _bytes; };
	void*	get()		const { return _shm_p; };
	ssize_t	pageSize()	const { return _1page; };

//==== 内部实现 =================================================================
private:
	//---- 内部例程 ---------------------------------------------

	//---- 内部变量 ---------------------------------------------
	void*	_shm_p {};
	size_t	_bytes {};
	ssize_t	_1page {};
};

};	// namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;

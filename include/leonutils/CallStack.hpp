#pragma once
#include <array>
#include <iosfwd>

using ost_t = std::ostream;

namespace leon_utl {

/* 多线程程序难于跟踪调试,尤其是高频交易,需要一种机制能够在实盘运行遇到崩溃时输出调用栈

Tracer_t 和 tl_call_stack 配合完成这一任务:
	进入一个函数时, 创建一个RAII的 Tracer_t 对象, 任何时候离开函数, 该对象会自动释放;
	在 Tracer_t 的构造器内会将函数名加入 tl_call_stack;
	在析构器内加入退出函数的标记;
*/
class Tracer_t {
public:
	static void ClearCallStack();
	static void PrintCallStack( ost_t& );

	explicit Tracer_t( const char* trace_point_name );
	~Tracer_t();

private:
	const char* _name;
};

#define TRACE_POINT Tracer_t _tracer { __func__ }

/*
#if defined DEBUG || defined APP_ROLE_IS_TD
#define TRACE_POINT Tracer_t _tracer { __func__ }
#else
#define TRACE_POINT {}
#endif
*/

// 0.3.13 运行发现动态加载的模块(*.so)未能正确入栈, 试试把 tl_call_stack 暴露成 extern
constexpr int		STACK_CAPA = 64;
constexpr int		STACK_MASK = 63;
extern thread_local std::array<const char*, STACK_CAPA> tl_call_stack;

}; //namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;

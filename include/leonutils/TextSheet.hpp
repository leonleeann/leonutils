#pragma once
#include <string>

namespace leon_utl {

using str_t = std::string;
using wstr_t = std::wstring;

// 将数据制作成字符串表格,可供终端文本输出
class TextSheet_t {
public:
	TextSheet_t( const str_t&	sheet_name,		// 表格名称
				 int			data_col_width,	// 数据列的宽度
				 int			data_precision	// 格式化小数位数(仅对浮点数有意义)
			   );
	~TextSheet_t();

//---- 初始化阶段 -------------------------------------
	// 1.一定要先清除原有定义
	void	clear();
	// 2.添加新列,返回总列数(可以做多次)
	int		addCol( const str_t& col_name );
	// 3.添加新行,返回总行数(可以做多次)
	int		addRow( const str_t& row_name );
	// 4.重新生成表头常量(完成初始化)
	void	skeleton();

//---- 使用阶段 -------------------------------------
	// 5.填入数据
	/*
	void	fill( int row, int col, double );
	void	fill( int row, int col, int64_t, int is_int_overload );
	void	fill( int row, int col, const str_t& ); */
	template<typename T>
	void	fill( int row, int col, T );
	void	fillStr( int row, int col, const str_t& );

	// 6.得到表格
	wstr_t	makeWstr() const;
	str_t	makeUtf8() const;

	// 返回总列数
	int cols() const;
	// 返回总行数
	int rows() const;

private:
	struct	Imp_t;
	Imp_t*	_imp;
};

}; //namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;

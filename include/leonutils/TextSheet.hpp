#pragma once
#include <string>

namespace leon_utl {

using str_t = std::string;
using wstr_t = std::wstring;

// 将数据制作成字符串表格,可供终端文本输出
class TextSheet_t {
public:
	TextSheet_t( const str_t& sheet_name );
	~TextSheet_t();

//---- 初始化阶段 -------------------------------------
	// 1.清除原有定义
	void	clear();

	// 2.添加新列,返回总列数(可以做多次)
	int		addCol( const str_t&	col_name,		// 列名
					int8_t			width,			// 列宽
					int8_t			precision,		// 显示精度
					int8_t			group = 0,		// 数字分组
					char			sep_chr = ',' );// 分组字符

	// 3.添加新行,返回总行数(可以做多次)
	int		addRow( const str_t& row_name );

	// 4.重新生成表头常量(完成初始化)
	void	skeleton();

//---- 使用阶段 -------------------------------------
	// 5.填入数据(既然是文本输出,就不必纠结性能了,全部都用浮点数)
	void	fill( int row, int col, double );

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

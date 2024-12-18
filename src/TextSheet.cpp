#include <iomanip>
#include <iostream>
#include <leonutils/Converts.hpp>
#include <leonutils/TextSheet.hpp>
#include <leonutils/Unicodes.hpp>
#include <sstream>
#include <vector>

template<typename T, typename A = std::allocator<T> >
using vct_t = std::vector<T, A>;

namespace leon_utl {

using woss_t = std::wostringstream;
using WsNames_t = vct_t<wstr_t>;

using RowData_t = vct_t<double>;
using AllData_t = vct_t<RowData_t>;

// 单列数据显示规格
struct ColSpec_t {
	wstr_t	_n;		// 列名
	int8_t	_w;		// 列宽
	int8_t	_p;		// 小数位数
	int8_t	_g;		// 分组位数
	char	_s;		// 组分隔符
	bool	_v;		// 要否显示
};
// 全部列定义
using Columns_t = vct_t<ColSpec_t>;

struct TextSheet_t::Imp_t {
	// 表格名称
	const wstr_t	_sheet_name;
	// 首列(名称列)宽度,自动找
	int				_name_width { 24 };

	// 所有数据行名称(Unicode)
	WsNames_t		_row_names;

	// 所有数据列定义(名称、显示规格)
	Columns_t		_col_specs;

	/* 表头,例如:
		┌─────────┬─────┬─────┬──────┐
		│ 抛弃原因 │ 总数 │ 大连 │ 上期 │ */
	wstr_t			_head_line;

	/* 行分割,例如:
		├────────┼──────┼─────┼─────┤ */
	wstr_t			_sepa_line;

	/* 表底,例如:
		└────────┴──────┴─────┴─────┘ */
	wstr_t			_foot_line;

	// 实际填入的数据
	AllData_t		_all_data;

//------------------------------------------------------
	void	skeleton();
	str_t	textOf( double, const ColSpec_t& ) const;
	wstr_t	make() const;
};

void TextSheet_t::Imp_t::skeleton() {
//---- 确定首列列宽 ----------------------------------------------
	// 在行名中找到最大宽度,作为首列(名称列)宽度
	_name_width = displ_width( _sheet_name );
	for( const auto& row_name : _row_names )
		_name_width = std::max( _name_width, displ_width( row_name ) );
	// 前后各留一个空格宽度
	_name_width += 2;
	// 把行名都适配到首列列宽,避免运行中重复算
	for( auto& row_name : _row_names )
		row_name = adapt_width( _name_width, row_name );

//---- 制作名称列 ------------------------------------------------
	// 名称列用横线
	wstr_t nc_line = wstr_t( _name_width, L'─' );
	wstr_t first_r;
	_head_line = L"\n┌" + nc_line;
	_sepa_line = L"\n├" + nc_line;
	_foot_line = L"\n└" + nc_line;
	first_r = L"\n│" + adapt_centr( _name_width, _sheet_name );

//---- 制作数据列 ------------------------------------------------
	for( const auto& col_spec : _col_specs )
		if( col_spec._v ) {
			// 数据列用横线
			wstr_t dc_line = wstr_t( col_spec._w, L'─' );
			_head_line += L'┬' + dc_line;
			_sepa_line += L'┼' + dc_line;
			_foot_line += L'┴' + dc_line;
			first_r += L'│' + adapt_centr( col_spec._w, col_spec._n );
		}

//---- 各header收口,组装成大header -------------------------------
	_head_line += L'┐';
	_sepa_line += L'┤';
	_foot_line += L'┘';
	first_r += L'│';
	_head_line += first_r;

//---- 初始化数据 ----------------------------------------------
	_all_data.clear();
	for( size_t r = 0; r < _row_names.size(); ++r ) {
		auto& row = _all_data.emplace_back();
		for( size_t c = 0; c < _col_specs.size(); ++c )
			row.emplace_back( int64_t{ 0 } );
		row.shrink_to_fit();
	};
	_all_data.shrink_to_fit();
};

str_t TextSheet_t::Imp_t::textOf( double data_, const ColSpec_t& spec_ ) const {

	return format( data_, spec_._w - 2, spec_._p, spec_._g, ' ', spec_._s );
};

wstr_t TextSheet_t::Imp_t::make() const {
	woss_t wos;
	wos << _head_line;
	int r = 0;
	for( const auto& row_name : _row_names ) {
		wos << _sepa_line << L"\n│" << row_name;
		auto& row = _all_data[r++];
		int c = 0;
		for( const auto& d : row ) {
			const auto& spec = _col_specs[c++];
			if( spec._v )
				wos << L'│' << adapt_width( spec._w, textOf( d, spec ) );
		}
		wos << L'│';
	}
	wos << _foot_line;
	return wos.str();
};

TextSheet_t::TextSheet_t( const str_t& name_ ) {
	_imp = new Imp_t { u8_2_ws( name_ ) };
};

TextSheet_t::~TextSheet_t() {
	delete _imp;
};

void TextSheet_t::clear() {
//	_imp->_sheet_name.clear();
	_imp->_row_names.clear();
	_imp->_col_specs.clear();
	_imp->_head_line.clear();
	_imp->_sepa_line.clear();
	_imp->_foot_line.clear();
	_imp->_all_data.clear();
};

int TextSheet_t::addCol( const str_t& n_, int8_t w_, int8_t p_,
						 int8_t g_, char s_, bool v_ ) {
	_imp->_col_specs.emplace_back( u8_2_ws( n_ ), w_, p_, g_, s_, v_ );
	return _imp->_col_specs.size();
};

int TextSheet_t::addRow( const str_t& name_ ) {
	_imp->_row_names.push_back( u8_2_ws( name_ ) );
	return _imp->_row_names.size();
};

void TextSheet_t::skeleton() {
	_imp->skeleton();
};

int TextSheet_t::cols() const {
	return _imp->_col_specs.size();
};

int TextSheet_t::rows() const {
	return _imp->_row_names.size();
};

void TextSheet_t::fill( int r_, int c_, double d_ ) {
	_imp->_all_data[r_][c_] = d_;
};

wstr_t TextSheet_t::makeWstr() const {
	return _imp->make();
};

str_t TextSheet_t::makeUtf8() const {
	return ws_2_u8( _imp->make() );
};

}; //namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;

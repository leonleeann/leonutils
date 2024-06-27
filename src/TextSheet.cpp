#include <sstream>
#include <variant>
#include <vector>
#include <leonutils/Converts.hpp>
#include <leonutils/TextSheet.hpp>
#include <leonutils/Unicodes.hpp>

namespace leon_utl {

using woss_t = std::wostringstream;
using U8Names_t = std::vector<str_t>;
using WsNames_t = std::vector<wstr_t>;

using Data_t = std::variant<int64_t, double, str_t>;
using RowData_t = std::vector<Data_t>;
using AllData_t = std::vector<RowData_t>;

struct TextSheet_t::Imp_t {
	// UTF8的行名、列名
	U8Names_t	_u8_cns;
	U8Names_t	_u8_rns;
	// Unicode行名、列名
	WsNames_t	_ws_cns;
	WsNames_t	_ws_rns;

	// 实际填入的数据
	AllData_t	_all_da;
	str_t		_myname;

	/* 表头,例如:
		┌─────────┬─────┬─────┬──────┐
		│ 抛弃原因 │ 总数 │ 大连 │ 上期 │ */
	wstr_t		_header;

	/* 行分割,例如:
		├────────┼──────┼─────┼─────┤ */
	wstr_t		_separa;

	/* 表底,例如:
		└────────┴──────┴─────┴─────┘ */
	wstr_t		_footer;

	// 首列(名称列)宽度,自动找
	int			_name_w { 24 };
	// 数据列宽
	int			_data_w;
	// 浮点数输出所需小数位数
	int			_precis;

//------------------------------------------------------
	void	skeleton();
	str_t	textOf( const Data_t& ) const;
	wstr_t	make() const;
};

void TextSheet_t::Imp_t::skeleton() {
//---- 确定首列列宽 ----------------------------------------------
	// 在行名中找到最大宽度,作为首列宽度
	_name_w = displ_width( _myname );
	_ws_rns.clear();
	for( const auto& n : _u8_rns ) {
		wstr_t& wrn = _ws_rns.emplace_back( u8_2_ws( n ) );
		_name_w = std::max( _name_w, displ_width( wrn ) );
	}
	_name_w += 2;
	// 把行名都适配到首列列宽,避免运行中重复算
	for( auto& w_rn : _ws_rns )
		w_rn = adapt_width( _name_w, w_rn );

//---- 制作每行内容 ----------------------------------------------
	// 名称列用横线
	wstr_t nc_line = wstr_t( _name_w, L'─' );
	// 数据列用横线
	wstr_t dc_line = wstr_t( _data_w, L'─' );
	wstr_t first_r;
	_header = L"\n┌" + nc_line;
	_separa = L"\n├" + nc_line;
	_footer = L"\n└" + nc_line;
	first_r = L"\n│" + adapt_centr( _name_w, _myname );

	// 根据每个数据列名,附加到各个header后面
	_ws_cns.clear();
	for( const auto& n : _u8_cns ) {
		wstr_t& wcn = _ws_cns.emplace_back( u8_2_ws( n ) );
		_header += L'┬' + dc_line;
		_separa += L'┼' + dc_line;
		_footer += L'┴' + dc_line;
		first_r += L'│' + adapt_centr( _data_w, wcn );
	}

//---- 各header收口,组装成大header ----------------------------------------------
	_header += L'┐';
	_separa += L'┤';
	_footer += L'┘';
	first_r += L'│';
	_header += first_r;

//---- 初始化数据 ----------------------------------------------
	_all_da.clear();
	for( size_t r = 0; r < _u8_rns.size(); ++r ) {
		auto& row = _all_da.emplace_back();
		for( size_t c = 0; c < _u8_cns.size(); ++c ) {
			Data_t d = int64_t{0};
			row.push_back( d );
		}
		row.shrink_to_fit();
	};
	_all_da.shrink_to_fit();
};

str_t TextSheet_t::Imp_t::textOf( const Data_t& data_ ) const {

	int neat_w = _data_w - 2;

	switch( data_.index() ) {
		case 0:	// int64_t
			return format( std::get<0>( data_ ), neat_w, 0, 4 );

		case 1:	// double
			return format( std::get<1>( data_ ), neat_w, _precis, 3, ' ', ',' );

		case 2:	// string
			return std::get<2>( data_ );

		default:;
	};

	return {};
};

wstr_t TextSheet_t::Imp_t::make() const {
	woss_t wos;
	wos << _header;
	int r = 0;
	for( const auto& rn : _ws_rns ) {
		wos << _separa << L"\n│" << rn;
		auto& row = _all_da[r++];
		for( const auto& data : row )
			wos << L'│' << adapt_width( _data_w, textOf( data ) );
		wos << L'│';
	}
	wos << _footer;
	return wos.str();
};

TextSheet_t::TextSheet_t( const str_t& name_, int width_, int prec_ ) {
	_imp = new Imp_t;
	_imp->_myname = name_;
	_imp->_data_w = width_;
	_imp->_precis = prec_;
};

TextSheet_t::~TextSheet_t() {
	delete _imp;
};

void TextSheet_t::clear() {
	_imp->_u8_cns.clear();
	_imp->_u8_rns.clear();
	_imp->_ws_cns.clear();
	_imp->_ws_rns.clear();
	_imp->_all_da.clear();
	_imp->_header.clear();
	_imp->_separa.clear();
	_imp->_footer.clear();
};

int TextSheet_t::addCol( const str_t& name_ ) {
	_imp->_u8_cns.push_back( name_ );
	return _imp->_u8_cns.size();
};

int TextSheet_t::addRow( const str_t& name_ ) {
	_imp->_u8_rns.push_back( name_ );
	return _imp->_u8_rns.size();
};

void TextSheet_t::skeleton() {
	_imp->skeleton();
};

int TextSheet_t::cols() const {
	return _imp->_u8_cns.size();
};

int TextSheet_t::rows() const {
	return _imp->_u8_rns.size();
};

template<typename T>
void TextSheet_t::fill( int r_, int c_, T d_ ) {
	_imp->_all_da[r_][c_] = d_;
};

// 特化版
template<> void TextSheet_t::fill<>( int r_, int c_, float	d_ ) { _imp->_all_da[r_][c_] = static_cast<double>( d_ ); };
template<> void TextSheet_t::fill<>( int r_, int c_, int	d_ ) { _imp->_all_da[r_][c_] = static_cast<int64_t>( d_ ); };
// 实例化
template void TextSheet_t::fill<double>( int, int, double );
template void TextSheet_t::fill<float>( int, int, float );
template void TextSheet_t::fill<int64_t>( int, int, int64_t );
template void TextSheet_t::fill<int>( int, int, int );

void TextSheet_t::fillStr( int r_, int c_, const str_t& s_ ) { _imp->_all_da[r_][c_] = s_; };

wstr_t TextSheet_t::makeWstr() const {
	return _imp->make();
};

str_t TextSheet_t::makeUtf8() const {
	return ws_2_u8( _imp->make() );
};

}; //namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;

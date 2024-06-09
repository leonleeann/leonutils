#include <LeonLog>
#include <mqueue.h>

#include "PosixMQ.hpp"
#include "chrono/Chrono.hpp"
#include "event/Message.hpp"

using namespace leon_utl;
using namespace leon_log;
using namespace std::chrono;
using namespace std::chrono_literals;
using std::string;

namespace octopus {

string PosixMQ_t::MsgQueName( string qn_ ) {
	qn_.erase( std::remove( qn_.begin(), qn_.end(), ' ' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), '!' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), '"' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), '#' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), '$' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), '%' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), '&' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), '(' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), ')' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), '*' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), '+' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), ',' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), '-' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), '.' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), '/' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), ':' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), ';' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), '<' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), '=' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), '>' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), '?' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), '@' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), '[' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), '\'' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), '\\' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), ']' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), '^' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), '`' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), '{' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), '|' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), '}' ), qn_.end() );
	qn_.erase( std::remove( qn_.begin(), qn_.end(), '~' ), qn_.end() );
	return '/' + qn_;
};

struct PosixMQ_t::Imp_t {
	std::string	_name;

	// 订阅者自定的数据,每次随同消息一并发给订阅者
	uintptr_t	_cust = 0;

	// 最后发过的消息
	union {
		Message_t	msg;
		DQMessg_t	dqm;
	} _last { .msg = Message_t{} };

	mq_attr		_attr {};
	mqd_t		_q_no = -1;
	// 错误计数,避免大量重复日志
	int			_errs = 0;

//--------------------------------------------------------------
	Imp_t( const string& name_, uintptr_t cust_ );
	~Imp_t();
	bool	getAttrb();
	void	clearQue();
	void	sendMssg( const char* msg_, size_t len_, unsigned prio_ );
};

bool PosixMQ_t::Imp_t::getAttrb() {
	if( mq_getattr( _q_no, &_attr ) < 0 ) {
		lg_erro << "获取mq:" << _name << "的属性失败("
				<< strerror( errno ) << ")!";
		return false;
	}
	return true;
};

void PosixMQ_t::Imp_t::clearQue() {
	char buf[ _attr.mq_msgsize ];
	timespec ts;
	while( getAttrb() && _attr.mq_curmsgs > 0 ) {
		timespec_get( &ts, TIME_UTC );
		// 等待接收 1ms,然后出来看看要不要退出
		ts += 1000 * 1000;
		// 万一还有其它消费者抢先消费了最后消息,所以咱不能傻等,不能用 mq_receive
		mq_timedreceive( _q_no, buf, _attr.mq_msgsize, NULL, &ts );
	};
};

void PosixMQ_t::Imp_t::sendMssg( const char* msg_, size_t len_, unsigned prio_ ) {
	int result = mq_send( _q_no, msg_, len_, prio_ );
	if( result == 0 ) {
		// 成功发送一次就可以将出错计数归零
		_errs = 0;
		return;
	}

	// 连续出错就不写日志了
	if( ++_errs > 5 )
		return;

	// 发送错误信息要先留存,因为 getAttrb 也可能出错,得到另一个错误码
	int 	send_err = errno;
	string	real_msg = strerror( send_err );

	if( send_err == EAGAIN && getAttrb() )
		lg_erro << "发消息给mq:" << _name << "失败(" << real_msg
				<< "),可能是队列已满(已有消息:" << _attr.mq_curmsgs
				<< ",容量:" << _attr.mq_maxmsg << ").";
	else
		lg_erro << "发消息给mq:" << _name << "失败(" << real_msg << ")!";
};

PosixMQ_t::Imp_t::Imp_t( const string& name_, uintptr_t cust_ )
	: _name( name_ ), _cust( cust_ ) {

	_q_no = mq_open( name_.c_str(), O_RDWR | O_NONBLOCK );
	if( _q_no > 0 && getAttrb() )
		lg_debg << "成功打开mq:" << name_ << ",mqd[" << _q_no
				<< "],已有消息:" << _attr.mq_curmsgs
				<< ",容量:" << _attr.mq_maxmsg;
	else
		lg_erro << "打开mq:" << name_ << "失败(" << strerror( errno ) << ")!";
};

PosixMQ_t::Imp_t::~Imp_t() {
	if( mq_close( _q_no ) == 0 )
		lg_debg << "成功关闭mq:" << _name;
	else
		lg_erro << "关闭mq:" << _name << "失败(" << strerror( errno ) << ")!";
};

PosixMQ_t::PosixMQ_t( const string& name_, uintptr_t cust_ ) {
	_imp = new Imp_t( name_, cust_ );
};

PosixMQ_t::~PosixMQ_t() {
	delete _imp;
};

uintptr_t PosixMQ_t::payLoad() const {
	return _imp->_cust;
};

void PosixMQ_t::sendMsg( const Message_t& msg_, unsigned prio_ ) {
	_imp->sendMssg( reinterpret_cast<const char*>( &msg_ ),
					sizeof( Message_t ), prio_ );
};

void PosixMQ_t::sendMsg( const DQMessg_t& msg_, unsigned prio_ ) {
	_imp->sendMssg( reinterpret_cast<const char*>( &msg_ ),
					sizeof( DQMessg_t ), prio_ );
};

};  //namespace octopus

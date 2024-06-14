#pragma once
#include <string>

namespace leon_utl {

using str_t = std::string;
struct Message_t;
struct DQMessg_t;

/* PosixMQ_t 封装一个 POSIX 消息队列,主要完成:
 * 1.打开/关闭队列
 * 2.处理发消息的细节(判断队列已满 / 出错后输出日志 / 避免日志洪水)
 * 3.处理收消息的细节(操心Buf大小 / 出错后输出日志 / 超时处理)
 */
class PosixMQ_t {
public:
//====== 类接口 =====================================================================
	// 构造一个合法的mq名字,去掉其中的特殊字符
	static str_t MsgQueName( str_t org_name );

//====== 对象接口 ===================================================================
	// 以生产者的身份创建一个mq(不创建底层mq,只是去获取它)
	PosixMQ_t( const str_t& que_name, uintptr_t custom_payload );
	~PosixMQ_t();

	uintptr_t	payLoad() const;
// 	bool		getAttrb( mq_attr& );
	void		sendMsg( const Message_t&, unsigned priority );
	void		sendMsg( const DQMessg_t&, unsigned priority );

	// 收到消息返回 true
	bool		recvMsg();

//--------------------------------------------------------------
private:
	struct Imp_t;
	Imp_t* _imp;
};

};  //namespace leon_utl

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;

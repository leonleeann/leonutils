#include <LeonLog>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mqueue.h>

#include "PosixMQ.hpp"
#include "chrono/Chrono.hpp"
#include "use-case/UseCase-Quotes.hpp"

using namespace leon_utl;
using namespace leon_log;
using namespace std::chrono;
using namespace std::chrono_literals;
using std::string;

namespace octopus {

struct TestPosixMQ_F : public testing::Test {
	string          mq_name;
	mqd_t			mq_id = -1;
	mq_attr			mq_at {};
	union {
		Message_t msg;
		DQMessg_t dqm;
	} buf {};

	void createMsgQue() {
		if( mq_id > 0 )
			return;

		const auto t_info = testing::UnitTest::GetInstance()->current_test_info();
		mq_name =
			PosixMQ_t::MsgQueName( string( t_info->test_suite_name() ) + t_info->name() );
		lg_debg << "准备创建mq:" << mq_name;
		mq_at.mq_maxmsg = MSG_QUE_LENGTH;
		mq_at.mq_msgsize = MAX_MESSG_SIZE;
		mq_id = mq_open( mq_name.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &mq_at );
		if( mq_id < 0 ) {
			lg_erro << "mq_open error:" << strerror( errno );
			exit( EXIT_FAILURE );
		}
		lg_debg << "成功创建mq:" << mq_name << ",mqd:" << mq_id;
	};

	void removeMsgQue() {
		if( mq_id <= 0 )
			return;
		mq_close( mq_id );
		mq_unlink( mq_name.c_str() );
	};

	bool receiveMsg() {
		timespec ts;
		timespec_get( &ts, TIME_UTC );
		ts += 1000 * 1000;	// 等待消息1ms,然后出来看看要不要退出
		return mq_timedreceive( mq_id,
								reinterpret_cast<char*>( &buf ), sizeof( buf ),
								NULL, &ts
							  ) > 0;
	};

	bool getAttrb() {
		if( mq_getattr( mq_id, &mq_at ) < 0 ) {
			lg_erro << "获取mq属性失败(" << strerror( errno ) << ")!";
			return false;
		}
		return true;
	};

	void clearMsgQue() {
		if( ! getAttrb() || mq_at.mq_curmsgs <= 0 )
			return;

		char buf[MAX_MESSG_SIZE];
		timespec ts;
		do {
			timespec_get( &ts, TIME_UTC );
			ts += 1000 * 1000;	// 等待消息1ms,然后出来看看要不要退出
			mq_timedreceive( mq_id, buf, MAX_MESSG_SIZE, 0, &ts );
		} while( getAttrb() && mq_at.mq_curmsgs > 0 );
	};

	void SetUp() override {
		createMsgQue();
	};

	void TearDown() override {
		removeMsgQue();
	};
};

TEST_F( TestPosixMQ_F, sendMessage ) {
	clearMsgQue();

	PosixMQ_t pmq( mq_name, 123456 );
	ASSERT_EQ( pmq.payLoad(), 123456 );

	Message_t msg1 { Period_e::epTick, pmq.payLoad(), 234 };
	pmq.sendMsg( msg1, MsgPriority_e::TicksQuote );
	ASSERT_TRUE( receiveMsg() );
	ASSERT_EQ( buf.msg, msg1 );

	msg1 = Message_t( pmq.payLoad(), QuoteState_e::Aucting, AS2C_t( 0 ) );
	pmq.sendMsg( msg1, MsgPriority_e::QuoteState );
	ASSERT_TRUE( receiveMsg() );
	ASSERT_EQ( buf.msg.type, msg1.type );
	ASSERT_EQ( buf.msg.period, msg1.period );
	ASSERT_EQ( buf.msg.asPubliMsg.contract, msg1.asPubliMsg.contract );
	ASSERT_EQ( buf.msg.asPubliMsg.quo_stat, msg1.asPubliMsg.quo_stat );
	ASSERT_EQ( buf.msg.asPubliMsg.sec2clos, msg1.asPubliMsg.sec2clos );

	TickTimeUS_t tt = makeTickTime( 2022, 8, 19, 15, 15 );
	msg1 = Message_t( Message_t::Index, tt, AS2C_t( 0 ) );
	pmq.sendMsg( msg1, MsgPriority_e::TicksQuote );
	ASSERT_TRUE( receiveMsg() );
	ASSERT_EQ( buf.msg, msg1 );

	DQMessg_t dmsg = {
		.type = Message_t::Daily,
		.period = Period_e::epDaily,
		.contra = 123456,
		.dq = {},
	};
	pmq.sendMsg( dmsg, MsgPriority_e::DailyQuote );
	ASSERT_TRUE( receiveMsg() );
	ASSERT_EQ( buf.dqm.type, dmsg.type );
	ASSERT_EQ( buf.dqm.period, dmsg.period );
	ASSERT_EQ( buf.dqm.contra, dmsg.contra );
	ASSERT_TRUE( unit_test::dqEqual( buf.dqm.dq, dmsg.dq ) );
};

};  //namespace octopus

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;

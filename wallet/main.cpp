#include "base/server/wsServer.h"
#include "base/server/SqlServer.h"
#include "base/common/dbg/Dbg.h"
#include "base/common/windowsService/WindowsService.h"
#include "base/common/wsClient/WsClient.h"
#include "base/server/PraseServer.h"
#include "base/common/watchDog/WatchDog.h"
#include "base\server\DbServer.h"
#include "base/server/CoreServer.h"
#include "base/common/leveldb/Leveldb.h"
#include "base/common/timer/Timer.h"
#include "coin/server/BtcServer.h"
#include "coin/server/EthServer.h"
#include "coin/server/DepositServer.h"
#include "coin/server/EthShiftServer.h"
bool serviceInit();
void serviceUnInit();

int main(int argc, char* argv[])
{
	CWindowsService::setServiceHandle(serviceInit, serviceUnInit);

	SERVICE_TABLE_ENTRY dispatchTable[] =
	{
		{ SZAPPNAME,(LPSERVICE_MAIN_FUNCTION)CWindowsService::ServiceMain },
		{ NULL,NULL }
	};
	if (argc > 1)
	{
		if (strcmp("-install", argv[1]) == 0)
		{
			CWindowsService::installService();
		}
		else if (strcmp("-remove", argv[1]) == 0)
		{
			CWindowsService::removeService();
		}
		else if (strcmp("-debug", argv[1]) == 0)
		{
			CWindowsService::debugService(argc, argv);
		}
	}
	else
	{
		CWindowsService::startServiceCtrlDispatcher(dispatchTable);
	}
	return 0;
}
#include "base/common/tool/Tool.h"
#include "base/common/timer/Timer.h"
#include "base/common/http/HttpClient.h"

bool serviceInit()
{
	//��־��ʼ��
	LOG_INIT(spdlog::level::debug);
	LOG_INFO("server begin to start");

	//�������Ź�
	CWatchDog::bark();

	//����dump
	CDbg::init();

	CLeveldb::init();

	CHttpClient::init();

	Instance(CSqlServer)->start();

	Instance(CDepositServer)->start();

	Instance(CBtcServer)->start(1,true);

	Instance(CEthShiftServer)->start();

	Instance(CEthServer)->start(4, true);

	Instance(CPraseServer)->start(2);//���Զ��߳�

	Instance(CCoreServer)->start(4);//���Զ��̣߳�����Ҫ

	Instance(CWsServer)->start(4,true);//���Զ��̣߳�����Ҫ

	LOG_INFO("server is running");
	return true;
}

void serviceUnInit()
{
	CWatchDog::die();

	LOG_INFO("server begin to stop");
	//websocket����ֹͣ
	Instance(CWsServer)->stop();

	Instance(CCoreServer)->stop();

	Instance(CPraseServer)->stop();

	Instance(CEthServer)->stop();

	Instance(CEthShiftServer)->stop();

	Instance(CBtcServer)->stop();

	Instance(CDepositServer)->stop();

	Instance(CSqlServer)->stop();

	LOG_INFO("server is stop");

	CHttpClient::uinit();

	CLeveldb::unInit();

	//��־����ʼ��
	LOG_UNINT();

}
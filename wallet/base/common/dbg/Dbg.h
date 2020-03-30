#pragma once
class CDbg
{
public:
	CDbg();
	virtual ~CDbg();

	//加入崩溃dump文件功能 
	static void init();
};


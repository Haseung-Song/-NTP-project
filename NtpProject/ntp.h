#pragma once

#include "looping_thread.h"

class NtpServer
{
public:
	NtpServer();
	~NtpServer();
	void SetTime(long long currentTime);
	bool SetTimeZONE(int timeZone);

private:
	bool IsValidTimeZone(int timeZone);
	void SetupNtpServer();
};


class NtpClient
{
public:
	NtpClient();
	~NtpClient();
	bool SetTimeZone(int timeZone);
	void SynchronizeNtpTime();

private:
	bool IsValidTimeZone(int timeZone);
	void SetupNtpClient();
	LoopingThread _loopingThread;
};

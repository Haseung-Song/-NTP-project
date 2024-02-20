#include "ntp.h"

//������ �ð� ���̺귯��
#include <Windows.h>

// �ð� ���̺귯�� 1
#include <chrono>

// �ð� ���̺귯�� 2
#include <iomanip>

// ǥ�� ����� ���̺귯��
#include <iostream>

#pragma warning(disable:4996) // _CRT_SECURE_NO_WARNINGS ����

using namespace std;

using namespace chrono;

// NtpServer() ������
NtpServer::NtpServer()
{
	SetupNtpServer();
}

// NtpServer() �Ҹ���
NtpServer::~NtpServer()
{

}

// ������ �ð��� �����ϱ� ���ؼ���

// �ڵ� ���� �� [������ ����] �Ӽ����� ������ ��.

// ������Ʈ �Ӽ� -> ���� �Ӽ� -> ��Ŀ -> �Ŵ��佺Ʈ ���� ->

// UAC ������� -> highestAvailable�� ����

void NtpServer::SetTime(long long currentTime)
{
	// �ý��� �ð� ���� ���� ��ɾ� ����
	microseconds microseconds(currentTime); // �߰� �ð��� microseconds�� ��ȯ

	// system_clock : �ý����� �ǽð� �ð踦 ������� �ϴ� �ð�

	//

	// ���ο� �ð� = [�ý����� ���� �ð� + �߰� �ð�]

	time_t new_time = system_clock::to_time_t(system_clock::now() + microseconds);

	// tm ����ü : ���ο� �ð��� �����ϴ� localtime �Լ��� new_t ������ ����!

	tm* new_t = localtime(&new_time);

	// SYSTEMTIME ����ü : ������ �ð� �Լ� ����

	SYSTEMTIME sys_time{}; // SYSTEMTIME ����ü ���� �� �ʱ�ȭ

	// ���ο� �ð��� ������ localtime�� ������ �ð� �Լ��� ����

	sys_time.wYear = new_t->tm_year + 1900; // years since 1900

	sys_time.wMonth = new_t->tm_mon + 1; // months since January - [0, 11]

	sys_time.wDayOfWeek = new_t->tm_wday; // days sice Sunday - [0, 6]

	sys_time.wDay = new_t->tm_mday; // day of the month - [1, 31]

	sys_time.wHour = new_t->tm_hour; // hours since midnight - [0, 23]

	sys_time.wMinute = new_t->tm_min; // minutes after the hour - [0, 59]

	sys_time.wSecond = new_t->tm_sec; // seconds after the minute - [0, 59]

	sys_time.wMilliseconds = 0; // milliseconds after the seconds - [0, 999]

	SetLocalTime(&sys_time); // ǥ�� �ð��뿡 ���ο� �ð� ����

	cout << "Setting time to: " << put_time(new_t, "%c") << '\n' << endl; // �ٲ� �ð� ���
}

bool NtpServer::SetTimeZONE(int timeZone)
{
	// �ý��� �ð� ���� ���� ��ɾ� ����

	// TIME_ZONE_INFORMATION ����ü
	//
	// ǥ�� �ð��뿡 ���� ���� ����
	TIME_ZONE_INFORMATION tzi{}; // TIME_ZONE_INFORMATION ����ü ���� �� �ʱ�ȭ

	// Bias �Լ�: �� ��ǻ���� ���� �ð� ��ȯ�� ���� [���� ���̾(��)]�� ǥ��
	//
	// ǥ�� �ð�(UTC) = ���̾(��) + ���� �ð�
	// ���̾(��) = ǥ�� �ð�(UTC) - ���� �ð�
	//
	// ���� ǥ�ؽð�(UTC) = 0(������)�̹Ƿ�,
	//
	// tzi.Bias = -timeZone; // ���̾(��) = -���� �ð�

	// 1. �ð� ������ ��ȯ
	//tzi.Bias = -(timeZone * 60); // ���̾(��) = -(���� �ð� * 60)

	// 2. �� ���� ����
	tzi.Bias = -timeZone;

	// SetTimeZONEInformation �Լ�: ǥ�� �ð�(UTC)���� ���� �ð������� ��ȯ ����
	//
	// 1. tzi ���� ���� 0�� �ƴϸ� true ��ȯ
	// 2. tzi ���� ���� 0�̸� false ��ȯ

	if (IsValidTimeZone(timeZone) && SetTimeZoneInformation(&tzi) != 0)
	{
		return true;
	}
	return false;
}

bool NtpServer::IsValidTimeZone(int timeZone) // Ÿ���� ��ȿ�� �˻� O
{
	// Ÿ���� üũ ��� ����

	// 1. Ÿ���� �ð��� -720 �̻��� ���
	// 2. Ÿ���� �ð��� +840 ������ ���
	//
	// Ÿ���� ���� = (-720) ~ (+840)���� �� [1, 2]�� ���� ��θ� ����
	if (timeZone >= -720 && timeZone <= 840)
	{
		cout << "The timeZone" << "[" << timeZone << "]" << " is now valid." << endl;

		return true;
	}
	cout << "The timeZone" << "[" << timeZone << "]" << " is not valid." << endl;

	return false;
}

void NtpServer::SetupNtpServer()
{
	// NTP ���� ���� ��ɾ� ����
	//
	// 1. ���� NTP �������� ���ϴ� NTP ������ ���� (Ex. ������ ���� IP: time.windows.com)
	int server_s1 = system("w32tm /config /syncfromflags:manual /manualpeerlist:time.windows.com /update");

	cout << " " << endl;

	// 2. Windows Time ���񽺰� �ڵ����� ���۵ǵ��� ����
	int server_s2 = system("sc config w32time start=auto");

	cout << " " << endl;

	// 3. ����� ��, Windows Time ���񽺰� �ڵ����� ���۵��� �ʴ� ������ �ذ�ǵ��� ����
	int server_s3 = system("sc triggerinfo w32time start/networkon stop/networkoff");

	cout << " " << endl;

	// 4-1. Windows Time ���� ����
	int server_s4_1 = system("net stop w32time");

	// 4-2. Windows Time ���� ����
	int server_s4_2 = system("net start w32time");

	// 5. Command ���� ���� ���

	if (server_s1 || server_s2 || server_s3 || server_s4_1 || server_s4_2 != 0)
	{
		cerr << "Error running command!" << endl;
	}

}

// NtpClient() ������
NtpClient::NtpClient()
{
	SetupNtpClient(); // NTP Ŭ���̾�Ʈ ���� ��ɾ� ����

	// �������� [���� ������] ȣ���� ���� ���
	_loopingThread.StartLoopingThread([this] // ������ ����
		{
			SynchronizeNtpTime(); // NTP ���� ����ȭ ��ɾ� ���� 3

			std::this_thread::sleep_for(std::chrono::seconds(1));

			// ������ ���
			printf("[%s:%d] thread call\n", __func__, __LINE__);

		});

	// NTP ���� ����ȭ ��ɾ� ���� 2. ���� ����: [1��] �ֱ� ����ȭ
	//SynchronizeNtpTime();

	// NTP ���� ����ȭ ��ɾ� ���� 1. ��ġ���� ��ü ����
	//SynchronizeNtpTime();
}

// NtpClient() �Ҹ���
NtpClient::~NtpClient()
{
	_loopingThread.StopLoopingThread(); // ������ ����
}

bool NtpClient::SetTimeZone(int timeZone)
{
	// �ý��� �ð� ���� ���� ��ɾ� ����

	// TIME_ZONE_INFORMATION ����ü
	//
	// ǥ�� �ð��뿡 ���� ���� ����
	TIME_ZONE_INFORMATION tzi{}; // TIME_ZONE_INFORMATION ����ü ���� �� �ʱ�ȭ

	// Bias �Լ�: �� ��ǻ���� ���� �ð� ��ȯ�� ���� [���� ���̾(��)]�� ǥ��
	//
	// ǥ�� �ð�(UTC) = ���̾(��) + ���� �ð�
	// ���̾(��) = ǥ�� �ð�(UTC) - ���� �ð�
	//
	// ���� ǥ�ؽð�(UTC) = 0(������)�̹Ƿ�,
	//
	// tzi.Bias = -timeZone; // ���̾(��) = -���� �ð�

	// 1. �ð� ������ ��ȯ
	//tzi.Bias = -(timeZone * 60); // ���̾(��) = -(���� �ð� * 60)

	// 2. �� ���� ����
	tzi.Bias = -timeZone;

	// SetTimeZONEInformation �Լ�: ǥ�� �ð�(UTC)���� ���� �ð������� ��ȯ ����
	//
	// 1. tzi ���� ���� 0�� �ƴϸ� true ��ȯ
	// 2. tzi ���� ���� 0�̸� false ��ȯ

	if (IsValidTimeZone(timeZone) && SetTimeZoneInformation(&tzi) != 0)
	{
		return true;
	}
	return false;
}

bool NtpClient::IsValidTimeZone(int timeZone) // Ÿ���� ��ȿ�� �˻� O
{
	// Ÿ���� üũ ��� ����

	// 1. Ÿ���� �ð��� -720 �̻��� ���
	// 2. Ÿ���� �ð��� +840 ������ ���
	//
	// Ÿ���� ���� = (-720) ~ (+840)���� �� [1, 2]�� ���� ��θ� ����
	if (timeZone >= -720 && timeZone <= 840)
	{
		cout << "The timeZone" << "[" << timeZone << "]" << " is now valid." << endl;

		return true;
	}
	cout << "The timeZone" << "[" << timeZone << "]" << " is not valid." << endl;

	return false;
}

void NtpClient::SetupNtpClient()
{
	// NTP Ŭ���̾�Ʈ ���� ��ɾ� ����
}

void NtpClient::SynchronizeNtpTime()
{
	// NTP ���� ����ȭ ��ɾ� ����
	//
	// 1. ��ġ���� ��ü ����
	//int sync_time = system("C:\\Manual_Sync\\sync.bat");

	//if (sync_time != 0)
	//{
	//	cerr << "Error running batch file!" << endl;
	//}

	// 2. [���� ����] ����
	while (true) // ���� ����: [1��] �ֱ� ����ȭ
	{
		int sync_time = system("w32tm /resync");

		cout << " " << endl;

		if (sync_time != 0)
		{
			cerr << "Error running command!" << endl;
		}

	}

	// 3. [���� ������] ����
	//int sync_time = system("w32tm /resync");

	//cout << " " << endl;

	//if (sync_time != 0)
	//{
	//	cerr << "Error running command!" << endl;
	//}

}
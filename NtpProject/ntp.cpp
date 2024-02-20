#include "ntp.h"

//윈도우 시간 라이브러리
#include <Windows.h>

// 시간 라이브러리 1
#include <chrono>

// 시간 라이브러리 2
#include <iomanip>

// 표준 입출력 라이브러리
#include <iostream>

#pragma warning(disable:4996) // _CRT_SECURE_NO_WARNINGS 무시

using namespace std;

using namespace chrono;

// NtpServer() 생성자
NtpServer::NtpServer()
{
	SetupNtpServer();
}

// NtpServer() 소멸자
NtpServer::~NtpServer()
{

}

// 윈도우 시간을 변경하기 위해서는

// 코드 실행 전 [관리자 권한] 속성으로 만들어야 함.

// 프로젝트 속성 -> 구성 속성 -> 링커 -> 매니페스트 파일 ->

// UAC 실행수준 -> highestAvailable로 변경

void NtpServer::SetTime(long long currentTime)
{
	// 시스템 시간 수동 설정 명령어 수행
	microseconds microseconds(currentTime); // 추가 시간을 microseconds로 변환

	// system_clock : 시스템의 실시간 시계를 기반으로 하는 시계

	//

	// 새로운 시간 = [시스템의 현재 시간 + 추가 시간]

	time_t new_time = system_clock::to_time_t(system_clock::now() + microseconds);

	// tm 구조체 : 새로운 시간을 참조하는 localtime 함수를 new_t 변수에 저장!

	tm* new_t = localtime(&new_time);

	// SYSTEMTIME 구조체 : 윈도우 시간 함수 존재

	SYSTEMTIME sys_time{}; // SYSTEMTIME 구조체 선언 및 초기화

	// 새로운 시간을 참조한 localtime을 윈도우 시간 함수에 저장

	sys_time.wYear = new_t->tm_year + 1900; // years since 1900

	sys_time.wMonth = new_t->tm_mon + 1; // months since January - [0, 11]

	sys_time.wDayOfWeek = new_t->tm_wday; // days sice Sunday - [0, 6]

	sys_time.wDay = new_t->tm_mday; // day of the month - [1, 31]

	sys_time.wHour = new_t->tm_hour; // hours since midnight - [0, 23]

	sys_time.wMinute = new_t->tm_min; // minutes after the hour - [0, 59]

	sys_time.wSecond = new_t->tm_sec; // seconds after the minute - [0, 59]

	sys_time.wMilliseconds = 0; // milliseconds after the seconds - [0, 999]

	SetLocalTime(&sys_time); // 표준 시간대에 새로운 시간 적용

	cout << "Setting time to: " << put_time(new_t, "%c") << '\n' << endl; // 바뀐 시간 출력
}

bool NtpServer::SetTimeZONE(int timeZone)
{
	// 시스템 시간 수동 설정 명령어 수행

	// TIME_ZONE_INFORMATION 구조체
	//
	// 표준 시간대에 대한 설정 지정
	TIME_ZONE_INFORMATION tzi{}; // TIME_ZONE_INFORMATION 구조체 선언 및 초기화

	// Bias 함수: 이 컴퓨터의 현지 시간 변환에 대한 [현재 바이어스(분)]을 표현
	//
	// 표준 시간(UTC) = 바이어스(분) + 현지 시간
	// 바이어스(분) = 표준 시간(UTC) - 현지 시간
	//
	// 현지 표준시간(UTC) = 0(기준점)이므로,
	//
	// tzi.Bias = -timeZone; // 바이어스(분) = -현지 시간

	// 1. 시간 단위로 변환
	//tzi.Bias = -(timeZone * 60); // 바이어스(분) = -(현지 시간 * 60)

	// 2. 분 단위 유지
	tzi.Bias = -timeZone;

	// SetTimeZONEInformation 함수: 표준 시간(UTC)에서 현지 시간으로의 변환 제어
	//
	// 1. tzi 참조 값이 0이 아니면 true 반환
	// 2. tzi 참조 값이 0이면 false 반환

	if (IsValidTimeZone(timeZone) && SetTimeZoneInformation(&tzi) != 0)
	{
		return true;
	}
	return false;
}

bool NtpServer::IsValidTimeZone(int timeZone) // 타임존 유효성 검사 O
{
	// 타임존 체크 기능 수행

	// 1. 타임존 시간이 -720 이상인 경우
	// 2. 타임존 시간이 +840 이하인 경우
	//
	// 타임존 범위 = (-720) ~ (+840)까지 위 [1, 2]의 조건 모두를 만족
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
	// NTP 서버 설정 명령어 수행
	//
	// 1. 기존 NTP 서버에서 원하는 NTP 서버로 설정 (Ex. 도메인 서버 IP: time.windows.com)
	int server_s1 = system("w32tm /config /syncfromflags:manual /manualpeerlist:time.windows.com /update");

	cout << " " << endl;

	// 2. Windows Time 서비스가 자동으로 시작되도록 설정
	int server_s2 = system("sc config w32time start=auto");

	cout << " " << endl;

	// 3. 재부팅 후, Windows Time 서비스가 자동으로 시작되지 않는 문제가 해결되도록 설정
	int server_s3 = system("sc triggerinfo w32time start/networkon stop/networkoff");

	cout << " " << endl;

	// 4-1. Windows Time 서비스 중지
	int server_s4_1 = system("net stop w32time");

	// 4-2. Windows Time 서비스 시작
	int server_s4_2 = system("net start w32time");

	// 5. Command 오류 문구 출력

	if (server_s1 || server_s2 || server_s3 || server_s4_1 || server_s4_2 != 0)
	{
		cerr << "Error running command!" << endl;
	}

}

// NtpClient() 생성자
NtpClient::NtpClient()
{
	SetupNtpClient(); // NTP 클라이언트 설정 명령어 수행

	// 정상적인 [루핑 스레드] 호출을 위한 방법
	_loopingThread.StartLoopingThread([this] // 스레드 시작
		{
			SynchronizeNtpTime(); // NTP 수동 동기화 명령어 수행 3

			std::this_thread::sleep_for(std::chrono::seconds(1));

			// 스레드 출력
			printf("[%s:%d] thread call\n", __func__, __LINE__);

		});

	// NTP 수동 동기화 명령어 수행 2. 무한 루프: [1초] 주기 동기화
	//SynchronizeNtpTime();

	// NTP 수동 동기화 명령어 수행 1. 배치파일 자체 실행
	//SynchronizeNtpTime();
}

// NtpClient() 소멸자
NtpClient::~NtpClient()
{
	_loopingThread.StopLoopingThread(); // 스레드 중지
}

bool NtpClient::SetTimeZone(int timeZone)
{
	// 시스템 시간 수동 설정 명령어 수행

	// TIME_ZONE_INFORMATION 구조체
	//
	// 표준 시간대에 대한 설정 지정
	TIME_ZONE_INFORMATION tzi{}; // TIME_ZONE_INFORMATION 구조체 선언 및 초기화

	// Bias 함수: 이 컴퓨터의 현지 시간 변환에 대한 [현재 바이어스(분)]을 표현
	//
	// 표준 시간(UTC) = 바이어스(분) + 현지 시간
	// 바이어스(분) = 표준 시간(UTC) - 현지 시간
	//
	// 현지 표준시간(UTC) = 0(기준점)이므로,
	//
	// tzi.Bias = -timeZone; // 바이어스(분) = -현지 시간

	// 1. 시간 단위로 변환
	//tzi.Bias = -(timeZone * 60); // 바이어스(분) = -(현지 시간 * 60)

	// 2. 분 단위 유지
	tzi.Bias = -timeZone;

	// SetTimeZONEInformation 함수: 표준 시간(UTC)에서 현지 시간으로의 변환 제어
	//
	// 1. tzi 참조 값이 0이 아니면 true 반환
	// 2. tzi 참조 값이 0이면 false 반환

	if (IsValidTimeZone(timeZone) && SetTimeZoneInformation(&tzi) != 0)
	{
		return true;
	}
	return false;
}

bool NtpClient::IsValidTimeZone(int timeZone) // 타임존 유효성 검사 O
{
	// 타임존 체크 기능 수행

	// 1. 타임존 시간이 -720 이상인 경우
	// 2. 타임존 시간이 +840 이하인 경우
	//
	// 타임존 범위 = (-720) ~ (+840)까지 위 [1, 2]의 조건 모두를 만족
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
	// NTP 클라이언트 설정 명령어 수행
}

void NtpClient::SynchronizeNtpTime()
{
	// NTP 수동 동기화 명령어 수행
	//
	// 1. 배치파일 자체 실행
	//int sync_time = system("C:\\Manual_Sync\\sync.bat");

	//if (sync_time != 0)
	//{
	//	cerr << "Error running batch file!" << endl;
	//}

	// 2. [무한 루프] 실행
	while (true) // 무한 루프: [1초] 주기 동기화
	{
		int sync_time = system("w32tm /resync");

		cout << " " << endl;

		if (sync_time != 0)
		{
			cerr << "Error running command!" << endl;
		}

	}

	// 3. [루핑 스레드] 실행
	//int sync_time = system("w32tm /resync");

	//cout << " " << endl;

	//if (sync_time != 0)
	//{
	//	cerr << "Error running command!" << endl;
	//}

}
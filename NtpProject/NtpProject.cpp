#include "ntp.h"

// 시간 라이브러리 1
#include <chrono>

// 시간 라이브러리 2
#include <iomanip>

// 표준 입출력 라이브러리
#include <iostream>

#pragma warning(disable:4996) // _CRT_SECURE_NO_WARNINGS 무시
#pragma warning(disable:6031)

using namespace std;
using namespace chrono;

int main()
{
	NtpServer ntpServer; // NtpServer 선언 및 호출!!!

	// 현재 시간 구하기!
	// 현재 시간 = [시스템의 현재 시간]
	time_t now_time = system_clock::to_time_t(system_clock::now());

	// tm 구조체: 새로운 시간을 참조하는 localtime 함수를 now_t 변수에 저장
	tm* now_t = localtime(&now_time);

	// 현재 시간 출력
	cout << "Current time is now: " << put_time(now_t, "%c") << '\n' << endl;

	// 바뀐 시간 구하기! [microseconds 기준: (1초 = 1,000,000 µs)]
	// ([1000000000] µs = 1000초) 이후의 시간?
	ntpServer.SetTime(1000000000); // SetTime(currentTime): [currentTime] µs 이후 시간 호출

	// 서버 표준 시간대(UTC) 호출
	ntpServer.SetTimeZONE(540); // Ex. (UTC + 9): 서울 표준 시간대(UTC) 호출

	//getchar(); // 중단점 호출

	NtpClient ntpClient; // NtpClient 선언 및 호출!!!

	// 클라이언트 표준 시간대(UTC) 호출
	ntpClient.SetTimeZone(540); // Ex. (UTC + 9): 서울 표준 시간대(UTC) 호출

	ntpClient.SynchronizeNtpTime();

	getchar(); // 중단점 호출

	return 0;
}

#include <am.h>
#include <nemu.h>
static uint64_t boot_time = 0;
void __am_timer_init() {
	//boot_time = (inl(RTC_ADDR + 4) << 32) + inl(RTC_ADDR);
	uint64_t hi_boottime = inl(RTC_ADDR + 4);
	uint64_t lo_boottime = inl(RTC_ADDR);
	boot_time = (hi_boottime << 32) | lo_boottime;
}
void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
	//get current time first
	//uint64_t current = (inl(RTC_ADDR + 4) << 32) + (inl(RTC_ADDR));
	uint64_t hi_current = inl(RTC_ADDR + 4);
	uint64_t lo_current = inl(RTC_ADDR);
	uint64_t current = (hi_current << 32) | lo_current;
	uptime->us = current - boot_time;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}

#ifndef ARCH_I386_RTC_H
#define ARCH_I386_RTC_H 1

#include <stdint.h>
#include <arch/i386/isr.h>

#define CMOS_ADDRESS_PORT	0x70
#define CMOS_DATA_PORT		0x71

typedef struct {
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hour;
	uint8_t	day;
	uint8_t month;
	uint16_t year;
} __attribute__ ((packed)) rtc_datetime_t;

typedef enum {
	CMOS_RTC_REG_SECONDS			= 0x00,
	CMOS_RTC_REG_SECONDS_ALARM		= 0x01,
	CMOS_RTC_REG_MINUTES			= 0x02,
	CMOS_RTC_REG_MINUTES_ALARM		= 0x03,
	CMOS_RTC_REG_HOURS				= 0x04,
	CMOS_RTC_REG_HOURS_ALARM		= 0x05,
	CMOS_RTC_REG_WEEK_DAY			= 0x06,
	CMOS_RTC_REG_MONTH_DAY			= 0x07,
	CMOS_RTC_REG_MONTH				= 0x08,
	CMOS_RTC_REG_YEAR				= 0x09,
	CMOS_RTC_REG_STATUS_A			= 0x0A,
	CMOS_RTC_REG_STATUS_B			= 0x0B,
	CMOS_RTC_REG_STATUS_C			= 0x0C
} CMOS_RTC_REGISTERS;

/**
 * CMOS RTC Status Register A Layout
 *   7  6     5  4             0
 * |----------------------------|
 * | U |   ST   |        DO     |
 * |----------------------------|
 *
 * DO: rate selection bits for divider output frequency
 * 		(set to 0110 = 1.024kHz)
 * ST: stage divider
 * U: 1 = update in progress, 0 = time/date available
 */
typedef enum {
	CMOS_RTC_REG_A_UPDATE		= 0x80
} CMOS_RTC_REG_A;

/**
 * CMOS RTC Status Register B Layout
 *
 *   7    6    5    4     3     2     1    0
 * |------------------------------------------|
 * | CU | PI | AI | UEI | SWF | BCD | HM | DS |
 * |------------------------------------------|
 *
 * DS: 1 = enable daylight savings, 0 = disabled (default)
 * HM: 1 = 24 hour mode, 0 = 12 hour mode (default is 24)
 * BCD: 1 = time/date in binary, 0 = BCCD (default is BCD)
 * SWF: 1 = square wave frequency enabled, 0 = disabled
 * UEI: 1 = update ended interrupt enabled, 0 = disabled
 * AI: 1 = alarm interrupt enabled, 0 = disabled
 * PI: 1 = periodic interrupt enabled, 0 = disabled
 * CU: 1 = clock update disabled, 0 = update count normally
 */
typedef enum {
	CMOS_RTC_REG_B_DS			= 0x01,
	CMOS_RTC_REG_B_24MODE		= 0x02,
	CMOS_RTC_REG_B_BINARY		= 0x04,
	CMOS_RTC_REG_B_SWF			= 0x08,
	CMOS_RTC_REG_B_UEI			= 0x10,
	CMOS_RTC_REG_B_AI			= 0x20,
	CMOS_RTC_REG_B_PI			= 0x40,
	CMOS_RTC_REG_B_CU			= 0x80
} CMOS_RTC_REG_B;

uint8_t cmos_update_in_progress(void);
uint8_t cmos_get_rtc_register(uint8_t);
void cmos_Read_rtc(void);
void rtc_print_datetime(void);

#endif /* !ARCH_I386_RTC_H */

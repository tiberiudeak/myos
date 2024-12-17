#ifdef CONFIG_RTC

#include <arch/i386/rtc.h>
#include <kernel/io.h>
#include <kernel/tty.h>

/**
 * @brief Check if CMOS has an update in progress
 *
 * If the incrementation of seconds rolled over, then the minutes are
 * increased next and a check is made if that also rolled over. If it did,
 * then the hour is incremented and so on, until the century is incremented.
 *
 * @return If an update is in progress
 */
uint8_t cmos_update_in_progress(void) {
	// select status register A and disable NMI (by setting the 0x80 bit)
	port_byte_out(CMOS_ADDRESS_PORT, CMOS_RTC_REG_STATUS_A | 0x80);

	// read byte from data port and check if update in progress bit is set (bit 7)
	return (port_byte_in(CMOS_DATA_PORT) & CMOS_RTC_REG_A_UPDATE);
}

/**
 * @brief Get byte from given CMOS register
 *
 * This function returns the byte in the given CMOS register.
 *
 * @param reg The register
 *
 * @return The byte at that register
 */
uint8_t cmos_get_rtc_register(uint8_t reg) {
	// also disable NMI
	port_byte_out(CMOS_ADDRESS_PORT, reg | 0x80);
	return port_byte_in(CMOS_DATA_PORT);
}

/**
 * @brief Print datetime to screen
 *
 * This function gets from the CMOS RTC the date and time two times until
 * they are the same to avoid getting inconsistent values. Then, if in
 * BCD mode, convert to binary.
 *
 * BCD mode: each of the two hex nibbles of the byte represents a deciaml number
 * example: 1:23:54 has hour = 1, minutes = 0x23 = 35, seconds = 0x54 = 84
 * conversion formula: binary = (bcd & 0x0F) + ((bcd >> 4) * 10)
 *
 * 24 hour time mode: hour 0 is midnight to 1am, hour 23 is 11pm
 *
 * 12 hour time: if the hour is pm, then the 0x80 bit is set on the hour byte.
 * midnight is 12, 1am  is 1 etc.
 */
void rtc_print_datetime(void) {
	struct rtc_datetime time1, time2;
	uint16_t century = 20; // update every century!

	while (cmos_update_in_progress()) ;

	time1.seconds = cmos_get_rtc_register(CMOS_RTC_REG_SECONDS);
	time1.minutes = cmos_get_rtc_register(CMOS_RTC_REG_MINUTES);
	time1.hour = cmos_get_rtc_register(CMOS_RTC_REG_HOURS);
	time1.day = cmos_get_rtc_register(CMOS_RTC_REG_MONTH_DAY);
	time1.month = cmos_get_rtc_register(CMOS_RTC_REG_MONTH);
	time1.year = cmos_get_rtc_register(CMOS_RTC_REG_YEAR);

	do {
		time2 = time1;
		while (cmos_update_in_progress()) ;

		time1.seconds = cmos_get_rtc_register(CMOS_RTC_REG_SECONDS);
		time1.minutes = cmos_get_rtc_register(CMOS_RTC_REG_MINUTES);
		time1.hour = cmos_get_rtc_register(CMOS_RTC_REG_HOURS);
		time1.day = cmos_get_rtc_register(CMOS_RTC_REG_MONTH_DAY);
		time1.month = cmos_get_rtc_register(CMOS_RTC_REG_MONTH);
		time1.year = cmos_get_rtc_register(CMOS_RTC_REG_YEAR);
	} while ((time1.seconds != time2.seconds) || (time1.minutes != time2.minutes) ||
	(time1.hour != time2.hour) || (time1.day != time2.day) || (time1.month != time2.month) ||
	(time1.year != time2.year));

	uint8_t regB_value = cmos_get_rtc_register(CMOS_RTC_REG_STATUS_B);

	if (!(regB_value & CMOS_RTC_REG_B_BINARY)) {
		// convert BCD to binary
		time1.seconds = (time1.seconds & 0x0F) + ((time1.seconds >> 4) * 10);
		time1.minutes = (time1.minutes & 0x0F) + ((time1.minutes >> 4) * 10);
		time1.hour = ((time1.hour & 0x0F) + (((time1.hour & 0x70) >> 4) * 10)) | (time1.hour & 0x80);
		time1.day = (time1.day & 0x0F) + ((time1.day >> 4) * 10);
		time1.month = (time1.month & 0x0F) + ((time1.month >> 4) * 10);
		time1.year = (time1.year & 0x0F) + ((time1.year >> 4) * 10);
	}

	// if 12 hour mode and pm
	if (!(regB_value & CMOS_RTC_REG_B_24MODE) && (time1.hour & 0x80)) {
		time1.hour = ((time1.hour & 0x7F) + 12) % 24;
	}

	time1.year += century * 100;

	printk("%d.%d.%d  %d:%d:%d\n", time1.day, time1.month, time1.year,
								time1.hour, time1.minutes, time1.seconds);
}
#endif /* CONFIG_RTC */

// TODO: create IRQ8 handler that prints the datetime to the screen with
// real time updates (so nothing will be printed to the bottom row except the datetime)

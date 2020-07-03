/*
 * DS1343.h
 * NAME: REAL TIME CLOCK
 * Created: 9/27/2018 10:15:55 AM
 *  Author: Sarthak
 */ 

#ifndef DS1343_H_
#define DS1343_H_



/*  PARAMETER FOR SENDING SECONDS, MINUTES, HOUR, DAY, DATE, MONTH, YEAR */
/*   
* SECONDS / MINUTES: 0x00 corresponds to 0 second / MINUTES
                     0x01 corresponds to 1 second / MINUTES
		                 .................
			             .................
		             0x59 corresponds to 59 seconds / MINUTES
					 
* HOURS: 0x00 TO 0x23
		   
* DAY:  SUNDAY := 0x01
        MONDAY := 0x02
		..............
		..............
		SATURDAY := 0x07
		
* MONTH: JANUARY := 0x01
         DECEMBER := 0x12
		 
* YEAR: 00-99
        2013 := 0x13

*/

/*  BIT MASKING OF CONTROL REG OF RTC */
 #define EOSC  0x80     // STOPPING OSCILLATOR DURING BATTERY BACKUP
 #define EGFIL 0x10     // ENABLING GLITCH FILTER
 #define SQW   0x08     // SQUARE WAVE GENERATION AT PIN INT1
 #define INTCN 0x04     // ALARM0 TO TRIGGER INT0 & ALARM1 TO TRIGGER INT1
 #define A1IE  0x02     // ALARM 1 INTERRUPT ENABLE
 #define A0IE  0x01     // ALARM 0 INTERRUPT ENABLE

/* ALARM TYPE */
#define ALARM_ONCE_SEC 0x01
#define ALARM_SEC_MATCH 0x02
#define ALARM_MS_MATCH 0x03
#define ALARM_HMS_MATCH 0x04
#define ALARM_DHMS_MATCH 0x05

uint32_t interval_Ala1;
int interval_Ala0;
int alarm_conf_Ala0;
int alarm_conf_Ala1;
/**********************************************************************************************************************************/
/*   
	FUNCTION NAME:	ds1343_init
	DESCRIPTION:	configures the SPI mode, clock & Chip select pin & interrupt                                                                   
*/
void ds1343_init(void);
/**********************************************************************************************************************************/
/**********************************************************************************************************************************/
/*
	FUNCTION NAME:  ds1343_read
	DESCRIPTION:	reads from the rtc
	RETURN VALUE:	returns unsigned integer                                                                     
*/
uint8_t ds1343_read(void);
/**********************************************************************************************************************************/
/**********************************************************************************************************************************/
/*                                                                      
	FUNCTION NAME:  ds1343_write
	DESCRIPTION:	writes to the rtc
	PARAMETER:		address or data to write to rtc
*/
void ds1343_write(uint8_t value);
/**********************************************************************************************************************************/
/**********************************************************************************************************************************/
/*                                                                      
	FUNCTION NAME:  ds1343_time_configuration
	DESCRIPTION:	configures time, calendar & alarm
*/
void ds1343_time_configuration(void);
/**********************************************************************************************************************************/
/**********************************************************************************************************************************/
/*
	FUNCTION NAME:  ds1343_alarm_write
	DESCRIPTION:	configures Alarm0 as per the interval
	PARAMETER:		time interval can be between 10 to 86400
			 :		type of alarm 
					Every Sec: ALARM_ONCE_SEC
					Second Match: ALARM_SEC_MATCH
					Minutes & Seconds Match: ALARM_MS_MATCH
					Hour, Minutes & Seconds Match: ALARM_HMS_MATCH
					Day, Hour, Minutes & Seconds Match: ALARM_DHMS_MATCH 0x05                   
*/
void ds1343_alarm_write(uint32_t interval,int alarm_conf,int *get_ala);
/**********************************************************************************************************************************/

void get_ds1343_time(void);

void ds1343_interrupt_init(void);

void ds1343_interrupt_disable(void);

void service_rtc_interrupt(void);

void Ack_ds1343_interrupt(void);

void ds1343_interrupt_off(void);

#endif /* DS1343_H_ */
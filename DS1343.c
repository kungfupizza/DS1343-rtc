/*
 * DS1343.c
 * NAME: REAL TIME CLOCK with alarm and ISR
 * Created: 9/27/2018 10:15:10 AM
 *  Author: Sarthak
 */ 
#include <DS1343.h>

struct spi_device ds1343_ce ={
	.id=IOPORT_CREATE_PIN(PORTA,4)
};


void ds1343_init(void)
{
	spi_master_init(&SPIC);
	spi_master_setup_device(&SPIC, &ds1343_ce,SPI_MODE_3, 1000000,1); // spi mode 3 & Freq: 1MHz
	spi_enable(&SPIC);
}

void ds1343_interrupt_init(void)
{
	PORTF.INT1MASK |= 0x04;                           // making the pin no 4 as interrupt
	PORTF.INTCTRL |= 0x08;                            // configuring interrupt for portE	
}

void ds1343_interrupt_disable(void)
{
	PORTF.INT1MASK |= 0x04;                           // making the pin no 4 as interrupt
	PORTF.INTCTRL &= 0xF7;                            // configuring interrupt for portE
}

uint8_t ds1343_read(void)
{
	uint8_t data;
	while(!(SPIC.STATUS & 0x80));
	data=SPIC_DATA;
	return data;
		
}

void ds1343_write(uint8_t value)
{
    SPIC.DATA=value;
	while (!spi_is_rx_full(&SPIC));	
}


void ds1343_time_configuration(void)
{
	uint8_t ret;
	uint8_t clk[6];
	/*	Array to store the user provided time to configure RTC every alternate bytes are address of RTC & 
		adjacent bytes will be value to store at particular address 
	*/
	int get_clk[14]={0x80,' ',0x81,' ',0x82,' ',0x83,' ',0x84,' ',0x85,' ',0x86,' '};
	int get_ala[8]={0x8b,' ',0x8c,' ',0x8d,' ',0x8e,' '};

		clk[0]=(date_tmp.second);
		clk[1]=(date_tmp.minute);
		clk[2]=(date_tmp.hour);
		clk[3]=(date_tmp.date)+1;
		clk[4]=(date_tmp.month)+1;
		clk[5]=(date_tmp.year)-2000;		// only one byte is allocated for year. so subtracted by current century.
	user_debug_1("please input sec,min,hr\r\n");
	/* configuring the RTC timing */
	for(int i=1,j=0;i<6;)
	{
		get_clk[i]=dec_to_hex(clk[j]);
		user_debug_1("get_clk[%d]=%x\r\n",i,get_clk[i]);
		j=j+1;
		i=i+2;
	}
	for(int i=9,j=3;i<14;)
	{
		get_clk[i]=dec_to_hex(clk[j]);
		user_debug_1("get_clk[%d]=%x\r\n",i,get_clk[i]);
		j=j+1;		
		i=i+2;
	}
	/*	writes to the RTC through SPI communication
		configures clock, calendar.
	*/
	for(int i=0;i<14;)
	{
		spi_master_setup_device(&SPIC, &ds1343_ce,SPI_MODE_3, 1000000,1);
		spi_deselect_device(&SPIC,&ds1343_ce);
		ds1343_write(get_clk[i++]);
		ds1343_write(get_clk[i++]);
		spi_select_device(&SPIC,&ds1343_ce);
	}	
	delay_ms(100);
	ds1343_alarm_write(0,ALARM_ONCE_SEC,get_ala);
	
	spi_master_setup_device(&SPIC, &ds1343_ce,SPI_MODE_3, 1000000,1);
	spi_deselect_device(&SPIC,&ds1343_ce);
	ds1343_write(0x8f);
	ds1343_write(0x06);
	spi_select_device(&SPIC,&ds1343_ce);
	delay_ms(1);
	spi_deselect_device(&SPIC,&ds1343_ce);
	ds1343_write(0x90);
	ds1343_write(0x00);
	spi_select_device(&SPIC,&ds1343_ce);
}

void ds1343_interrupt_off(void)
{
	spi_master_setup_device(&SPIC, &ds1343_ce,SPI_MODE_3, 1000000,1);
	spi_deselect_device(&SPIC,&ds1343_ce);
	ds1343_write(0x8f);
	ds1343_write(0x00);
	spi_select_device(&SPIC,&ds1343_ce);	
}

void ds1343_alarm_write(uint32_t interval,int alarm_conf,int *get_ala)
{	
	uint8_t add_rtc[4]={0x00,0x01,0x02,0x03};
	uint8_t get_clk[4];
	spi_master_setup_device(&SPIC, &ds1343_ce,SPI_MODE_3, 1000000,1);
	/* gets the current time from RTC to configure Alarm0 as per interval */
	for(int i=0;i<4;i++)
	{
		spi_master_setup_device(&SPIC, &ds1343_ce,SPI_MODE_3, 1000000,1);
		spi_deselect_device(&SPIC,&ds1343_ce);
		ds1343_write(add_rtc[i]);
		ds1343_read();
		ds1343_write(0x00);
		get_clk[i]=ds1343_read();
		spi_select_device(&SPIC,&ds1343_ce);
		delay_ms(1);
	}
	get_ala[1]=get_clk[0];
	get_ala[3]=get_clk[1];
	get_ala[5]=get_clk[2];
	get_ala[7]=get_clk[3];
	for(int i=0;i<interval;i++)
	{
		if(get_clk[0]==0x59) //if second equal to 59 then roll off to 0.
		{
			get_ala[1]=0x00;
			get_clk[0]=0x00;
		
			if(get_clk[1] == 0x59) // if minutes equal to 59 then roll off to 0.
			{
				get_ala[3]=0x00;
				get_clk[1]=0x00;
				
				if(get_clk[2]==0x23) // if hour equal to 23 then roll off to 0
				{
					get_ala[5] = 0x00;
					get_clk[2]=0x00;
					
					if(get_clk[3]==0x07) // if week equal to 7 then roll off to 0
					{
						get_ala[7]==0x00;
						get_clk[3]=0x00;
					}
					else
					{
						get_clk[3] = get_clk[3] + 0x01; // increment the day by one.
						get_ala[7]= get_clk[3];
					}	
					
				}
				else if(get_clk[2]==0x09||get_clk[2]==0x19) //if hour equal to 9 or 19 then BCD roll off to 10 or 20.
				{
					get_ala[5] = get_clk[2] & 0xf0;
					get_ala[5] = get_ala[5] + 0x10;
					get_clk[2] = get_ala[5];
				}	
				else
				{
					get_clk[2] = get_clk[2] + 0x01; // increment the hour by one.
					get_ala[5]= get_clk[2];
				}
				
				
			}
			/* if minutes equal to 9 / 19 / 29 / 39 / 49 then BCD roll off to 10 / 20 / 30 / 40 / 50.*/
			else if(get_clk[1] == 0x09 || get_clk[1] == 0x19 || get_clk[1] == 0x29 || get_clk[1] == 0x39 || get_clk[1] == 0x49)
			{
				get_ala[3] = get_clk[1] & 0xf0;
				get_ala[3] = get_ala[3] + 0x10;
				get_clk[1] = get_ala[3];
			}
			else
			{
				get_clk[1] = get_clk[1] + 0x01;   // increment minute by one.
				get_ala[3]= get_clk[1];
			}
		
		}
		/* if seconds equal to 9 / 19 / 29 / 39 / 49 then BCD roll off to 10 / 20 / 30 / 40 / 50.*/
		else if(get_clk[0] == 0x09 || get_clk[0] == 0x19 || get_clk[0] == 0x29 || get_clk[0] == 0x39 || get_clk[0] == 0x49)
		{
			get_ala[1] = get_clk[0] & 0xf0;
			get_ala[1] = get_ala[1] + 0x10;
			get_clk[0] = get_ala[1];
		}
		else
		{
			get_clk[0] = get_clk[0] + 0x01;  // increment second by one
			get_ala[1]= get_clk[0];
		}
	}
	for(int i=1;i<8;)
	{
		user_debug_1("the alarm is %x\r\n",get_ala[i]);
		i=i+2;
	}

	switch(alarm_conf)
	{
		case ALARM_ONCE_SEC:
			for(int i=0;i<8;)
			{
				// configure Alarm0 as every second
				spi_master_setup_device(&SPIC, &ds1343_ce,SPI_MODE_3, 1000000,1);
				spi_deselect_device(&SPIC,&ds1343_ce);
				ds1343_write(get_ala[i++]);
				ds1343_write(get_ala[i++]|0x80);
				spi_select_device(&SPIC,&ds1343_ce);
			}
		break;

		case ALARM_DHMS_MATCH:
		    for(int i=0;i<8;)
		    {
				// configure Alarm0 as day, hour, minutes & seconds match with the RTC clock
			   	spi_master_setup_device(&SPIC, &ds1343_ce,SPI_MODE_3, 1000000,1);
				spi_deselect_device(&SPIC,&ds1343_ce);
			   	ds1343_write(get_ala[i++]);
			   	ds1343_write(get_ala[i++]);
			   	spi_select_device(&SPIC,&ds1343_ce);
		    }
		break;
			
		case ALARM_HMS_MATCH:
		    	for(int i=0;i<8;)
		    	{
					// configure Alarm0 as hour, minutes & seconds match with the RTC clock
			    	spi_master_setup_device(&SPIC, &ds1343_ce,SPI_MODE_3, 1000000,1);
					spi_deselect_device(&SPIC,&ds1343_ce);
			    	if(get_ala[i]==0x8A)
					{
						ds1343_write(get_ala[i++]);
			    		ds1343_write(get_ala[i++] + 0x80);
					}
					else
					{
						ds1343_write(get_ala[i++]);
						ds1343_write(get_ala[i++]);
					}
					spi_select_device(&SPIC,&ds1343_ce);
		    	}
		break;
			
		case ALARM_MS_MATCH:
		    for(int i=0;i<8;)
		    {
				// configure Alarm0 as minutes & seconds match with the RTC clock
			    spi_master_setup_device(&SPIC, &ds1343_ce,SPI_MODE_3, 1000000,1);
				spi_deselect_device(&SPIC,&ds1343_ce);
			    if(get_ala[i]==0x89 || get_ala[i]==0x8a)
			    {
				    ds1343_write(get_ala[i++]);
				    ds1343_write(get_ala[i++] + 0x80);
			    }
			    else
			    {
				    ds1343_write(get_ala[i++]);
				    ds1343_write(get_ala[i++]);
			    }
			    spi_select_device(&SPIC,&ds1343_ce);
		    }
		break;
			
		case ALARM_SEC_MATCH:
		    for(int i=0;i<8;)
		    {
				// configure Alarm0 as seconds match with the RTC clock
			    spi_master_setup_device(&SPIC, &ds1343_ce,SPI_MODE_3, 1000000,1);
				spi_deselect_device(&SPIC,&ds1343_ce);
			    if(get_ala[i]==0x87)
			    {
				    ds1343_write(get_ala[i++]);
				    ds1343_write(get_ala[i++]);
			    }
			    else
			    {
				    ds1343_write(get_ala[i++]);
				    ds1343_write(get_ala[i++] + 0x80);
			    }
			    spi_select_device(&SPIC,&ds1343_ce);
		    }
		break;
		
		default:
		break;
				
	}
	
}

void get_ds1343_time(void)
{
	uint8_t add_rtc[6]={0x00,0x01,0x02,0x04,0x05,0x06};
	uint8_t get_clk[6];
	ds1343_init();
	spi_master_setup_device(&SPIC, &ds1343_ce,SPI_MODE_3, 1000000,1);
	/* gets the current time from RTC to configure Alarm0 as per interval */
	for(int i=0;i<6;i++)
	{
		spi_deselect_device(&SPIC,&ds1343_ce);
		ds1343_write(add_rtc[i]);
		ds1343_read();
		ds1343_write(0x00);
		get_clk[i]=ds1343_read();
		spi_select_device(&SPIC,&ds1343_ce);
		user_debug_1("the system clock is %x\r\n",get_clk[i]);
		delay_ms(1);
	}	
		date_tmp.second = hex_to_dec(get_clk[0],sizeof(get_clk[0]));
		date_tmp.minute = hex_to_dec(get_clk[1],sizeof(get_clk[1]));
		date_tmp.hour = hex_to_dec(get_clk[2],sizeof(get_clk[2]));
		date_tmp.date = hex_to_dec(get_clk[3],sizeof(get_clk[3]))-1;
		date_tmp.month = hex_to_dec(get_clk[4],sizeof(get_clk[4]))-1;
		date_tmp.year = hex_to_dec(get_clk[5],sizeof(get_clk[5]))+2000;		// only one byte is allocated for year. so subtracted by current century.
}

void Ack_ds1343_interrupt(void)
{
	spi_master_setup_device(&SPIC, &ds1343_ce,SPI_MODE_3, 1000000,1);
	spi_deselect_device(&SPIC,&ds1343_ce);
	ds1343_write(0x90);
	ds1343_write(0xFD);
	spi_select_device(&SPIC,&ds1343_ce);
	spi_master_setup_device(&SPIC, &ds1343_ce,SPI_MODE_3,1000000,1);  // setting it to 1Mhz.
}

void service_rtc_interrupt(void)
{
	if (rtc_commn_diff<=(ds1343_tick-rtc_commn_int))
	{
		rtc_commn_int=ds1343_tick;
		rtc_1_isr=1;
	}
	if(rtc_rcd_diff<=(ds1343_tick-rtc_rcd_int))
	{
		rtc_rcd_int=ds1343_tick;
		rtc_2_isr=1;
	}
	if(screen_diff<=(ds1343_tick-screen))
	{
		rtc_0_isr=1;
		screen=ds1343_tick;
	}
	if(rtc_sd_diff<=(ds1343_tick-rtc_sd_int))
	{
		rtc_3_isr=1;
		rtc_sd_int=ds1343_tick;
	}
	if(rtc_smp_diff<=(ds1343_tick-rtc_smp_int))
	{
		rtc_4_isr=1;
		rtc_smp_int=ds1343_tick;
	}
}
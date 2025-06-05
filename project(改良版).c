#include <reg52.h>
#include <stdio.h>
#define uchar unsigned char
#define uint unsigned int 
#define LCD_DATA P0

// ����DS1302ʱ�ӼĴ�����ַ
#define DS1302_SEC_REG 0x80
#define DS1302_MIN_REG 0x82
#define DS1302_HR_REG 0x84
#define DS1302_DAY_REG 0x86
#define DS1302_MONTH_REG 0x88
#define DS1302_YEAR_REG 0x8C

// ����DS1302���ƼĴ�������
#define DS1302_CMD_WRITE 0x80
#define DS1302_CMD_READ 0x81

// ���崮�ڲ�����Ϊ9600
#define BAUDRATE 9600
#define FOSC 11059200L
#define TIMER_INTERVAL (65536 - FOSC / 12 / BAUDRATE)

// ����Data��Command�Ĵ���ѡ��˿�
sbit LCD_RS = P2^0;// RS���ţ��Ĵ���ѡ��
sbit LCD_RW = P2^1;// RW���ţ���дѡ��
sbit LCD_EN = P2^2; // EN���ţ�ʹ�ܣ�

// ����ȫ�ֱ���
uchar time_buffer[20];   // ���ʱ���ַ���
uchar alarm_buffer[20];  // �������ʱ���ַ���
uint i;
bit flag;                // ����Ƿ���յ���λ����ʱ���ַ���

// ��ʼ��UARTģ��
void InitUart()
{
	TMOD &= 0x0F;
	TMOD |= 0x20;
	TH1 = TIMER_INTERVAL / 256;
	TL1 = TIMER_INTERVAL % 256;
	PCON |= 0x80; SCON = 0x50;
	ES = 1;
	TR1 = 1; 
	EA = 1;
}

// �������ֽڷ��͵�����
void SendData(uchar dat)
{ 
	SBUF = dat;
	while (!TI); 
	TI = 0; 
} 

// ���ַ������͵�����
void SendString(uchar *s) 
{ 
	while (*s != '\0') 
	{ 
		SendData(*s++);
	} 
}

// ��ʼ��DS1302ʱ��оƬ
void InitDS1302() 
{ 
	uchar i;

// ʹ��DS1302д�������� 
	DS1302_CE = 0; 
	DS1302_SCL = 0; 
	DS1302_CE = 1; 
	Write_DS1302(DS1302_CMD_WRITE | 0x8e, 0x80); 

	// �ر�ʱ������׼��д������ 
	Write_DS1302(DS1302_CMD_WRITE | 0x90, 0x00);
	
	// д��������ʱ������ 
	Write_DS1302(DS1302_SEC_REG, 0x00); 
	Write_DS1302(DS1302_MIN_REG, 0x30); 
	Write_DS1302(DS1302_HR_REG, 0x11); 
	Write_DS1302(DS1302_DAY_REG, 0x08);
	Write_DS1302(DS1302_MONTH_REG, 0x09); 
	Write_DS1302(DS1302_YEAR_REG, 0x21);
	Write_DS1302(0x8e, 0x00);
	
	// ��ʼ������ʱ�� 
	for (i = 0; i < 20; i++) 
	{
		alarm_buffer[i] = 0; 
	}
} 

// ��DS1302д������ 
void Write_DS1302(uchar addr, uchar dat) 
{
	uchar i; 
	DS1302_CE = 0;
	DS1302_SCL = 0;

	// ������ʼ�ź�
	DS1302_CE = 1;
	DS1302_SCL = 1;
	DS1302_CE = 0;

	DS1302_WriteByte(addr);// ���������ֽڵ�ַ
	DS1302_WriteByte(dat);// ���������ֽ�

	// ֹͣ�ź�
	DS1302_SCL = 0;
	DS1302_CE = 1;

	// ��ʱ����1us
	for (i = 0; i < 10; i++);
} 

// ��DS1302��ȡ����
uchar Read_DS1302(uchar addr) 
{ 
	uchar dat;
	uchar i; 

	DS1302_CE = 0; 
	DS1302_SCL = 0; 

	// ������ʼ�ź� 
	DS1302_CE = 1; 
	DS1302_SCL = 1; 
	DS1302_CE = 0; 

	DS1302_WriteByte(addr | 0x01); // ���������ֽڵ�ַ 
	dat = DS1302_ReadByte(); // ��ȡ�����ֽ� 

	// ֹͣ�ź� 
	DS1302_SCL = 0; 
	DS1302_CE = 1; 

	// ��ʱ����1us 
	for (i = 0; i < 10; i++); 
	return dat; 
} 
// ��ȡDS1302ʱ�䲢��ӡ������ 
void ReadTime() 
{ 
	uchar sec, min, hour, day, month, year; 
	sprintf(time_buffer, "Time: "); 
	sec = Read_DS1302(DS1302_SEC_REG); 
	min = Read_DS1302(DS1302_MIN_REG); 
	hour = Read_DS1302(DS1302_HR_REG); 
	day = Read_DS1302(DS1302_DAY_REG); 
	month = Read_DS1302(DS1302_MONTH_REG); 
	year = Read_DS1302(DS1302_YEAR_REG); 
	sprintf(time_buffer + 6, "%02d:%02d:%02d %02d/%02d/%02d\r\n", hour, min, sec, day, month, year); 
	SendString(time_buffer); 
} 
// ��DS1302д������ʱ�� 
void SetAlarm(uchar *str) 
{
	uint i = 0; 

	// ���ַ���ת��Ϊ���� 
	while (str[i] != '\0') 
	{ 
		alarm_buffer[i] = str[i] - '0'; 
		i++; if (i > 19) 
		// ��ֹ��� break; 
	}

	// д������ʱ�� 
	Write_DS1302(DS1302_CMD_WRITE | 0x81, alarm_buffer[10] << 4 | alarm_buffer[11]); 
	Write_DS1302(DS1302_CMD_WRITE | 0x83, alarm_buffer[8] << 4 | alarm_buffer[9]); 
	Write_DS1302(DS1302_CMD_WRITE | 0x85, alarm_buffer[6] << 4 | alarm_buffer[7]); 
} 

// �Ӵ��ڽ��������н�����ʱ����Ϣ 
void ParseTime() 
{ 
	uchar i, j; 
	uchar temp; 
	for (i = 0; i < 20; i++) 
	{ 
		time_buffer[i] = 0; 
	} 
	// �����ַ�����ʽΪ��hh:mm:ss dd/mm/yy 
	for (i = 0; i < 8; i++) 
	{ 
		temp = 0; 
		for (j = 0; j < 2; j++) 
		{ 
			temp *= 10; 
			temp += (SBUF - '0'); 
			while (!RI); // �ȴ�������� RI = 0; 
		} 
		time_buffer[i] = temp; 
		if (i == 2 || i == 4) 
		{ 
			while (SBUF != ' ');// �����ո��ַ� 
			while (!RI);// �ȴ��������
	
			RI = 0; 
		} 
	} 
	flag = 1; // ����Ѿ����յ��ַ���	
} 

// ������ 
void main() 
{ 
	InitUart(); 
	InitDS1302(); 
	flag = 0; 
	while (1) 
	{ 
		if (flag) 
		{ 
			// ���յ�ʱ���ַ������������Ӻ�ʱ�� 
			SetAlarm(time_buffer); 
			Write_DS1302(DS1302_CMD_WRITE | 0x80, time_buffer[6] << 4 | time_buffer[7]); 
			Write_DS1302(DS1302_CMD_WRITE | 0x82, time_buffer[3] << 4 | time_buffer[4]); 
			Write_DS1302(DS1302_CMD_WRITE | 0x84, time_buffer[0] << 4 | time_buffer[1]); 
			flag = 0; 
		} 
		ReadTime(); // ��ȡ��ǰʱ�䲢���͵�����	
	} 
} 

// UART�����жϺ��� 
void UartIsr() interrupt 4 
{ 
	if (RI) 
	{ 
		// ���յ����� 
		ParseTime(); // ����ʱ���ַ���	
	} 
	RI = 0;
}

void DelayMs(unsigned int ms)
{ 
	unsigned int i, j;
	for (i = 0; i < ms; i++)
		for (j = 0; j < 120; j++);
}
void WriteCommand(unsigned char cmd)
{
	LCD_RS = 0;// ѡ��ָ��Ĵ���
	LCD_RW = 0;// дģʽ
	LCD_EN = 0; // �͵�ƽʹ�� 
	LCD_DATA = cmd; // ����ָ�� 
	DelayMs(1); // ��ʱ�ȴ�ָ��д��
	LCD_EN = 1; // �ߵ�ƽʹ��
	DelayMs(1); // ����һ��ʱ��
	LCD_EN = 0; // ����ʹ��
}
void WriteData(unsigned char dat)
{ 
	LCD_RS = 1; // ѡ�����ݼĴ���
	LCD_RW = 0; // дģʽ
	LCD_EN = 0; // �͵�ƽʹ��
	LCD_DATA = dat; // ��������
	DelayMs(1); // ��ʱ�ȴ�����д��
	LCD_EN = 1; // �ߵ�ƽʹ��
	DelayMs(1); // ����һ��ʱ��
	LCD_EN = 0; // ����ʹ��
} 
void LCDInit() 
{ 
	WriteCommand(0x38); // ������ʾģʽΪ2�С�5x8�����ַ� 
	WriteCommand(0x0C); // ��ʾ���������ر� 
	WriteCommand(0x06); // ������ƣ��������ƶ� 
	WriteCommand(0x01); // �����ʾ�����ù��ص���ʼλ��
} 
void LCDDisplayTime(char* time) 
{ 
	int i; 
	WriteCommand(0x80); // ���ù��λ��Ϊ��һ�е���ʼλ��

	for (i = 0; i < 16; i++) 
	{ 
		WriteData(time[i]);// �ڵ�һ����ʾʱ���ַ���
	} 

	WriteCommand(0xC0); // ���ù��λ��Ϊ�ڶ��е���ʼλ�� 

	for (i = 0; i < 16; i++) 
	{ 
		WriteData(time[16 + i]); // �ڵڶ�����ʾʱ���ַ��� 
	} 
}
void main() 
{ 
	char time_buffer[32] = "Current Time: 00:00:00";// ʱ���ַ���
	unsigned char sec = 0, min = 0, hour = 0; // ��ǰʱ�����
	LCDInit(); // ��ʼ��LCD��ʾ��

	while (1)
	{ 
		// ����ʱ�����
		sec++;
		if (sec >= 60)
		{ 
			sec = 0;
			min++;
			if (min >= 60)
			{ 
				min = 0;
				hour++;
				if (hour >= 24)
				{ 
					hour = 0;
				} 
			} 
		} 

		// ��ʽ��ʱ���ַ���
		sprintf(time_buffer + 14, "%02d:%02d:%02d", hour, min, sec);
	
		// ��ʾʱ���ַ���
		LCDDisplayTime(time_buffer);
		DelayMs(1000); // ��ʱ1��
	}
}

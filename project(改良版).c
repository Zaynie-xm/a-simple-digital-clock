#include <reg52.h>
#include <stdio.h>
#define uchar unsigned char
#define uint unsigned int 
#define LCD_DATA P0

// 定义DS1302时钟寄存器地址
#define DS1302_SEC_REG 0x80
#define DS1302_MIN_REG 0x82
#define DS1302_HR_REG 0x84
#define DS1302_DAY_REG 0x86
#define DS1302_MONTH_REG 0x88
#define DS1302_YEAR_REG 0x8C

// 定义DS1302控制寄存器命令
#define DS1302_CMD_WRITE 0x80
#define DS1302_CMD_READ 0x81

// 定义串口波特率为9600
#define BAUDRATE 9600
#define FOSC 11059200L
#define TIMER_INTERVAL (65536 - FOSC / 12 / BAUDRATE)

// 定义Data和Command寄存器选择端口
sbit LCD_RS = P2^0;// RS引脚（寄存器选择）
sbit LCD_RW = P2^1;// RW引脚（读写选择）
sbit LCD_EN = P2^2; // EN引脚（使能）

// 声明全局变量
uchar time_buffer[20];   // 存放时间字符串
uchar alarm_buffer[20];  // 存放闹钟时间字符串
uint i;
bit flag;                // 标记是否接收到上位机的时间字符串

// 初始化UART模块
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

// 将单个字节发送到串口
void SendData(uchar dat)
{ 
	SBUF = dat;
	while (!TI); 
	TI = 0; 
} 

// 将字符串发送到串口
void SendString(uchar *s) 
{ 
	while (*s != '\0') 
	{ 
		SendData(*s++);
	} 
}

// 初始化DS1302时钟芯片
void InitDS1302() 
{ 
	uchar i;

// 使能DS1302写保护功能 
	DS1302_CE = 0; 
	DS1302_SCL = 0; 
	DS1302_CE = 1; 
	Write_DS1302(DS1302_CMD_WRITE | 0x8e, 0x80); 

	// 关闭时钟允许，准备写入数据 
	Write_DS1302(DS1302_CMD_WRITE | 0x90, 0x00);
	
	// 写入年月日时分秒周 
	Write_DS1302(DS1302_SEC_REG, 0x00); 
	Write_DS1302(DS1302_MIN_REG, 0x30); 
	Write_DS1302(DS1302_HR_REG, 0x11); 
	Write_DS1302(DS1302_DAY_REG, 0x08);
	Write_DS1302(DS1302_MONTH_REG, 0x09); 
	Write_DS1302(DS1302_YEAR_REG, 0x21);
	Write_DS1302(0x8e, 0x00);
	
	// 初始化闹钟时间 
	for (i = 0; i < 20; i++) 
	{
		alarm_buffer[i] = 0; 
	}
} 

// 向DS1302写入数据 
void Write_DS1302(uchar addr, uchar dat) 
{
	uchar i; 
	DS1302_CE = 0;
	DS1302_SCL = 0;

	// 发送起始信号
	DS1302_CE = 1;
	DS1302_SCL = 1;
	DS1302_CE = 0;

	DS1302_WriteByte(addr);// 发送命令字节地址
	DS1302_WriteByte(dat);// 发送数据字节

	// 停止信号
	DS1302_SCL = 0;
	DS1302_CE = 1;

	// 延时至少1us
	for (i = 0; i < 10; i++);
} 

// 向DS1302读取数据
uchar Read_DS1302(uchar addr) 
{ 
	uchar dat;
	uchar i; 

	DS1302_CE = 0; 
	DS1302_SCL = 0; 

	// 发送起始信号 
	DS1302_CE = 1; 
	DS1302_SCL = 1; 
	DS1302_CE = 0; 

	DS1302_WriteByte(addr | 0x01); // 发送命令字节地址 
	dat = DS1302_ReadByte(); // 读取数据字节 

	// 停止信号 
	DS1302_SCL = 0; 
	DS1302_CE = 1; 

	// 延时至少1us 
	for (i = 0; i < 10; i++); 
	return dat; 
} 
// 读取DS1302时间并打印到串口 
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
// 向DS1302写入闹钟时间 
void SetAlarm(uchar *str) 
{
	uint i = 0; 

	// 将字符串转换为数字 
	while (str[i] != '\0') 
	{ 
		alarm_buffer[i] = str[i] - '0'; 
		i++; if (i > 19) 
		// 防止溢出 break; 
	}

	// 写入闹钟时间 
	Write_DS1302(DS1302_CMD_WRITE | 0x81, alarm_buffer[10] << 4 | alarm_buffer[11]); 
	Write_DS1302(DS1302_CMD_WRITE | 0x83, alarm_buffer[8] << 4 | alarm_buffer[9]); 
	Write_DS1302(DS1302_CMD_WRITE | 0x85, alarm_buffer[6] << 4 | alarm_buffer[7]); 
} 

// 从串口接收数据中解析出时间信息 
void ParseTime() 
{ 
	uchar i, j; 
	uchar temp; 
	for (i = 0; i < 20; i++) 
	{ 
		time_buffer[i] = 0; 
	} 
	// 接收字符串格式为：hh:mm:ss dd/mm/yy 
	for (i = 0; i < 8; i++) 
	{ 
		temp = 0; 
		for (j = 0; j < 2; j++) 
		{ 
			temp *= 10; 
			temp += (SBUF - '0'); 
			while (!RI); // 等待接收完成 RI = 0; 
		} 
		time_buffer[i] = temp; 
		if (i == 2 || i == 4) 
		{ 
			while (SBUF != ' ');// 跳过空格字符 
			while (!RI);// 等待接收完成
	
			RI = 0; 
		} 
	} 
	flag = 1; // 标记已经接收到字符串	
} 

// 主函数 
void main() 
{ 
	InitUart(); 
	InitDS1302(); 
	flag = 0; 
	while (1) 
	{ 
		if (flag) 
		{ 
			// 接收到时间字符串，设置闹钟和时间 
			SetAlarm(time_buffer); 
			Write_DS1302(DS1302_CMD_WRITE | 0x80, time_buffer[6] << 4 | time_buffer[7]); 
			Write_DS1302(DS1302_CMD_WRITE | 0x82, time_buffer[3] << 4 | time_buffer[4]); 
			Write_DS1302(DS1302_CMD_WRITE | 0x84, time_buffer[0] << 4 | time_buffer[1]); 
			flag = 0; 
		} 
		ReadTime(); // 读取当前时间并发送到串口	
	} 
} 

// UART接收中断函数 
void UartIsr() interrupt 4 
{ 
	if (RI) 
	{ 
		// 接收到数据 
		ParseTime(); // 解析时间字符串	
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
	LCD_RS = 0;// 选择指令寄存器
	LCD_RW = 0;// 写模式
	LCD_EN = 0; // 低电平使能 
	LCD_DATA = cmd; // 发送指令 
	DelayMs(1); // 延时等待指令写入
	LCD_EN = 1; // 高电平使能
	DelayMs(1); // 持续一段时间
	LCD_EN = 0; // 结束使能
}
void WriteData(unsigned char dat)
{ 
	LCD_RS = 1; // 选择数据寄存器
	LCD_RW = 0; // 写模式
	LCD_EN = 0; // 低电平使能
	LCD_DATA = dat; // 发送数据
	DelayMs(1); // 延时等待数据写入
	LCD_EN = 1; // 高电平使能
	DelayMs(1); // 持续一段时间
	LCD_EN = 0; // 结束使能
} 
void LCDInit() 
{ 
	WriteCommand(0x38); // 设置显示模式为2行、5x8点阵字符 
	WriteCommand(0x0C); // 显示器开，光标关闭 
	WriteCommand(0x06); // 光标右移，整屏不移动 
	WriteCommand(0x01); // 清除显示并设置光标回到初始位置
} 
void LCDDisplayTime(char* time) 
{ 
	int i; 
	WriteCommand(0x80); // 设置光标位置为第一行的起始位置

	for (i = 0; i < 16; i++) 
	{ 
		WriteData(time[i]);// 在第一行显示时间字符串
	} 

	WriteCommand(0xC0); // 设置光标位置为第二行的起始位置 

	for (i = 0; i < 16; i++) 
	{ 
		WriteData(time[16 + i]); // 在第二行显示时间字符串 
	} 
}
void main() 
{ 
	char time_buffer[32] = "Current Time: 00:00:00";// 时间字符串
	unsigned char sec = 0, min = 0, hour = 0; // 当前时间变量
	LCDInit(); // 初始化LCD显示器

	while (1)
	{ 
		// 更新时间变量
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

		// 格式化时间字符串
		sprintf(time_buffer + 14, "%02d:%02d:%02d", hour, min, sec);
	
		// 显示时间字符串
		LCDDisplayTime(time_buffer);
		DelayMs(1000); // 延时1秒
	}
}

/*
 * oled.c
 *
 *  Created on: Jun 11, 2022
 *      Author: lf
 */
#include "bsp_oled.h"
#include "oledfont.h"
#include "oledfont_cn16.h"
#include "gpio.h"

//Oled port macro definition
//OLED端口宏定义
#define OLED_RST_Clr() HAL_GPIO_WritePin(OLED_RES_GPIO_Port,OLED_RES_Pin,GPIO_PIN_RESET);   //RST 置为0
#define OLED_RST_Set() HAL_GPIO_WritePin(OLED_RES_GPIO_Port,OLED_RES_Pin,GPIO_PIN_SET);     //RST 置为1

#define OLED_DC_Clr() HAL_GPIO_WritePin(OLED_DC_GPIO_Port,OLED_DC_Pin,GPIO_PIN_RESET);      //DC  置为0
#define OLED_DC_Set() HAL_GPIO_WritePin(OLED_DC_GPIO_Port,OLED_DC_Pin,GPIO_PIN_SET);        //DC  置为1

#define OLED_SCLK_Clr()  HAL_GPIO_WritePin(OLED_SCL_GPIO_Port,OLED_SCL_Pin,GPIO_PIN_RESET); //SCL 置为0
#define OLED_SCLK_Set()  HAL_GPIO_WritePin(OLED_SCL_GPIO_Port,OLED_SCL_Pin,GPIO_PIN_SET);   //SCL 置为1

#define OLED_SDIN_Clr() HAL_GPIO_WritePin(OLED_SDA_GPIO_Port,OLED_SDA_Pin,GPIO_PIN_RESET);  //SDA 置为0
#define OLED_SDIN_Set()  HAL_GPIO_WritePin(OLED_SDA_GPIO_Port,OLED_SDA_Pin,GPIO_PIN_SET);   //SDA 置为1

#define OLED_CMD  0	//Command //写命令
#define OLED_DATA 1	//Data    //写数据


uint8_t OLED_GRAM[128][8];   //定义显存数组

/**************************************************************************
Function: Refresh the OLED screen
Input   : none
Output  : none
函数功能：刷新OLED屏幕
入口参数：无
返回  值：无
**************************************************************************/
void OLED_Refresh_Gram(void)
{
	uint8_t i,n;
	for(i=0;i<8;i++)  // 第一层循环：遍历8个页面 Page0 ~ Page7
	{
		OLED_WR_Byte (0xb0+i,OLED_CMD);    //设置页地址（0~7）
		OLED_WR_Byte (0x00,OLED_CMD);      //设置显示位置—列低地址  0000 A3 A2 A1 A0
		OLED_WR_Byte (0x10,OLED_CMD);      //设置显示位置—列高地址  0001 A7 A6 A5 A4
		for(n=0;n<128;n++)OLED_WR_Byte(OLED_GRAM[n][i],OLED_DATA);  // 第二层循环：一页内遍历全部128列，把显存数据写到屏幕
	}
}
/**************************************************************************
Function: write one byte to Oled
Input   : Dat: data/command to write, CMD: data/command flag 0, represents the command;1, represents data
Output  : none
函数功能：向OLED写入一个字节
入口参数：dat:要写入的数据/命令，cmd:数据/命令标志 0,表示命令;1,表示数据
返回  值：无
说    明：参照手册P17-18
**************************************************************************/
void OLED_WR_Byte(uint8_t dat,uint8_t cmd)
{
	uint8_t i;
	if(cmd)
	{
		OLED_DC_Set();    //写数据
	}

	else
	{
		OLED_DC_Clr();     //写命令
	}

	for(i=0;i<8;i++)
	{
		OLED_SCLK_Clr();
		if(dat&0x80)
		{
			 OLED_SDIN_Set();
		}

		else
		{
			 OLED_SDIN_Clr();
		}

		OLED_SCLK_Set();  //时钟上升沿把数据移进8-bit的移位寄存器
		dat<<=1;
	}
	OLED_DC_Set();
}

/**************************************************************************
Function: Turn on the OLED display
Input   : none
Output  : none
函数功能：开启OLED显示
入口参数：无
返回  值：无
**************************************************************************/
//void OLED_Display_On(void)
//{
//	OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC command //SET DCDC命令
//	OLED_WR_Byte(0X14,OLED_CMD);  //DCDC ON
//	OLED_WR_Byte(0XAF,OLED_CMD);  //DISPLAY ON
//}
///**************************************************************************
//Function: Turn off the OLED display
//Input   : none
//Output  : none
//函数功能：关闭OLED显示
//入口参数：无
//返回  值：无
//**************************************************************************/
//void OLED_Display_Off(void)
//{
//	OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC command //SET DCDC命令
//	OLED_WR_Byte(0X10,OLED_CMD);  //DCDC OFF
//	OLED_WR_Byte(0XAE,OLED_CMD);  //DISPLAY OFF
//}

/**************************************************************************
Function: Screen clear function, clear the screen, the entire screen is black, and did not light up the same
Input   : none
Output  : none
函数功能：清屏函数,清完屏,整个屏幕是黑色的，和没点亮一样
入口参数：无
返回  值：无
**************************************************************************/
void OLED_Clear(void)
{
	uint8_t i,n;
	for(i=0;i<8;i++)for(n=0;n<128;n++)OLED_GRAM[n][i]=0x00;  //显存数组全置0
	OLED_Refresh_Gram();                                     //更新显示
}
/**************************************************************************
Function: Draw point
Input   : x,y: starting coordinate;t :1, fill,0, empty
Output  : none
函数功能：画点
入口参数：x,y :起点坐标; t:1,填充,0,清空
返回  值：无
**************************************************************************/
void OLED_DrawPoint(uint8_t x,uint8_t y,uint8_t t)
{
	uint8_t pos,bx,temp=0;
	if(x>127||y>63)return;          //超出范围了.
	pos=7-y/8;                      //计算页号 ，注意页号上下翻转 ，跟屏幕初始化命令0XC0有关，上下倒置了
	bx=y%8;                         //计算y 在一页 8 行里的垂直偏移位
	temp=1<<(7-bx);
	if(t)OLED_GRAM[x][pos]|=temp;   //指定像素点置1
	else OLED_GRAM[x][pos]&=~temp;  //指定像素点置0
}
/**************************************************************************
Function: Displays a character, including partial characters, at the specified position
Input   : x,y: starting coordinate;Len: The number of digits;Size: font size;Mode :0, anti-white display,1, normal display
Output  : none
函数功能：在指定位置显示一个字符,包括部分字符
入口参数：x,y :起点坐标; len :数字的位数; size:字体大小; mode:0,反白显示,1,正常显示
返回  值：无
**************************************************************************/

/* Draw one 16x16 Chinese glyph from oledfont_cn16.h into the GRAM buffer. */
void OLED_ShowCN16(uint8_t x,uint8_t y,const unsigned char *glyph)
{
	uint8_t page,col,bit,b;
	for(page=0;page<2;page++)
	{
		for(col=0;col<16;col++)
		{
			b=glyph[page*16+col];
			for(bit=0;bit<8;bit++)
			{
				OLED_DrawPoint(x+col,y+page*8+bit,(b & (1u<<bit))?1:0);
			}
		}
	}
}

/* Fixed-point float using the 8x16 ASCII font (no printf, small code size). */
void OLED_ShowFloat16(uint8_t x,uint8_t y,float v,uint8_t intDigits,uint8_t decDigits)
{
	uint8_t i,k;
	uint32_t ip,fp,scale=1,d;
	float av;
	if(intDigits==0) intDigits=1;
	if(decDigits>4) decDigits=4;
	for(i=0;i<decDigits;i++) scale*=10;
	av=(v<0.0f)? -v : v;
	ip=(uint32_t)av;
	fp=(uint32_t)((av-(float)ip)*(float)scale + 0.5f);
	if(fp>=scale) { fp-=scale; ip++; }
	OLED_ShowChar(x,y,(uint8_t)((v<0.0f)?'-':' '),16,1);
	x+=8;
	for(i=0;i<intDigits;i++)
	{
		d=1;
		for(k=0;k<(uint8_t)(intDigits-i-1);k++) d*=10;
		OLED_ShowChar(x,y,(uint8_t)('0'+((ip/d)%10)),16,1);
		x+=8;
	}
	if(decDigits>0)
	{
		OLED_ShowChar(x,y,'.',16,1);
		x+=8;
		for(i=0;i<decDigits;i++)
		{
			d=1;
			for(k=0;k<(uint8_t)(decDigits-i-1);k++) d*=10;
			OLED_ShowChar(x,y,(uint8_t)('0'+((fp/d)%10)),16,1);
			x+=8;
		}
	}
}
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t size,uint8_t mode)
{
	uint8_t temp,t,t1;
	uint8_t y0=y;
	chr=chr-' '; //ASCII 里：空格 `' '` 的编码是0x20（十进制 32），字库里下标`0` → 空格 ，每个字符减去空格就得到偏移后的索引值
  for(t=0;t<size;t++)
  {
		if(size==12)temp=oled_asc2_1206[chr][t];  //调用1206字体       一列分成2个字节
		else temp=oled_asc2_1608[chr][t];		      //调用1608字体
    for(t1=0;t1<8;t1++)                       //写列像素点
		{
			if(temp&0x80)OLED_DrawPoint(x,y,mode);  //绘制一个像素点
			else OLED_DrawPoint(x,y,!mode);
			temp<<=1;
			y++;
			if((y-y0)==size)      //控制列的像素点数
			{
				y=y0;       //回到起始行
				x++;        //写下一列
				break;
			}
		}
  }
}
/**************************************************************************
Function: Find m to the NTH power
Input   : m: base number, n: power number
Output  : none
函数功能：求m的n次方的函数
入口参数：m：底数，n：次方数
返回  值：无
**************************************************************************/
uint32_t oled_pow(uint8_t m,uint8_t n)
{
	uint32_t result=1;
	while(n--)result*=m;
	return result;
}

/**************************************************************************
Function: Display  number
Input   : x,y: starting coordinate;Len: The number of digits;Size: font size;Mode: mode, 0, fill mode, 1, overlay mode;Num: value (0 ~ 4294967295);
Output  : none
函数功能：显示n个数字
入口参数：x,y :起点坐标; len :数字的位数; size:字体大小; mode:模式, 0,填充模式, 1,叠加模式; num:数值(0~4294967295);
返回  值：无
注意：len和num的位数要匹配，不然就截取num的后len位，如len=3，num=12345，显示345
**************************************************************************/
void OLED_ShowNumber(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size)
{
	uint8_t t,temp;
	uint8_t enshow=0;             // 标志位：是否已经出现有效非0数字（用来消前导零）
	
	// 入参合法性保护
  if(len == 0 || size == 0)
      return;
	
	for(t=0;t<len;t++)            // 循环 len 次，依次取出每一位
	{
		temp=(num/oled_pow(10,len-t-1))%10;  // 取出当前最高位数字
		if(enshow==0&&t<(len-1))             // enshow=0 说明前面全是0，且不是最后一位
		{
			if(temp==0)          //第一个数字是0 ，是前导0，不显示
			{
				OLED_ShowChar(x+(size/2)*t,y,' ',size,1);    // 前导零不显示，输出空格占位
				continue;
			}else enshow=1;      //第一个数字非0，后面的都显示 

		}
	 	OLED_ShowChar(x+(size/2)*t,y,temp+'0',size,1);   //(size/2)字符宽度，每写一个数字要往后移动相应宽度
	}
}


/**************************************************************************
Function: Display string
Input   : x,y: starting coordinate;*p: starting address of the string
Output  : none
函数功能：显示字符串
入口参数：x,y :起点坐标; *p:字符串起始地址
返回  值：无
**************************************************************************/
void OLED_ShowString(uint8_t x,uint8_t y,const char *p)
{
  #define MAX_CHAR_POSX 122       // X最大显示边界，超出则换行
  #define MAX_CHAR_POSY 58        // Y最大显示边界，超出整屏清空重置坐标
  
	// 空指针保护
  if(p == NULL)
      return;
	
	while(*p!='\0')             // 遍历字符串直到结束符
  {
        if(x>MAX_CHAR_POSX){x=0;y+=16;}               // 横坐标越界：换行
        if(y>MAX_CHAR_POSY){y=x=0;OLED_Clear();}      // 纵坐标越界：全屏清屏，光标回到左上角(0,0)
        OLED_ShowChar(x,y,*p,12,1);                   //只能显示12*6字符
        x+=8;		                                      //坐标右移下一个字符
        p++;                                          // 指针下一个字符
  }
}

//short类型显示
void OLED_ShowShortNum(uint8_t x,uint8_t y,short num,uint8_t len,uint8_t size)
{
	if( num < 0 ) num=-num,OLED_ShowString(x,y,"-");   //显示符号
	else if( num>=0 ) OLED_ShowString(x,y,"+");

	OLED_ShowNumber(x+10,y,num,len,size);
}

/**************************************************************************
Function: Initialize the OLED
Input   : none
Output  : none
函数功能：初始化OLED
入口参数: 无
返回  值：无
**************************************************************************/
void OLED_Init(void)
{
	OLED_RST_Clr();
	HAL_Delay(100);
	OLED_RST_Set();

	OLED_WR_Byte(0xAE,OLED_CMD); //Close display //关闭显示,AF 打开显示
	
	OLED_WR_Byte(0xD5,OLED_CMD); //设置时钟分频因子,震荡频率
	OLED_WR_Byte(0x80,OLED_CMD); //[3:0],分频因子;[7:4],震荡频率
	
	OLED_WR_Byte(0xA8,OLED_CMD); //配置屏幕垂直扫描行数（COM 口复用率）
	OLED_WR_Byte(0X3F,OLED_CMD); //Default 0x3f(63) SSD1306 规则：复用行数 = 参数值 + 1
	
	OLED_WR_Byte(0xD3,OLED_CMD); //设置屏幕垂直显示偏移量，整体上下移动一整屏画面，全局画面上下平移，硬件 COM 扫描起始点偏移，整屏移动
	OLED_WR_Byte(0X00,OLED_CMD); //偏移参数，默认为0，范围00-3F

	OLED_WR_Byte(0x40,OLED_CMD); //Sets the number of rows to display starting line [5:0] //设置显示开始行 [5:0],行数
                               //显存输出起始行，控制 RAM 数据从第几行开始送 COM，也是整体平移，但逻辑层偏移
	OLED_WR_Byte(0x8D,OLED_CMD); //Charge pump setup //电荷泵设置
	OLED_WR_Byte(0x14,OLED_CMD); //bit2，开启/关闭电荷泵升压电路
	
	OLED_WR_Byte(0x20,OLED_CMD); //Set up the memory address mode //设置内存地址模式
	OLED_WR_Byte(0x02,OLED_CMD); //列地址模式，01；行地址模式，00；页地址模式，默认10;
	
	OLED_WR_Byte(0xA1,OLED_CMD); //段重定义设置,bit0:0,0->0;1,0->127;0xa0左右反置 0xa1正常
	OLED_WR_Byte(0xC0,OLED_CMD); //设置COM扫描方向;bit3:0,普通模式;1,重定义模式 COM[N-1]->COM0;N:驱动路数；*****0xc0上下反置 0xc8正常
	
	OLED_WR_Byte(0xDA,OLED_CMD); //设置COM硬件引脚配置
	OLED_WR_Byte(0x12,OLED_CMD); //[5:4]配置P41，Bit5：COM Left/Right Remap（COM 左右重映射）Bit4：COM Alternate Mode（COM 左右交替）

	OLED_WR_Byte(0x81,OLED_CMD); //Contrast Settings //对比度设置
	OLED_WR_Byte(0xEF,OLED_CMD); //1~255;默认0X7F (亮度设置,越大越亮)
	
	OLED_WR_Byte(0xD9,OLED_CMD); //Set the pre-charging cycle //设置预充电周期
	OLED_WR_Byte(0xf1,OLED_CMD); //高 4 位 Bit7~Bit4：PHASE2 第二阶段充电时长；低 4 位 Bit3~Bit0：PHASE1 第一阶段充电时长
	
	OLED_WR_Byte(0xDB,OLED_CMD); //设置VCOMH 消隐电压倍率；VCOMH 是 COM 行关闭时的基准负压，用于消除像素残影、抑制屏幕闪烁：
	OLED_WR_Byte(0x30,OLED_CMD); //[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

	OLED_WR_Byte(0xA4,OLED_CMD); //全局显示开启;bit0:1,开启;0,关闭;(白屏/黑屏)
	OLED_WR_Byte(0xA6,OLED_CMD); //设置显示方式;bit0:1,反相显示;0,正常显示
		
	OLED_WR_Byte(0xAF,OLED_CMD); //Open display //开启显示
	OLED_Clear();
}

/**************************************************************************
Function: Set the coordinates (position) displayed on the screen.
Input   : x, y: starting point coordinates
Output  : none
函数功能：设置汉字在屏幕上显示的坐标（位置）
入口参数: x,y :起点坐标
返回  值：无
**************************************************************************/
//void OLED_Set_Pos(unsigned char x, unsigned char y)
//{
//	 OLED_WR_Byte(0xb0+y,OLED_CMD);
//	 OLED_WR_Byte(((x&0xf0)>>4)|0x10,OLED_CMD);
//	 OLED_WR_Byte((x&0x0f),OLED_CMD);
//}


//浮点数分解
static int* FenJie_float(const float fudian)
{
	static int tmp[3];
	float temp;
	temp = fudian;
	if(temp<0) temp = -temp,tmp[2]=-1;  //p[2]符号位
	else tmp[2]=1;
	
	tmp[0] = (int)temp; 		      	//获取整数部分
	tmp[1] = (temp - tmp[0])*1000;  //小数部分，取3位小数
	return tmp;
}



//函数功能：OLED显示浮点数
//入口参数：需要显示的浮点数、x坐标、y坐标、整数显示的个数、小数显示的个数
//返回值：无
void OLED_ShowFloat(uint8_t show_x,uint8_t show_y,const float needtoshow,uint8_t zs_num,uint8_t xs_num)
{
	static int* p;
	p = FenJie_float(needtoshow);
	//符号位显示
	if(p[2]>0) OLED_ShowChar(show_x,show_y,'+',12,1);	 
	else OLED_ShowChar(show_x,show_y,'-',12,1); 		 
	//整数部分
	OLED_ShowNumber(show_x+8,show_y,p[0],zs_num,12); 
	//小数点
	OLED_ShowChar(show_x+6+8*zs_num,show_y,'.',12,1);
	//小数部分
	if(p[1]<100)//需要补零
	{	
		if(xs_num==3)
		{
			OLED_ShowNumber(show_x+12+8*zs_num,show_y,0,1,12);	 //补第一个0
			if(p[1]>=10)		
				OLED_ShowNumber(show_x+18+8*zs_num,show_y,p[1],2,12);	 //后两位不需要补0
			else
				OLED_ShowNumber(show_x+18+8*zs_num,show_y,0,1,12), //补第二个0
				OLED_ShowNumber(show_x+24+8*zs_num,show_y,p[1],1,12);
		}
		else
		{
			if(p[1]>=0&&p[1]<100)
				OLED_ShowNumber(show_x+12+8*zs_num,show_y,0,1,12);	 //补第一个0
				OLED_ShowNumber(show_x+18+8*zs_num,show_y,p[1]/10,1,12);
		}
	}
	else
	{
		if(xs_num==3)
		{
			OLED_ShowNumber(show_x+12+8*zs_num,show_y,p[1],3,12);
		}	
		else
			OLED_ShowNumber(show_x+12+8*zs_num,show_y,p[1]/10,2,12);
	}
}



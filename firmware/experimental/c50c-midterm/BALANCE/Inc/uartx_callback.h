#ifndef __UARTX_CALLBACK_H
#define __UARTX_CALLBACK_H 

#include "system.h"

//机器人接收控制命令的数据长度
#define RECEIVE_DATA_SIZE 11

//机器人接收控制命令的结构体
typedef struct _RECEIVE_DATA_  
{
	unsigned char buffer[RECEIVE_DATA_SIZE];
	struct _Control_Str_
	{
		unsigned char Frame_Header; //1 bytes //1个字节
		float X_speed;	            //4 bytes //4个字节
		float Y_speed;              //4 bytes //4个字节
		float Z_speed;              //4 bytes //4个字节
		unsigned char Frame_Tail;   //1 bytes //1个字节
	}Control_Str;
}RECEIVE_DATA;


#define C10B_HEAD1 0xCC
#define C10B_HEAD2 0xDD
#pragma pack(1) 
typedef struct {
	uint8_t Head1;            // [0]  0xCC
	uint8_t Head2;            // [1]  0xDD
	uint8_t k210_alive;       // [2]  1 = K210 frame seen within 1 s
	uint8_t flags;            // [3]  bit0 obstacle, bit1 front scan valid, bit2 k210
	uint16_t k210_cx;         // [4~5]
	uint16_t k210_size;       // [6~7]
	uint16_t distance;        // [8~9]   front-zone minimum, mm
	uint16_t distance_min;    // [10~11] minimum of all servo sectors, mm
	short temperature;        // [12~13] 0.01 C
	short pitch;              // [14~15] 0.01 deg
	short roll;               // [16~17] 0.01 deg
	short yaw;                // [18~19] 0.01 deg
	int8_t servo_deg;         // [20] current scan sector
	uint8_t scan_mask;        // [21] bit n = sector n valid
	uint8_t BCCcheck;         // [22] XOR of bytes 0..21
}C10B_Recvmsg;
#pragma pack() 

extern C10B_Recvmsg c10b_recv;
extern u8  c10b_active_flag;
extern u32 c10b_active_cnt;
extern volatile u32 c10b_rx_bytes;
extern volatile u32 c10b_head_ok;
extern volatile u32 c10b_frame_ok;
extern volatile u32 c10b_frame_bad;

//内部函数
static float XYZ_Target_Speed_transition(u8 High,u8 Low);
static u8 AT_Command_Capture(u8 uart_recv);
static void _System_Reset_(u8 uart_recv);

#endif


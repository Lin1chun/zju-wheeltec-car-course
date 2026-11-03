#ifndef __BALANCE_H
#define __BALANCE_H			  	 
#include "system.h"

#include "bsp_gamepad.h"

//RTOS��������
#define BALANCE_TASK_PRIO		4             //Task priority //�������ȼ�
#define BALANCE_STK_SIZE 		512           //Task stack size //�����ջ��С
#define BALANCE_TASK_RATE       RATE_100_HZ   //����Ƶ��

//�����˿�����ز���
typedef struct{
	u8 ControlMode;       //�����˿���ģʽ_(����˵��:���û�ֱ�Ӳ�������,�������ֱ�Ӹ�ֵ����,���ɶ�)
	u8 FlagStop;          //�������Ƿ�ʹ�ܱ�־λ,1����������ʧ��,0����ʹ��_(����˵��:���û�ֱ�Ӳ�������,�������ֱ�Ӹ�ֵ����,���ɶ�)
	u8 command_lostcount; //�����˶�ʧ�����������,��ʧ�����һ��ʱ���ֹͣ�����˵����˶�
	u8 SoftWare_Stop;     //������������ͣ,����1��ͣ,0ʹ��. ��ΪԤ������,����Ҫʹ��ʱ�����ø�ֵΪ1����
	float Vx;            //������x��Ŀ���ٶ�
	float Vy;            //������y��Ŀ���ٶ�
	float Vz;            //������z��Ŀ���ٶ�
	float smooth_Vx;     //�������ٶ�ƽ�����ֵ
	float smooth_Vy;     //�������ٶ�ƽ�����ֵ
	float smooth_Vz;     //�������ٶ�ƽ�����ֵ
	
	float smooth_MotorStep;   //�����˵���ٶ�ƽ������ֵ
	float smooth_ServoStep;   //�����˶���ٶ�ƽ������ֵ
	float smooth_Servo;  //�������ٶ�ƽ�����ֵ
	u8 ServoLow_flag;    //���ٶ��ģʽ,���ϵ�ʱ�Լ�С��ʧ�ܺ���λ�÷����ı�ʱʹ��(�����䰢����).
	
	float rc_speed;      //������ң���ٶȻ�׼,Ĭ��500 mm/s
	float limt_max_speed;//����������������ٶ�����.��λm/s,��׼����Ĭ������3.5 m/s��ʵ���ϱ�׼�����˴ﲻ������ٶ�.
	uint32_t LineDiffParam;//��ƫϵ����0-100�ɵ���

}ROBOT_CONTROL_t;

//�������Լ���ر���
typedef struct{
	u8 errorflag; //�Լ챨����־λ
	u8 check_end; //�Լ������־λ
	int check_a;  //4�����ӱ������Լ���ر���
	int check_b;
	int check_c;
	int check_d;
	u8 DeepCheck; //������ȼ���־λ
}ROBOT_SELFCHECK_t;

//������ʧ��ʱ�ĵ��״̬. 
enum{
	UN_LOCK = 0, //�������
	LOCK    = 1  //�������
};

//������פ��ģʽ��ر���
typedef struct{
	u8 wait_check;  //�ȴ����pwm�����־λ
	u16 timecore;   //��ʱ����
	u8 start_clear; //�Ƿ�ʼ�������PWM
	u8 clear_state; //pwm���������״̬.ʹ�����λ�͵�4λ:��4λ����4�����,���λΪ���A,��������. ���λ������������Ƿ����.
}ROBOT_PARKING_t;

//����ʽPI�������ṹ��
typedef struct{
	float Bias;    // ��Ŀ��ֵƫ��
	float LastBias;// ��һ�ε�ƫ��
	int Output;    // ���
	int kp;        // kpֵ
	int ki;        // kiֵ
}PI_CONTROLLER;

//���䰢����ר�ñ���,���ڶ�����������ܵ�ʵ��
typedef struct{
	uint8_t UnLock;      //���������ģʽ��־λ
	uint8_t wait_Calib;  //�ȴ�У׼
	float UnLock_Pos;   //���������ģʽ�¼�¼��λ��
	float UnLock_Target;//������ģʽ�µı��浽��Ŀ��ֵ
	uint16_t UnLock_Output;//���������ģʽʱ�Ķ��PWMֵ,��������ʾʹ��.
}AKM_SERVO_UNLOCK_t;

extern ROBOT_CONTROL_t robot_control;
extern ROBOT_SELFCHECK_t robot_check;
extern AKM_SERVO_UNLOCK_t ServoState;

//�����˿��Ʒ�ʽ�������ȡ
enum
{
	_ROS_Control   =  (1<<0), //ROS����
	_PS2_Control   =  (1<<1), //PS2����
	_APP_Control   =  (1<<2), //APP����
	_RC_Control    =  (1<<3), //��ģң�ؿ���
	_CAN_Control   =  (1<<4), //CANͨ�ſ���
	_USART_Control =  (1<<5), //���ڿ���
	_K210_Control  =  (1<<6), //K210Ѳ�߿���
};
//���û����˿��Ʒ�ʽ
#define Set_Control_Mode(mask)  (robot_control.ControlMode |= (mask), robot_control.ControlMode &= (mask)) 
//��ȡ�����˿��Ʒ�ʽ
#define Get_Control_Mode(mask)  (robot_control.ControlMode & (mask))

//�������
extern SYS_VAL_t SysVal;
extern ROBOT_CONTROL_t robot_control;
extern PI_CONTROLLER PI_MotorA,PI_MotorB,PI_MotorC,PI_MotorD,PI_Servo;



//=================================================================
//��������ʻ����
typedef struct
{
	// ԭ�е����Ӳ������...
  float total_dist;       // �ۼ���ʻ�ܾ��� m
  float target_dist;      // Ŀ����� m
  uint8_t dist_run_en;   	// ��������ʹ�� 1���� 0�ر�
} ROBOT_DISTANCE_t;

//=================================================================



//���⺯��
void Balance_task(void *pvParameters); //��������
//void Akm_ReadServo_Param(void);//��Flash��ȡ�������������Ϣ
void PI_Controller_Init(PI_CONTROLLER* p,int kp,int ki); //������ʼ������
void ROBOT_CONTROL_t_Init(ROBOT_CONTROL_t* p);
void  Set_Robot_PI_Param(int kp,int ki); //PI���������ò���
float rad_to_angle(const float rad);  //�Ƕ��뻡�Ȼ�ת
float angle_to_rad(const float angle);//�Ƕ��뻡�Ȼ�ת
float Akm_Vz_to_Angle(float Vx,float Vz);//����������Ŀ���ٶ�ת��Ϊ��ǰ��ת��
short get_ServoPWM(short TmpPos);//���ݻ�������ݹ�������PWM��ֵ
float target_limit_float(float insert,float low,float high);
void FlashParam_Read(void);

//�ڲ�ʹ�ú���
static void ROBOT_SELFCHECK_t_Init(ROBOT_SELFCHECK_t* p); //��ʼ����

static void PI_Controller_Reset(PI_CONTROLLER *p); //PI������
static void PI_SetParam(PI_CONTROLLER* p,int kp,int ki);
static int Incremental_MOTOR(PI_CONTROLLER* p,float current,float target);
static int Incremental_Servo(PI_CONTROLLER* p,float current,float target);
static uint8_t PI_Clear_Output(PI_CONTROLLER* p);

static void InverseKinematics_akm(float Vx,float Vz); //�˶�ѧ������
static void InverseKinematics_diff(float Vx,float Vz);
static void InverseKinematics_mec(float Vx,float Vy,float Vz);
static void InverseKinematics_4wd(float Vx,float Vz);
static void InverseKinematics_omni(float Vx,float Vy,float Vz);

static void Drive_Motor(float T_Vx,float T_Vy,float T_Vz); //������
static void ResponseControl(void);
static void UnResponseControl(uint8_t mode);
static void Set_Pwm(int m_a,int m_b,int m_c,int m_d,int servo);
static u8 Turn_Off(void);
static void Get_APPcmd(void);
static void Remote_Control(void);
static void PS2_control(void);
static void robot_mode_check(void);
static void Get_Robot_FeedBack(void);
static void Servo_UnLock_Check(uint8_t car_stopflag);

static void Robot_ParkingCheck(void); //״̬�����
static void UserKey_Scan(u16 rate);
static void Charger_DevCheck(void);

//static uint8_t Akm_SaveServo_Param(uint8_t *flag); //������
static float Vel_SmoothControl(float now_speed,float targetSpeed,float step);

static int target_limit_int(int insert,int low,int high);
static int Slide_Mean_Filter(int data);
static uint8_t Deep_SelfCheck( u16 RATE );
uint8_t ValChangeCheck(const uint16_t rate,const short checkval,const uint8_t changeEva);
static uint8_t FlashParam_Save(uint8_t *flag);
static void K210_control(void);

//=================================================================
#ifndef MIDTERM_ENABLE
#define MIDTERM_ENABLE 1   /* 0 = only show sensor/attitude pages, car stays still */
#endif

enum {
	MID_STATE_IDLE = 0,
	MID_STATE_OUT,
	MID_STATE_PAUSE,
	MID_STATE_TURN,
	MID_STATE_BACK,
	MID_STATE_DONE
};

void Start_Midterm(void);
void Midterm_Control(void);
u8   Midterm_State(void);
float Midterm_Distance(void);
float Midterm_AvgSpeed(void);
float Midterm_Obstacle_m(void);

//static float XYZ_Target_Speed_Distance_transition(u8 High,u8 Low);
//=================================================================

#endif  


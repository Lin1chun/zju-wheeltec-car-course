import sensor , image , time , lcd
from fpioa_manager import fm  #串口引脚映射
from machine import UART  #从 machine 包导入 UART 类，K210 有 3 路硬件串口：UART.UART1 / UART2 / UART3，本程序用的是 UART1

from board import board_info
from maix import GPIO

# 导入数据打包和解析类方法
import struct
from collections import OrderedDict # 用于创建有序字典

# 定时器模块
from machine import Timer



"""
对接受的数据解包类
"""
class Uart_RecvPack():  #接收端解包类，使用BCC校验接收的包有没有出错，校验通过才还原成Python 数值。这个功能目前没有用
    def __init__(self,packmsg,dataformat):

        # 传输的数据包
        self.msg = packmsg

        # 传输的数据格式
        self.recvformat = dataformat

        # 要传输的数据长度
        self.data_len = struct.calcsize(self.recvformat)

    # BCC校验函数
    def calculate_BCC(self,datalist,datalen):
        ref = 0
        for i in range(datalen):
            ref = (ref^datalist[i])  #BCC校验就是把指定范围内的字节逐个做"异或(XOR)"，结果取低 8 位。
        return ref&0xff

    # 接收读取到的数据列表并解包
    def unpack_value(self,datalist):
        try:
            bccref = self.calculate_BCC( datalist , self.data_len-2 ) # 校验范围：整包长度 - 2（即扣掉"校验位本身"和"帧尾"两个字节）
            if bccref == datalist[self.data_len-2]:
                tmpmsg = bytes(datalist)
                tmpmsg = struct.unpack(self.recvformat,tmpmsg)
                self.msg.update( zip(self.msg.keys(),tmpmsg ) )
            else:
                return False      # 校验不通过：说明这一包数据不可信，直接丢弃
        except Exception as e:
            return False  # 字节数不够等情况也会走到这里，同样视为失败

        return True

"""
将需要发送的数据打包 类
"""
class Uart_SendPack(): #发送端打包类
    def __init__(self,packmsg,dataformat):

        # 传输的数据包
        self.msg = packmsg

        # 传输的数据格式
        self.sendformat = dataformat

        # 要传输的数据长度
        self.data_len = struct.calcsize(self.sendformat)


    # BCC校验函数
    def calculate_BCC(self,datalist,datalen):  #BCC校验就是把指定范围内的字节逐个做"异或(XOR)"，结果取低 8 位。
        ref = 0
        for i in range(datalen):
            ref = (ref^datalist[i])
        return ref&0xff

    # 将要打包的数据进行BCC校验,并返回最终BCC校验的值
    def pack_BCC_Value(self):
        tmp_list = list( self.msg.values() ) # 将字典的值取出转换成列表

        tmp_packed = struct.pack(self.sendformat,*tmp_list) # 根据指定的数据类型对列表值进行自动打包

        # 自动打包后的数值，进行BCC校验，获得最终需要发送的BCC校验值
        # 注意：这里只校验前 (总长-2) 个字节，也就是不含"校验位"和"帧尾"这两个，
        #       所以此刻 msg['BccCheck'] 里装的旧值是多少，都不影响计算结果。
        return self.calculate_BCC(tmp_packed,len(tmp_packed)-2)

    # 获取最终要发送的数据列表
    def get_Pack_List(self):
        tmplist = list(self.msg.values())# 将字典的值取出,转换成列表
        # struct.pack 返回的是 bytes 类型，这正是 uart.write() 需要的参数类型，
        # 不需要再做 str() 或 encode() 转换。
        return struct.pack(self.sendformat,*tmplist)

""" 创建需要往stm32端发送数据的数据包 """
# 需要发送的数据(如果需要增删数据,则修改此处以及修改数据格式,同时在32端也对应增删即可)！！！
# =============================================================================
# 串口通信协议（帧结构）—— 发送方向：K210 → STM32
# 这里用 OrderedDict 是为了"保证顺序"：字典的取值顺序必须和下面的格式串严格对应。
# -----------------------------------------------------------------------------
#  字节序号    字段        类型       说明
#   [0]      Head        uint8     帧头，固定 0xCC（STM32 靠它判断"一包开始了"）
#   [1][2]   Cam_W       uint16    图像宽 320，小端存储
#   [3][4]   Cam_H       uint16    图像高 240，小端存储
#   [5][6]   follow_x    uint16    目标中心 x 坐标（像素）
#   [7][8]   follow_y    uint16    目标中心 y 坐标（像素）
#   [9][10]  size        uint16    目标矩形面积 = 宽 × 高
#   [11]     BccCheck    uint8     前 11 个字节的异或校验值
#   [12]     End         uint8     帧尾，固定 0xDD
#                                              合计 13 字节
# -----------------------------------------------------------------------------
# 提醒：Cam_W / Cam_H 是写死的 320/240，如果改了摄像头分辨率，这里也要同步改。
# =============================================================================
send_pack1_msg = OrderedDict([
        ('Head',0xCC),   # 帧头           uint8_t类型
        ('Cam_W', 320),  # 相机的像素宽度  uint16_t 类型
        ('Cam_H', 240),  # 相机的像素长度  uint16_t 类型
        ('follow_x',0),  # 需要跟踪的点x   uint16_t 类型
        ('follow_y',0),  # 需要跟踪的点y   uint16_t 类型
        ('size',0),      # 目标矩形面积    uint16_t 类型
        ('BccCheck',0),  # 数据BCC校验位   uint8_t类型
        ('End',0xDD)     # 帧尾           uint8_t类型
])

# 数据格式 <代表小端模式, B代表uint8_t类型,5H代表5个uint16_t类型,2B代表2个uint8_t类型
send_pack1_format = "<B5H2B"  #struct 模块的格式化字符串，用于把多个变量打包成二进制字节流

#实例化数据打包对象
send_pack1 = Uart_SendPack(send_pack1_msg,send_pack1_format)

# 更新需要发送的数据并返回打包结果,将结果直接发送到stm32端即可
def update_sendpack1_data(follow_x,follow_y,size):
    global send_pack1
    send_pack1.msg['follow_x'] = follow_x # 写入本次识别到的目标 x
    send_pack1.msg['follow_y'] = follow_y # 写入本次识别到的目标 y
    send_pack1.msg['size'] = size         # 写入本次识别到的目标面积
    # ★顺序很重要：必须"先把数据填好，再算 BCC"，否则算出来的校验值对不上。
    send_pack1.msg['BccCheck'] = send_pack1.pack_BCC_Value()
    # 返回打包好的 13 字节 bytes，可直接交给 uart1.write()  
    return send_pack1.get_Pack_List()
""" 创建需要往stm32端发送数据的数据包 END """


# 各传感器初始化
lcd.init()
sensor.reset()
sensor.reset(freq=24000000, dual_buff=1)
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)

sensor.set_auto_gain(False)      # 颜色跟踪必须关闭自动增益
sensor.set_auto_whitebal(False)  # 颜色跟踪必须关闭白平衡

sensor.skip_frames(time = 1000)

#帧率时钟
clock = time.clock()

# 指定串口引脚并构造UART对象
# =============================================================================
# ★★★★ 串口初始化：整个串口功能最关键的三行 ★★★★
# K210 用串口分两步走：① 先用 FPIOA 声明引脚功能；② 再构造 UART 对象。两步缺一不可。
# -----------------------------------------------------------------------------
# 引脚分配（可对照 STM32 端的接线）：
#   IO0 → UART1_RX ：K210 的"接收脚"，应接 STM32 的 TX
#   IO1 → UART1_TX ：K210 的"发送脚"，应接 STM32 的 RX
#   ★ 串口要交叉接线（TX→RX、RX→TX），并且两边必须共地(GND)！
#   ★ 若程序跑起来但 STM32 收不到数据，优先检查：这两个引脚是否被其它外设占用、
#     接线是否交叉、电平是否匹配。
# -----------------------------------------------------------------------------
fm.register(0, fm.fpioa.UART1_RX)     # 把物理引脚 IO0 复用为 UART1 的接收功能
fm.register(1, fm.fpioa.UART1_TX)     # 把物理引脚 IO1 复用为 UART1 的发送功能
# 构造 UART 对象：
#   第 1 个参数 UART.UART1  —— 指定使用 1 号硬件串口（也可换成 UART2/UART3，但引脚要跟着改）
#   第 2 个参数 115200      —— 波特率，★必须与 STM32 端设置完全一致，否则收到全是乱码
# 其余参数（8 位数据位 / 无校验 / 1 位停止位）为默认值，两端用默认即可。
uart1 = UART(UART.UART1, 115200)

color_thresholds = [
    (0, 15, -128, 127, -128, 127) # 黑色标准阈值
]


# 追踪的颜色选择
cmd_color = 0

# 辅组接收数据的变量
Last_recv32 = None


# 感兴趣的区域设置:x,y,w,h
# 像素共有：320x240
roi = (40, 200, 240, 40)


g_follow_blob = None
g_last_blob = None
g_errortime = 0


# 定时器中断回调函数
#   这个函数是"串口发送的实际触发者"：
#   定时器每 50ms 自动进来一次，把最新的识别结果打包并通过串口发出去。
#   也就是说串口发送频率 = 1000ms / 50ms = 20Hz（每秒发 20 帧）。
def timer_callback(timer):
    global g_follow_blob
    global g_last_blob
    global g_errortime

    # 最大最小面积定义(通过实测获得)
    min_area = 500
    max_area = 2500

    sendcx = 0
    sendcy = 0
    sendarea = 0
    try:
        # 将最大物体的中心坐标发送到stm32
        if g_follow_blob!=None:
            # 计算识别到的面积
            sendarea = g_follow_blob.rect()[2]*g_follow_blob.rect()[3]
            sendcx = g_follow_blob.cx()
            sendcy = g_follow_blob.cy()
        # =====================================================================
        # ★★★★ 真正执行发送的那一步 ★★★★
        # update_sendpack1_data() 先把 x / y / 面积填进数据包并算好 BCC，
        # 返回的是 13 字节的 bytes；uart1.write() 把这串字节原样从 IO1(TX) 推出去。
        # ★ uart1.write() 的参数必须是 bytes / bytearray，不能传字符串(str)。
        # ★ 这一句在"定时器中断"里执行，所以它不会阻塞主循环的图像处理。
        # =====================================================================
        send_to_32 = update_sendpack1_data(sendcx,sendcy,sendarea)  #更新数据
        uart1.write(send_to_32)   #发送数据

    except Exception as e:
        print(e)


# 创建一个定时器,period为定时时间,单位ms.
# period=50 同时决定了"串口发送节拍"：每 50ms 触发一次 timer_callback。
timer = Timer(Timer.TIMER0, Timer.CHANNEL0, mode=Timer.MODE_PERIODIC, period=50, callback=timer_callback)

# 启动定时器
timer.start()



while True:

    #用于计算帧率的函数，这里表示开始计时
    clock.tick()

    try:
        #从传感器捕获一张图像
        img = sensor.snapshot()

        # 框选感兴趣的区域用于提示
        img.draw_rectangle(roi, color=(255 ,255,255),thickness = 3)

        # 根据色块阈值找出感兴趣区域的色块
        blobs = img.find_blobs([color_thresholds[cmd_color]], pixels_threshold=100, area_threshold=100, merge=True, margin=10,roi = roi)

        max_box_size = 0
        max_blob = None

        if blobs:

            max_blob = blobs[0]
            for blob in blobs:
                # 面积计算：blob.rect()[2]*blob.rect()[3]
                if blob.rect()[2]*blob.rect()[3] > max_blob.rect()[2]*max_blob.rect()[3]:
                    max_blob = blob

            # 框选出识别到的色块
            g_follow_blob = max_blob
            img.draw_rectangle(g_follow_blob.rect(), color=(255 , 0,0),thickness = 3)
            img.draw_cross(g_follow_blob.cx(), g_follow_blob.cy(), color=(255 , 0,0))


        # 显示帧率
        fps = clock.fps()
        img.draw_string(0, 0, "%2.1fps" %(fps), color=(0, 60, 255), scale=2.0)

        if max_blob!=None:
            img.draw_string(0, 20, "size:%d" %(g_follow_blob.rect()[2]*g_follow_blob.rect()[3]), color=(255, 60, 255), scale=2.0)


        #显示在LCD上
        lcd.display(img)

    except Exception as e:
        print(e)


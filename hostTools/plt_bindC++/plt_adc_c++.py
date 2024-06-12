
import socket
import struct
import threading
import time
import signal
import matplotlib.pyplot as plt
import numpy as np

import sys
sys.path.append("./build")
import netOcd

STEP=1.5
BUFSIZE=10000
# def tcp_send_command_thread(stop_event):
#     # Configure Socket
#     with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
#         # Connect to motor
#         s.connect(('192.168.1.102', 1000))

#         # Compose String to send to the drive
#         st = '"<control pos=\"15000\" frequency=\"20\" torque=\"200\" mode=\"135\" offset=\"0\" phase=\"0\" />"'
#         s.sendall(st.encode('ascii')) # send XML command  
        
#         # The main thread can continue executing other tasks
#         while not stop_event.is_set():
#             time.sleep(1)

def receive_packet(s,ring_buffer):
   buffer = b''  # 初始化数据缓冲区
   frames_received = 0
   start_time = time.time()
   adc_values = []
   dacp_values = []
   dacn_values = []
   showAdcMeanList = []
   index = 0
   times = 0
  #  plt.ion() # <-- work in "interactive mode"
  #  fig, ax = plt.subplots()
  #  fig.canvas.set_window_title('Live Chart')
  #  ax.set_title("detect velocity adc val")
  #  ax.set_ylim(1000, 3000)  # 设置 y 坐标范围
   while True:
       data = s.recv(1024)  # 一次性读取更多数据
       if not data:
           break
       buffer += data

       while b'#' in buffer:  # 在缓冲区中查找 #
           start_index = buffer.index(b'#')  # 找到 # 的位置
           if len(buffer) - start_index >= 11:  # 确保剩余数据足够解析一个完整包
               packet = buffer[start_index:start_index + 11]  # 提取一整个包的数据
               buffer = buffer[start_index + 11:]  # 更新缓冲区数据
               
               try:
                   adc, dacP, dac_n = struct.unpack('<HLL', packet[1:])
                   cppResult = netOcd.parsePkg(packet)
                   #print(f"Received Data: ADC={adc}, DAC_P={dac_p}, DAC_N={dac_n}")
                #    print(f"c++ data:{cppResult[0]},{cppResult[1]},{cppResult[2]}")
                   adc_values.append(cppResult[0])
                   dacp_values.append(cppResult[1])
                   dacn_values.append(cppResult[2])
                   frames_received += 1
               except struct.error:
                   print("Error parsing packet")
                   break
           else:
               #print("Incomplete packet received")
               break

       # 计算每秒钟帧数
       elapsed_time = time.time() - start_time
       if (elapsed_time >= STEP) and (len(adc_values) != 0):
           meanADC = sum(adc_values) / len(adc_values)
           meanDAP = sum(dacp_values) / len(dacp_values)
           meanDAN = sum(dacn_values) / len(dacn_values)
        #    meanDAP = meanDAP%1000
           #meanDAP和meanDAN显示小数点后固定长度，长度为10位,不足的用0补齐
           print(f"Frames received in the last second: {frames_received}, meanADC={meanADC:010.10f}, meanDAP={meanDAP:010.10f}, meanDAN={meanDAN:010.10f}")
           frames_received = 0
           start_time = time.time()
           ring_buffer[index] = [times, meanADC, meanDAP, meanDAN]
           index = (index + 1) % len(ring_buffer)
           times = times + 1
           adc_values = []
           dacp_values = []
           dacn_values = []
   remaining_data = buffer  # 保留超出11个字节的部分


def receive_tcp_data(udp_port, stop_event, ring_buffer):
    HOST = '192.168.0.61'  # 远程主机的IP地址
    PORT = 6100  # 远程主机的端口号

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
       s.connect((HOST, PORT))
       print('Connected to', HOST)
       while True:
        data = receive_packet(s,ring_buffer)
     

def keyboard_interrupt_handler(signal, frame):
    print("Keyboard interrupt received. Stopping the program.")
    # Set the stop event to signal the thread to exit
    stop_event.set()
    exit(0)

if __name__ == '__main__':
    stop_event = threading.Event()  # create an event object

    # Initialize the ring buffer
    ring_buffer = [[0, -1, 0, 0] for i in range(BUFSIZE)]
    distribution_data = []
    # create a thread that receives on UDP port 1001    
    udp_receive_thread = threading.Thread(target=receive_tcp_data, args=(1001, stop_event, ring_buffer))
    udp_receive_thread.start()

    # create a thread that can send commands to the motor    
    # tcp_send_command_thread = threading.Thread(target=tcp_send_command_thread, args=(stop_event,))
    # tcp_send_command_thread.start()

    # Install the keyboard interrupt handler
    signal.signal(signal.SIGINT, keyboard_interrupt_handler)

    # Initialize the plot

    plt.ion()
    #fig, ax = plt.subplots()
    fig, (ax1, ax2, ax3) = plt.subplots(3, 1)  # 创建两个子图
    ax1.set_xlabel('Time') # Set the x-axis label
    ax2.set_xlabel('Time') # Set the x-axis label
    ax1.set_ylim(0.0, 3.3)  # 设置 y 轴范围
    ax1.set_title('Plot of CP ADC')  # 设置子图标题
    ax2.set_ylim(0.0, 3.3)  # 设置 y 轴范围
    ax2.set_title('Plot of dac_non_negative')  # 设置子图标题
    ax3.set_xlim(1.536 , 1.538)
    plt.tight_layout()  # 调整图形布局，防止重叠


    fnum = 0
    # The main thread can continue executing other tasks
    while True:
        
        # Get the latest data from the ring buffer
        time_values = [entry[0] for entry in ring_buffer]
        y_values = [entry[1] for entry in ring_buffer]
        dac_values_p = [entry[2] for entry in ring_buffer]
        dac_values_n = [entry[3] for entry in ring_buffer]
        non_negative_indices = [i for i, val in enumerate(y_values) if val != -1]
        y_non_negative =   [y_values[i] for i in non_negative_indices]
        dac_non_negative = [dac_values_p[i] for i in non_negative_indices]
        dac_negative =     [dac_values_n[i] for i in non_negative_indices]
        if y_non_negative:
        #   ax.clear()  # Clear the previous plot
        #   ax.plot(y_non_negative, color='r', label='cp_values')  # Plot the non-negative y_values
        #   ax.plot(dac_non_negative, color='b', label='dac_values')  # Plot the non-negative dac_values
        #   ax.legend()  # Show the legend
        #   ax.set_ylim(0.5, 2)  # 设置 y 坐标范围
        #   print("Displaying non-negative values.")
            
    
            # 第一个子图显示 y_non_negative
            ax1.plot(y_non_negative, color='r', label='cp_values')  # 绘制非负的 y 值
            #ax1.legend()  # 显示图例
            # ax1.set_ylim(0.5, 2)  # 设置 y 轴范围
            # ax1.set_title('Plot of y_non_negative')  # 设置子图标题
    
            # 第二个子图显示 dac_non_negative
            ax2.plot(dac_non_negative, color='b', label='dac_p')  # 绘制非负的 dac 值
            ax2.plot(dac_negative,     color='r', label='dac_n')  # 绘制非负的 dac 值
            #ax2.legend()  # 显示图例
            # ax2.set_ylim(0.5, 3.3)  # 设置 y 轴范围
            # ax2.set_title('Plot of dac_non_negative')  # 设置子图标题
    
            # plt.tight_layout()  # 调整图形布局，防止重叠
            # plt.show()  # 显示图形



            #2024.6.11
            #show distribution of data
            distribution_data.append(dac_non_negative[-1])
            ax3.cla()  # Clear the previous plot
            ax3.hist(distribution_data, bins=200, density=True, alpha=0.6, color='g')
            mean= np.mean(distribution_data)
            ax3.axvline(mean, color='b', linestyle='dashed', linewidth=2, label=f'均值={mean:.2f}')
            #ax3.xlabel('data')
            #ax3.ylabel('happend')
            #ax3.title('disturbution of data')
            #ax3.legend()
            #ax3.show()

            # Show the plot
            plt.draw()
            plt.pause(0.1)  # Give the plot time to refresh
            
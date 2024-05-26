import socket
import struct
import time
import matplotlib.pyplot as plt
#import pandas as pd


def receive_packet(s):
   buffer = b''  # 初始化数据缓冲区
   frames_received = 0
   start_time = time.time()
   adc_values = []
   showAdcMeanList = []
   # create plot
   plt.ion() # <-- work in "interactive mode"
   fig, ax = plt.subplots()
   fig.canvas.set_window_title('Live Chart')
   ax.set_title("detect velocity adc val")
   ax.set_ylim(-10, 3000)  # 设置 y 坐标范围
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
                   adc, dac_p, dac_n = struct.unpack('<HLL', packet[1:])
                   #print(f"Received Data: ADC={adc}, DAC_P={dac_p}, DAC_N={dac_n}")
                   adc_values.append(adc)
                   
                   frames_received += 1
               except struct.error:
                   print("Error parsing packet")
                   break
           else:
               #print("Incomplete packet received")
               break

       # 计算每秒钟帧数
       elapsed_time = time.time() - start_time
       if elapsed_time >= 1:
           meanADC = sum(adc_values) / len(adc_values)
           print(f"Frames received in the last second: {frames_received}, meanADC={meanADC}")
           frames_received = 0
           start_time = time.time()
           showAdcMeanList.append(meanADC)
           ax.plot(showAdcMeanList, color='r')
           # show the plot
           plt.show()
           plt.pause(0.0001) # <-- sets the current plot until refreshed
           time.sleep(.1)
           adc_values = []

   remaining_data = buffer  # 保留超出11个字节的部分


def main():
   HOST = '192.168.0.83'  # 远程主机的IP地址
   PORT = 6100  # 远程主机的端口号

   with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
       s.connect((HOST, PORT))
       print('Connected to', HOST)

       while True:
           data = receive_packet(s)



if __name__ == "__main__":
   main()
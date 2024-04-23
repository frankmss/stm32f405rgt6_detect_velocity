import socket
import struct

def receive_packet(s):
   buffer = b''  # 初始化数据缓冲区
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
                   adc, dac_p, dac_n = struct.unpack('!HLL', packet[1:])
                   return adc, dac_p, dac_n
               except struct.error:
                   print("not return")
                   continue

   return None


def main():
   HOST = '192.168.0.70'  # 远程主机的IP地址
   PORT = 6100  # 远程主机的端口号

   with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
       s.connect((HOST, PORT))
       print('Connected to', HOST)

       while True:
           data = receive_packet(s)

           if data:
               adc, dac_p, dac_n = data
               print(f"Received Data: ADC={adc}, DAC_P={dac_p}, DAC_N={dac_n}")
           else:
               print("Error parsing packet, continuing...")

if __name__ == "__main__":
   main()
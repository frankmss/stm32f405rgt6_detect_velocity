cmake_minimum_required(VERSION 3.11) #这个也可以不写
project(test) #定义工程名字

#如果想要指定找的python解释器，可以设置Python_ROOT_DIR路径，如下所示
#set (Python_ROOT_DIR "/home/handsome/software/anaconda3/envs/standard310")
find_package(Python 3.9.5 COMPONENTS Interpreter Development REQUIRED)
#注意找Python包一定要放在pybind11之前，否则无法根据自己想要的版本进行正确链接
find_package(pybind11 REQUIRED) #找到依赖


pybind11_add_module(netOcd src/main.cpp)#重点
#target_link_libraries(netOcd PRIVATE -lnsl -lsocket)#
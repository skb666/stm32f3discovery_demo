# STM32F3-Discovery Demo

+ [RING_FIFO](https://github.com/skb666/RING_FIFO) 实际应用
+ [task_event](https://github.com/skb666/task_event) 实际应用
+ [xcmd](https://github.com/skb666/xcmd) 移植使用
+ USB-CDC shell
+ USB-CDC 透传 USART1/USART3
+ I2C 从机的中断方式实现（LL 库）
+ 通过串口读写 I2C
+ 串口升级协议调试
+ I2C 升级协议调试
+ shell 实现:
	- led 控制
	- i2cdetect
	- i2ctransfer
	- usart 调试助手

**cmake 编译方法**

```bash
cd bld
cmake -G "Unix Makefiles" -H. -Bbuild -DCMAKE_TOOLCHAIN_FILE=toolchains.cmake
cmake --build build -t all -- -j${nproc}

cd app
cmake -G "Unix Makefiles" -H. -Bbuild -DCMAKE_TOOLCHAIN_FILE=toolchains.cmake
cmake --build build -t all -- -j${nproc}
```

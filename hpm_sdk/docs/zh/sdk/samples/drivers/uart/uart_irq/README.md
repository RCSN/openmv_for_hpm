# 通用异步收发器UART

## 概述

**uart**通用异步收发器UART驱动示例工程展示了UART的配置使用方法，实现串口数据的接收和发送
- 示例代码中，进行UART串口资源的初始化配置，设置串口时钟及收发FIFO深度，配置中断，实现串口收据的发送与接收

## 端口设置

-  串口波特率设置为``115200bps``，``1个停止位``，``无奇偶校验位``

## 注意

- 通过串口单次输入的字符串长度不应该超过TEST_UART_MAX_BUFFER_SIZE

## 运行现象

当工程正确运行后，通过串口手动输入字符串，如：12345678909876543210，则串口终端会收到如下信息：
```console
uart driver example
fifo mode
uart will send back received characters, echo every 20 bytes
12345678909876543210
```

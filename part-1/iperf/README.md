# Iperf Example

## 关于 iperf 版本的注意事项

iperf 示例不支持标准 iperf 中的所有功能。它与 iperf 版本 2.x.兼容。

## 注意 80MHz 的 flash 频率

如果 SPI 闪存频率设置为 80MHz，iperf 可以获得更好的吞吐量，但对于 ESP-WROVER-KIT，系统可能会在 80MHz 模式下崩溃。
从电路板上移除 R140\~R145 可以解决此问题。目前，默认 SPI 频率设置为 40MHz，如果要将 SPI 闪存频率更改为 80MHz，请确保从 ESP-WROVER-KIT 中删除 R140\~R145 或使用 ESP32 DevKitC。

## 介绍

本例实现了通用性能度量工具[iPerf](https://iperf.fr/)使用的协议.
可以在运行本例的两台 ESP32 之间，或者在单个 ESP32 和运行 iPerf 工具的计算机之间测量性能

演示了测试站 TCP Tx 性能的步骤:

1. 使用“sdkconfig.defaults”构建 iperf 示例，其中包含特定于性能测试的配置

2. 以站点模式运行演示，并加入目标 AP sta ssid 密码

3. 在 AP 端以服务器身份运行 iperf
   iperf -s -i 3

4. 在 ESP32 端以客户端身份运行 iperf
   iperf -c 192.168.10.42 -i 3 -t 60

控制台输出由站点 TCP RX 吞吐量测试打印，如下所示：

> esp32> sta aptest
>
> I (5325) iperf: sta connecting to 'aptest'
>
> esp32> I (6017) event: ip: 192.168.10.248, mask: 255.255.255.0, gw: 192.168.10.1
>
> esp32> iperf -s -i 3 -t 1000
>
> I (14958) iperf: mode=tcp-server sip=192.168.10.248:5001, dip=0.0.0.0:5001, interval=3, time=1000
>
> Interval Bandwidth
>
> esp32> accept: 192.168.10.42,62958
>
> 0- 3 sec 8.43 Mbits/sec
>
> 3- 6 sec 36.16 Mbits/sec
>
> 6- 9 sec 36.22 Mbits/sec
>
> 9- 12 sec 36.44 Mbits/sec
>
> 12- 15 sec 36.25 Mbits/sec
>
> 15- 18 sec 24.36 Mbits/sec
>
> 18- 21 sec 27.79 Mbits/sec

测试“station/soft AP”TCP/UDP RX/TX 吞吐量的步骤与 station TCP TX 中的测试步骤类似。

See the README.md file in the upper level 'examples' directory for more information about examples.

```

 ==================================================
 |       Steps to test WiFi throughput            |
 |                                                |
 |  1. Print 'help' to gain overview of commands  |
 |  2. Configure device to station or soft-AP     |
 |  3. Setup WiFi connection                      |
 |  4. Run iperf to test UDP/TCP RX/TX throughput |
 |                                                |
 =================================================


Type 'help' to get the list of commands.
Use UP/DOWN arrows to navigate through command history.
Press TAB when typing command name to auto-complete.
iperf> help
help
  Print the list of registered commands

free
  Get the current size of free heap memory

heap
  Get minimum size of free heap memory that was available during program execu
  tion

version
  Get version of chip and SDK

restart
  Software reset of the chip

deep_sleep  [-t <t>] [--io=<n>] [--io_level=<0|1>]
  Enter deep sleep mode. Two wakeup modes are supported: timer and GPIO. If no
  wakeup option is specified, will sleep indefinitely.
  -t, --time=<t>  Wake up time, ms
      --io=<n>  If specified, wakeup using GPIO with given number
  --io_level=<0|1>  GPIO level to trigger wakeup

light_sleep  [-t <t>] [--io=<n>]... [--io_level=<0|1>]...
  Enter light sleep mode. Two wakeup modes are supported: timer and GPIO. Mult
  iple GPIO pins can be specified using pairs of 'io' and 'io_level' arguments
  . Will also wake up on UART input.
  -t, --time=<t>  Wake up time, ms
      --io=<n>  If specified, wakeup using GPIO with given number
  --io_level=<0|1>  GPIO level to trigger wakeup

sta  <ssid> [<pass>]
  WiFi is station mode, join specified soft-AP
        <ssid>  SSID of AP
        <pass>  password of AP

scan  [<ssid>]
  WiFi is station mode, start scan ap
        <ssid>  SSID of AP want to be scanned

ap  <ssid> [<pass>]
  AP mode, configure ssid and password
        <ssid>  SSID of AP
        <pass>  password of AP

query
  query WiFi info

iperf  [-suVa] [-c <ip>] [-p <port>] [-l <length>] [-i <interval>] [-t <time>]
  iperf command
  -c, --client=<ip>  在客户端模式下运行，连接到 <host>
  -s, --server  在服务器模式下运行
     -u, --udp  使用UDP而不是TCP
  -V, --ipv6_domain  使用IPV6地址而不是IPV4地址
  -p, --port=<port>  要侦听/连接的服务器端口
  -l, --len=<length>  设置读/写缓冲区大小
  -i, --interval=<interval>  seconds between periodic bandwidth reports
  -t, --time=<time>  time in seconds to transmit for (default 10 secs)
   -a, --abort  中止运行iperf
```

## 命令

### AP

```
ap esp32-s3 88888888  // 创建ap
iperf -s -i 2 -t 1000   // 服务器端，TCP模式，每2s报告一次带宽，传输1000s

iperf -s -u -i 2 -t 1000   // 服务器端，TCP模式，每2s报告一次带宽，传输1000s


iperf -a
```

### STA

```
sta esp32-s3 88888888             // 连接ap
iperf -c 192.168.4.1 -i 2 -t 30  // 客户端，主机地址192.168.4.1，TCP模式，每2s报告一次带宽，传输60s
iperf -c 192.168.4.1 -u -i 2 -t 30  // 客户端，主机地址192.168.4.1，UDP模式，每2s报告一次带宽，传输60s
```

### esp32

```
sta hi 88888888             // 连接ap
// sta hi123 88888888
iperf -s -i 2 -t 1000

iperf -c 192.168.155.3 -i 2 -t 30
```


### 电脑

```
./iperf -c 192.168.137.119 -i 2 -t 30  // 客户端，主机地址192.168.4.1，TCP模式，每2s报告一次带宽，传输60s
./iperf -c 192.168.155.2 -i 2 -t 30


./iperf -s -i 2 -t 1000
```

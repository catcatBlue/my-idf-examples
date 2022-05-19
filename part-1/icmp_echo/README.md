# ICMP Echo-Reply (Ping) example

(See the README.md file in the upper level 'examples' directory for more information about examples.)

Ping 是一个有用的网络实用程序，用于测试远程主机是否可以在 IP 网络上访问。它测量从源主机发送到目标主机的消息被回显到源的往返时间。

Ping 通过向目标主机发送 Internet 控制消息协议（ICMP）回显请求数据包并等待 ICMP 回显回复来进行操作。

本例基于[console 组件](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/console.html). 实现了一个简单的 ping 命令行 util

**Notes:** 目前这个例子只支持 IPv4。

## How to use example

### Hardware Required

This example should be able to run on any commonly available ESP32 development board.

### Configure the project

```
idf.py menuconfig
```

In the `Example Connection Configuration` menu:

- 在`Connect using`下选择本例使用的网络接口（Wi-Fi 或以太网）。
- 如果选择了 Wi-Fi 接口，您还必须设置：
  - 您的线路板将连接到的 Wi-Fi SSID 和 Wi-Fi 密码。
- 如果选择 Ethernet interface（以太网接口），还必须设置：
  - Select Ethernet type under `Ethernet Type`, for example, `Internal EMAC` or `SPI Ethernet Module`.
  - Select Ethernet PHY chip model under `Ethernet PHY Device`, for example, `IP101`.
  - You might also have to set other Ethernet driver specific parameters under `Component Config > Ethernet`, for example, EMAC Clock mode, GPIO used by SMI, and etc.

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

(Replace PORT with the name of the serial port to use.)

(To exit the serial monitor, type `Ctrl-]`.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.

## Example Output

- 运行`help`获取所有受支持命令的手册。

```
esp32> help
help
  打印已注册命令的列表

ping  [-W <t>] [-i <t>] [-s <n>] [-c <n>] [-Q <n>] <host>
  向网络主机发送ICMP ECHO_REQUEST
  -W, --timeout=<t>  等待响应的时间，以秒为单位
  -i, --interval=<t>  发送每个数据包之间的等待间隔秒
  -s, --size=<n>  指定要发送的数据字节数
  -c, --count=<n>  发送计数数据包后停止
  -Q, --tos=<n>  在IP数据报中设置服务相关位的类型
        <host>  主机地址
```

- 运行`ping`命令测试远程服务器的可访问性。

```
esp32> ping www.espressif.com
64 bytes from 119.9.92.99 icmp_seq=1 ttl=51 time=36 ms
64 bytes from 119.9.92.99 icmp_seq=2 ttl=51 time=34 ms
64 bytes from 119.9.92.99 icmp_seq=3 ttl=51 time=37 ms
64 bytes from 119.9.92.99 icmp_seq=4 ttl=51 time=36 ms
64 bytes from 119.9.92.99 icmp_seq=5 ttl=51 time=33 ms

--- 119.9.92.99 ping statistics ---
5 packets transmitted, 5 received, 0% packet loss, time 176ms
```

- 使用错误的域名运行`ping`

```bash
esp32> ping www.hello-world.io
ping: unknown host www.hello-world.io
Command returned non-zero error code: 0x1 (ERROR)
```

- 在无法访问的服务器上运行`ping`

```bash
esp32> ping www.zoom.us
From 69.171.230.18 icmp_seq=1 timeout
From 69.171.230.18 icmp_seq=2 timeout
From 69.171.230.18 icmp_seq=3 timeout
From 69.171.230.18 icmp_seq=4 timeout
From 69.171.230.18 icmp_seq=5 timeout

--- 69.171.230.18 ping statistics ---
5 packets transmitted, 0 received, 100% packet loss, time 4996ms
```

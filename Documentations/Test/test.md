# TESTING THE FIREWALL

## 1. If you have a Linux virtual machine

#### 1.1 Build the firewall on your virtual machine (or on your device if your device is Linux) following the steps on readme file. The machine that you builded your firewall is called server and the other machine is called client from now on.

#### 1.2 Make sure your virtual machine is connected to your device with "Bridged Adaptor" network. If you are using VirtualBox, you can follow steps **Devices->Network->Network Settings** to change network settings of your virtual machine.

#### 1.3 On server and client, use `ifconfig` command to get the private ip of the server and client. We will use server ip to be able to ping the server and we will use client ip to set the proper firewall rule to drop the packets from the client.

#### 1.4 Create the server with `iperf3` using followind command.
```sh
iperf3 -s
```

#### 1.5 On client, ping to the server with `iperf3` using followind command.
```sh
iperf3 -c {server ip} -t 300 -u
```

#### 1.6 Now you should be able to see the upcoming packets on your server terminal.
```sh
baranbologur-vm@baranbologur-vm:~/Desktop/platform_i$ iperf3 -s
-----------------------------------------------------------
Server listening on 5201
-----------------------------------------------------------
Accepted connection from {client ip}, port {client port 1}
[  5] local {server ip} port {server port} connected to {client ip} port {client port 2}
[ ID] Interval           Transfer     Bitrate         Jitter    Lost/Total Datagrams
[  5]   0.00-1.00   sec   123 KBytes  1.01 Mbits/sec  0.078 ms  0/87 (0%)  
[  5]   1.00-2.00   sec   129 KBytes  1.05 Mbits/sec  0.084 ms  0/91 (0%)
```
The server port is the one that we will use in our test. Note this port as well.

#### 1.7 On your server execute **aird** and **cli** processes and add "{client ip} {server port} udp" to the firewall rules using following commands.
```sh
./aird
./cli -a "{client ip} {server port} udp"
```
Note that we use udp since I used "-u" in iperf command and so the connection type is UDP.

#### 1.8 As a result, packets sent from client should be dropped and we should see the following text in terminal
```sh
[  5]  38.00-39.00  sec   129 KBytes  1.06 Mbits/sec  0.087 ms  0/91 (0%)  
[  5]  39.00-40.00  sec   127 KBytes  1.04 Mbits/sec  0.097 ms  0/90 (0%)  
[  5]  40.00-41.00  sec   129 KBytes  1.06 Mbits/sec  0.048 ms  0/91 (0%)  
[  5]  41.00-42.00  sec   127 KBytes  1.04 Mbits/sec  0.113 ms  0/90 (0%)  
[  5]  42.00-43.00  sec   129 KBytes  1.05 Mbits/sec  0.104 ms  0/91 (0%)  
[  5]  43.00-44.00  sec  29.7 KBytes   243 Kbits/sec  0.151 ms  0/21 (0%)  
[  5]  44.00-45.00  sec  0.00 Bytes  0.00 bits/sec  0.151 ms  0/0 (0%)  #packets are successfully dropped
[  5]  45.00-46.00  sec  0.00 Bytes  0.00 bits/sec  0.151 ms  0/0 (0%)  
[  5]  46.00-47.00  sec  0.00 Bytes  0.00 bits/sec  0.151 ms  0/0 (0%)  
[  5]  47.00-48.00  sec  0.00 Bytes  0.00 bits/sec  0.151 ms  0/0 (0%)  
[  5]  48.00-49.00  sec  0.00 Bytes  0.00 bits/sec  0.151 ms  0/0 (0%)  
[  5]  49.00-50.00  sec  0.00 Bytes  0.00 bits/sec  0.151 ms  0/0 (0%)  
[  5]  50.00-51.00  sec  0.00 Bytes  0.00 bits/sec  0.151 ms  0/0 (0%)  
[  5]  51.00-52.00  sec  0.00 Bytes  0.00 bits/sec  0.151 ms  0/0 (0%)  
[  5]  52.00-53.00  sec  0.00 Bytes  0.00 bits/sec  0.151 ms  0/0 (0%)
```

#### 1.9 Now you can use following commands to see the index of our firewall rule and delete the rule to accept the packets again.
```sh
./cli -s
./cli -d {rule index}
```
After these commands, the packets should go back to the normal.
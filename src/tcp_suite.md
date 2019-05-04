
# Run the TCP example suite against Zephyr app in qemu

# 1 Build and run [net-test-tools](https://github.com/intel/net-test-tools)
```
# ./autogen.sh
# make
# ./loop-slipcat.sh
```

# 2 Configure and Run echo app in qemu

Add the following into ```samples/net/sockets/echo/prj.conf```:
```
CONFIG_NET_CONFIG_NEED_IPV6=n
CONFIG_NET_CONFIG_NEED_IPV4=n

CONFIG_NET_CONFIG_MY_IPV6_ADDR="fe80::1"
CONFIG_NET_CONFIG_PEER_IPV6_ADDR="fe80::2"

CONFIG_NET_CONFIG_MY_IPV4_ADDR="10.0.0.1"
CONFIG_NET_CONFIG_PEER_IPV4_ADDR="10.0.0.2"

CONFIG_NET_SLIP_TAP=y
CONFIG_SLIP_MAC_ADDR="00:00:00:00:00:01"
```

```
# cd samples/net/sockets/echo
# mkdir build && cd build
# cmake -DBOARD=qemu_x86 ..
# make run
```

# 3 Observe the traffic with Wireshark
```
# sudo apt-get install wireshark-gtk
# wireshark-gtk -p -i lo -f "udp port 5555" -d udp.port==5555,eth -k &
```

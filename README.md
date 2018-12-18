
# Net-test-suites

Net-test-suites includes the TCP protocol test suite written in TTCN-3 language.

The test suites can be compiled with the TTCN-3 compiler and executed
with the TTCN-3 runtime (Eclipse titan.core).

## Building and Running the TCP Test Suite

### 1. Install the Dependencies

Ubuntu:

```
    # sudo apt-get install g++ libxml2-dev libssl-dev expect
```

Fedora:

```
    # sudo dnf install gcc-c++ libxml2-devel openssl-devel expect
```

### 2. Download and Install the titan.core TTCN-3 Runtime

Follow one of the following options to install Eclipse titan.core on your
Linux host system:

* To install into the default location ```~/titan```, run the supplied
titan-instal.sh script:

```
    # cd net-test-suites.git
    # . titan-install.sh
```

* Alternatively, visit the project's download page, get a suitable version
and unpack to the desired location.

    https://projects.eclipse.org/projects/tools.titan/downloads

* titan.core can be also compiled and installed from the source
from https://github.com/eclipse/titan.core

    Note: The newest upstream versions require "-e" to be added
    to the COMPILER_FLAGS in the generated makefile for the TCP test suite
    to compile (legacy RAW codec behaivior).

* Ubuntu's default titan.core:

```
    # sudo apt-get install eclipse-titan
```

Note: Ubuntu’s default titan.core is packaged incorrectly and pulls extra
dependencies, this option isn't recommended.

### 3. Set the Project Environment

```
    # . titan-env.sh
```

Or simply add the following to your ```~/.bashrc```:

```
    export TTCN3_DIR=~/titan
    export PATH=${TTCN3_DIR}/bin:${PATH}
```

Note: In case of Ubuntu's default titan.core:

```
    export TTCN3_DIR=/usr
```

### 4. Build the Test Suite

```
    # cd src
    # . make.sh
```

### 5. Run the Test Suite

```
    # ttcn3_start test_suite tcp_suite.cfg
```

The test suite expects Ethernet frames encapsulated over UDP/IPv4.

By default it listens to localhost:7777 and sends to localhost:7771.

TCP suite expects Zephyr's echo_server app configured to listen at
10.0.0.1, port 4242 and communicates from address 10.0.0.2, port 4242.

This can be customized by setting module parameters of libtest.ttcn
through the configuration file (tcp_suite.cfg), for example:

```
    [MODULE_PARAMETERS]
    libtest.m_ip_dst := "10.0.0.1"
    libtest.m_ip_src := "10.0.0.2"
```

### 6. Running the Test Suites Against Zephyr app in QEMU

#### 6.1 Get, build and start the net-test-tools

```
   # git clone https://github.com/intel/net-test-tools.git
   # cd net-test-tools
   # ./autogen.sh
   # make
```

Start net-test-tools:

```
   # ./loop-slipcat.sh
```

#### 6.3 Configure and Run echo-server Application in QEMU

Add the following entries to the prj.conf:

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
   # mkdir build
   # cd build
   # cmake -DBOARD=qemu_x86 ..
   # make
   # make run
```

#### 6.4 Wireshark

Observe the traffic with Wireshark:

```
    # sudo apt-get install wireshark-gtk
    # wireshark-gtk -p -i lo -f "udp port 5555" -d udp.port==5555,eth -k &
```

#### Reporting a Security Issue

If you have information about a security issue or vulnerability,
please follow the process at https://01.org/security


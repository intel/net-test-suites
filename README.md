# PROJECT NOT UNDER ACTIVE MANAGEMENT #  
This project will no longer be maintained by Intel.  
Intel has ceased development and contributions including, but not limited to, maintenance, bug fixes, new releases, or updates, to this project.  
Intel no longer accepts patches to this project.  
 If you have an ongoing need to use this project, are interested in independently developing it, or would like to maintain patches for the open source software community, please create your own fork of this project.  
  

# Net-test-suites

Net-test-suites include TCP test suites written in [TTCN-3](https://www.netdevconf.org/2.2/papers/welte-ttcn3-talk.pdf) for [Eclipse titan.core](https://gitlab.eclipse.org/eclipse/titan/titan.core/-/blob/master/README.md) (open source TTCN-3 compiler and runtime).

Test Suite | Description
--------|--------
[tcp_suite.ttcn](https://github.com/intel/net-test-suites/blob/master/src/tcp_suite.ttcn) | This is a sample TCP suite using a black-box testing approach
[tcp2_check.ttcnpp](https://github.com/intel/net-test-suites/blob/master/src/tcp2_check.ttcnpp) | This is a sanity check suite for the [experimental TCP](https://github.com/ozhuraki/zephyr) that additionally uses a white-box testing with [JSON-based test protocol](https://github.com/intel/net-test-suites/blob/master/src/tcp2_utils.ttcnpp#L73)

# 1 Build and Run

## 1.1 Install Dependencies
Ubuntu: ```# sudo apt install g++ libxml2-dev libssl-dev expect```

Fedora: ```# sudo dnf install gcc-c++ libxml2-devel openssl-devel expect```

## 1.2 Install [titan.core](https://gitlab.eclipse.org/eclipse/titan/titan.core/-/blob/master/README.md)
To install titan.core onto the Linux host:

* Install through the package manager (Ubuntu):
    ```# sudo apt install --no-install-recommends eclipse-titan```
* Get a version from the project's download [page](https://projects.eclipse.org/projects/tools.titan/downloads)
* Install from the [source](https://gitlab.eclipse.org/eclipse/titan/titan.core)

## 1.3 Build
```
# cd src
# ./make.sh
```
## 1.4 Run

Test Suite | Command
--------|--------
[tcp_suite.ttcn](src/tcp_suite.ttcn) | ```# ttcn3_start test_suite tcp_suite.cfg```
[tcp2_check.ttcnpp](src/tcp2_check.ttcnpp) | ```# ttcn3_start test_suite tcp2_check_3_runs.cfg```

To run a single test:
```
# ttcn3_start test_suite tcp2_check_3_runs.cfg tcp2_check.test_tcp_connect_data_close
```

# 2 Communication with the System Under Test (SUT)

Test suites send Ethernet frames over UDP/IPv4 to the [net-test-tools](https://github.com/intel/net-test-tools) that support:

SUT interface | Default | Encapsulation
--------|--------|--------
AF_UNIX | /tmp/slip.sock | SLIP
PTY | - | SLIP
TAP | - | -

## 2.1 Endpoints

IPv4/UDP Endpoint | Purpose
--------|--------
localhost:7771 | [net-test-tools](https://github.com/intel/net-test-tools) interfaced to the SUT
localhost:7777 | Test suite

Ethernet MAC | TCP Endpoint | Purpose | Manual
--------|--------|--------|--------
00:00:00:00:00:01 | 192.0.2.1:4242 | SUT | [Zephyr echo app manual](https://github.com/intel/net-test-suites/blob/master/src/tcp_suite.md), [TCP2 manual](https://github.com/ozhuraki/zephyr)
00:00:00:00:00:02 | 192.0.2.2:4242 | Test suite | [tcp_suite.ttcn](src/tcp_suite.ttcn), [tcp2_check.ttcnpp](src/tcp2_check.ttcnpp)

All endpoints are [configurable](src/tcp_suite.cfg#L6).

### Reporting a Security Issue
If you have an information about a security issue or vulnerability,
please follow the [process](https://01.org/security).

# References

- [https://projects.eclipse.org/projects/tools.titan](https://projects.eclipse.org/projects/tools.titan)
- [TTCN-3 and Eclipse TITAN for testing protocol stacks](https://legacy.netdevconf.info/2.2/papers/welte-ttcn3-talk.pdf)
- [https://gitlab.eclipse.org/eclipse/titan/titan.core/-/blob/master/README.md](https://gitlab.eclipse.org/eclipse/titan/titan.core/-/blob/master/README.md)
- [Programmers' Technical Reference Guide for the TITAN TTCN-3 Toolset](https://mirror.umd.edu/eclipse/titan/ReferenceGuide.pdf)
- [User Guide for TITAN TTCN-3 Test Executor](https://mirror.umd.edu/eclipse/titan/UserGuide.pdf)


# Net-test-suites

Net-test-suites include a few TCP test suites written in [TTCN-3](https://www.netdevconf.org/2.2/papers/welte-ttcn3-talk.pdf) for [Eclipse titan.core](https://projects.eclipse.org/projects/tools.titan) (open source TTCN-3 compiler and runtime).

Test suite | Description
--------|--------
[tcp_suite.ttcn](https://github.com/intel/net-test-suites/blob/master/src/tcp_suite.ttcn) | Example TCP suite
[tcp2_check.ttcnpp](https://github.com/intel/net-test-suites/blob/master/src/tcp2_check.ttcnpp) | Sanity check suite for [experimental TCP](https://github.com/ozhuraki/zephyr) (TCP2)

# 1 Build and Run

## 1.1 Install Dependencies
Ubuntu: ```# sudo apt-get install g++ libxml2-dev libssl-dev expect```

Fedora: ```# sudo dnf install gcc-c++ libxml2-devel openssl-devel expect```
## 1.2 Install titan.core
Options to install titan.core onto your Linux host:
* Install with the supplied script (```~/titan```): ```# . titan-install.sh```
* Get a suitable version from [project's download](https://projects.eclipse.org/projects/tools.titan/downloads) page.
* Install from the [source](https://github.com/eclipse/titan.core).
* Ubuntu's default titan.core (packaged incorrectly, not recommended): ```# sudo apt-get install eclipse-titan```

## 1.3 Set the Environment
```
# . titan-env.sh
```
For Ubuntu's packaged titan.core, ```TTCN3_DIR=/usr```.
## 1.4 Build
```
# cd src
# . make.sh
```
## 1.5 Run

Test suite | Command
--------|--------
[tcp_suite.ttcn](https://github.com/intel/net-test-suites/blob/master/src/tcp_suite.ttcn) | ```# ttcn3_start test_suite tcp_suite.cfg```
[tcp2_check.ttcnpp](https://github.com/intel/net-test-suites/blob/master/src/tcp2_check.ttcnpp) | ```# ttcn3_start test_suite tcp2_check_3_runs.cfg```

# 2 Description

Test suites send Ethernet frames over UDP/IPv4 to [net-test-tools](https://github.com/intel/net-test-tools) that support:

SUT interface | Default | Encapsulation
--------|--------|--------
AF_UNIX | /tmp/slip.sock | SLIP
PTY | - | SLIP
TAP | - | -

## 2.1 Endpoints

IPv4/UDP Endpoint | Purpose
--------|--------
localhost:7771 | [net-test-tools](https://github.com/intel/net-test-tools) interfaced to system under test
localhost:7777 | Test suite

Ethernet MAC | TCP Endpoint | Purpose | Manual
--------|--------|--------|--------
00:00:00:00:00:01 | 192.0.2.1:4242 | System under test | [echo app manual](https://github.com/intel/net-test-suites/blob/master/src/tcp_suite.md)/[TCP2 manual](https://github.com/ozhuraki/zephyr)
00:00:00:00:00:02 | 192.0.2.2:4242 | Test suite | [tcp_suite.ttcn](https://github.com/intel/net-test-suites/blob/master/src/tcp_suite.ttcn)/[tcp2_check.ttcnpp](https://github.com/intel/net-test-suites/blob/master/src/tcp2_check.ttcnpp)

All endpoints are [configurable](https://github.com/intel/net-test-suites/blob/master/src/tcp_suite.cfg#L6).

### Reporting a Security Issue
If you have information about a security issue or vulnerability,
please follow the [process](https://01.org/security).


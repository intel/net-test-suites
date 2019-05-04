
# Net-test-suites

Net-test-suites includes a few TCP protocol test suites written in [TTCN-3](https://www.netdevconf.org/2.2/papers/welte-ttcn3-talk.pdf) for [Eclipse titan.core](https://projects.eclipse.org/projects/tools.titan) (open source TTCN-3 compiler and runtime).

**Contents**

Test suite | Description
--------|--------
[tcp_suite.ttcn](https://github.com/intel/net-test-suites/blob/master/src/tcp_suite.ttcn) | Example TCP suite
[tcp2_check.ttcnpp](https://github.com/intel/net-test-suites/blob/master/src/tcp2_check.ttcnpp) | Sanity check suite for [experimental TCP (TCP2)](https://github.com/ozhuraki/zephyr)

# 1 Build and Run

## 1.1 Install Dependencies

Ubuntu:

```
# sudo apt-get install g++ libxml2-dev libssl-dev expect
```
Fedora:

```
# sudo dnf install gcc-c++ libxml2-devel openssl-devel expect
```
## 1.2 Install titan.core

Follow one of the options to install titan.core on your Linux host:

* Install with the supplied script (```~/titan```):

```
# . titan-install.sh
```
* Get a suitable version from [project's download](https://projects.eclipse.org/projects/tools.titan/downloads) page.
* Install from the [source](https://github.com/eclipse/titan.core).
* Ubuntu's default titan.core (packaged incorrectly, not recommended):
```
# sudo apt-get install eclipse-titan
```
## 1.3 Set the Environment

```
# . titan-env.sh
```
Or add into ```~/.bashrc```:

```
# export TTCN3_DIR=~/titan
# export PATH=${TTCN3_DIR}/bin:${PATH}
```
Note: In case of Ubuntu's packaged titan.core, ```TTCN3_DIR=/usr```.

## 1.4 Build
```
# cd src
# . make.sh
```
## 1.5 Run
```
# ttcn3_start test_suite tcp_suite.cfg
```
or
```
# ttcn3_start test_suite tcp2_check_3_runs.cfg
```

# 2 Description

The test suites expect Ethernet frames encapsulated over IPv4/UDP.

IPv4/UDP Endpoint | Purpose
--------|--------
localhost:7777 | Test suite
localhost:7771 | System under test (SUT)

Ethernet Endpoint | Purpose
--------|--------
00:00:00:00:00:02 | Test suite
00:00:00:00:00:01 | System under test (SUT)

Endpoints are configureable.

## 2.1 TCP Suite

TCP Endpoint | Purpose
--------|--------
10.0.0.2:4242 | Example TCP suite, [tcp_suite.ttcn](https://github.com/intel/net-test-suites/blob/master/src/tcp_suite.ttcn)
10.0.0.1:4242 | System under test (SUT), [Zephyr OS](https://www.zephyrproject.org) samples/net/sockets/echo app

Endpoints are [configureable](https://github.com/intel/net-test-suites/blob/master/src/tcp_suite.cfg#L6).

Follow the [guide](https://github.com/intel/net-test-suites/blob/master/src/tcp_suite.md).

## 2.2 TCP2 Sanity Check

TCP Endpoint | Purpose
--------|--------
192.0.2.2:4242 | TCP2 Sanity Check, [tcp2_check.ttcnpp](https://github.com/intel/net-test-suites/blob/master/src/tcp2_check.ttcnpp)
192.0.2.1:4242 | System under test (SUT), [experimental TCP (TCP2)](https://github.com/ozhuraki/zephyr)

Follow the [guide](https://github.com/ozhuraki/zephyr).

### Reporting a Security Issue

If you have information about a security issue or vulnerability,
please follow the [process](https://01.org/security).


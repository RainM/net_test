# net_test
Simple test for testing end-to-end latency for established TCP connection.
As a result, application outputs HDR histograms of either write latency and network connectivity latency.

## Hot it works

1. It establishes TCP connection over specified port and host (conterparty server is here as well)
2. Sends 16-byte message
3. Counterparty reads it and sends back
4. We receives data and measure two timings:
4.1. How much time did we write to socket on our side (time for 'write' syscall)
4.2. How much time did we wait before data returned (time from 'write' syscall to end of 'read' syscall)

Therefore, we get two numbers. The first one is how much time did we send data to TCP stack. The second number is true TCP connection latency.

## How to build

```
$ make download-modules
$ make all
```

## How to use

In one shell:
```
$ HOST=CONNECTION_HOST make start-mirror
```
Here we copy 'mirror' executable to remove server and start it (on 12346 port)

Next, in another shell:

```
$ ./net_test CONNECTION_HOST 12346
```
Here we start 'net_test' itself. It will will connect to mirror server over TCP and make measurements.

As a result, you will get HDR histograms for write and transmition latency.

## Tested platforms

- CentOS 7.3
- Arch Linux

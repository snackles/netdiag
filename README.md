# NetDiag
 
Lightweight Linux network diagnostic utility written in C.
 
`NetDiag` provides quick access to common network information such as:
 
- network interfaces
- routing table
- VLAN interfaces
- link status
- gateway connectivity
- diagnostic collection
 
The project uses Linux sysfs and standard networking utilities (`ip`, `ping`, `ss`, `iptables`) to gather information.
 
---
 
# Features
 
- Display network interfaces and IP addresses
- Show routing table
- Detect VLAN interfaces
- Check interface link state
- Test gateway reachability
- Collect network diagnostics into compressed archive
- Simple command-line interface
- Minimal dependencies
 
---
 
# Requirements
 
Linux system with:
 
- GCC or Clang
- GNU Make
- `iproute2`
- `ping`
- `ss`
- `tar`
 
---
 
# Build
 
Clone the repository:
 
```bash
git clone https://github.com/snackles/netdiag.git
cd netdiag
```
 
Build the project:
 
```bash
make
```
 
---
 
# Usage
 
```bash
./netdiag <command>
```
 
---
 
# Commands
 
## Show network interfaces
 
```bash
./netdiag show interfaces
```
 
Displays:
 
- interface name
- operational state
- MAC address
- MTU
- IPv4/IPv6 addresses
 
### Example Output
 
```text
IFACE         STATE         MAC                MTU    IP ADDRESSES
---------------------------------------------------------------------------
lo            unknown       00:00:00:00:00:00 65536  127.0.0.1/8 ::1/128
eth0          up            52:54:00:12:34:56 1500   192.168.1.100/24
wlan0         down          a4:bb:6d:11:22:33 1500   -
docker0       down          02:42:2f:aa:bb:cc 1500   172.17.0.1/16
```
 
---
 
## Show routing table
 
```bash
./netdiag show routes
```
 
Displays IPv4 routing table.
 
---
 
## Show VLAN interfaces
 
```bash
./netdiag show vlans
```

 Displays detected VLAN interfaces from sysfs.
 
---
 
## Check interface link status
 
```bash
./netdiag check link <interface>
```

---
 
## Check gateway connectivity
 
```bash
./netdiag check gateway <ip>
```
 
---
 
## Collect diagnostics
 
```bash
./netdiag collect
```
 
Collects:
 
- IP configuration
- routing table
- ARP table
- listening ports
- kernel messages
- firewall rules
- network statistics
 
Creates archive:
 
```text
netdiag_archive.tar.gz
```
 
---
 
# Project Structure
 
```text
.
├── include
│   ├── commands.h
│   ├── constants.h
│   └── utils.h
│
├── src
│   ├── commands.c
│   ├── main.c
│   └── utils.c
│
├── Makefile
└── README.md
```
 
---
 
# Logging
 
The project includes a simple logging system with levels:
 
- DEBUG
- INFO
- WARNING
- ERROR
 
Enable debug logging:
 
```bash
make debug
```
 
---
 
# Notes

- The utility is Linux-specific because it relies on `/sys/class/net`.
- VLAN detection depends on sysfs availability.
 
---
 
# License
 
MIT License

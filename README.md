This project is carried out during an internship at company Airties by Taha Eren Sahin, Ahmet Koca, and Baran Bologur.

# Introduction

The project integrates a daemon, CLI, and kernel module into a single embedded Linux system to enhance network security through custom firewall rules and intrusion detection. The functionalities are as follows:

    Kernel Module: This component hooks into the networking stack to intercept and filter incoming and outgoing packets based on custom firewall rules.

    Daemon: The daemon communicates with the kernel module using ioctl calls to configure firewall rules and log events. It allows administrators to enable or disable rules, update rule sets, and log security events efficiently.

    CLI App: The command-line interface (CLI) application connects to the daemon via Linux sockets to configure firewall rules.


# Building the project

## Requirements
	
	gcc - GNU project C and C++ compiler
	make - GNU make utility to maintain groups of programs

## Clone the repository
	git clone https://github.com/terensahin/platform_i.git

## Build 
```sh
make
cd build/kbuild && sudo insmod kmodule.ko && cd ..
sudo ./aird
```



# Test cases
---

## 1. Test the add firewall rule action

Commands:
```sh
cli -a "192.168.100.11 5005 tcp"
```

Expected Output:
* Daemon informs that the rule has been successfully added.


Events:

	A new firewall rule is sent to daemon
	A new firewall rule is added to the kernel module.
	The kernel module successfully blocks TCP packets from the given IP on the given port.


---

## 2. Test the show firewall rules action

Commands:
```sh
cli -s
```

Expected Output:

* All firewall rules are printed with indexes.

---

## 3. Test the delete firewall rule action

Commands:
```sh
cli -d [index]
```

Expected Output:

* Daemon informs that rule with the given index is successfully deleted. 

Events:

	Firewall rule is removed from kernel module
	Connections are accepted from the deleted IP on the given port.

---

## 4. Test the terminate action

Commands:
```sh
cli -t
```

Expected Output:

* Daemon informs the termination.

Events:

	Daemon is terminated

---

## 5. Test the change log level action

Commands:
```sh
	cli -l [0-5]
```

Expected Output:

* Daemon informs the log level change.

Events:

	Daemon's log level is changed.



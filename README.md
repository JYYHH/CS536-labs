## Usage
### Compile the demo
```bash
make clean
make
```
### Compile the TCP part
```bash
make clean
make TCP
```
### Compile the UDP part
```bash
make clean
make UDP
``` 
### Compile the Multi-thread TCP part
```bash
make clean
make Mul
```
### Compile the Web Server
```bash
make clean
make Web
```
### Run the bin
- ___TOTALLY THE SAME AS WHAT IN___ [this pdf](./lab1-cs536.pdf)
    - the first parameter of client can be a `url` as well, instead of an `ip` address.

- in PartA and PartB, you should first launch server, and then client
    - In order to exit safely from the waiting loop of a TCP server (serverTCP/serverMul/serverWeb), you need to send a `SIGINT` signal to the process manually (by typing `Ctrl+C` in the terminal or using other shell command).
        - Remember only send `SIGINT` to the server when it's in idle.
    - But the server will flush the output to the log file immediately after it receives the TCP end from the client.
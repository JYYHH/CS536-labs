## 0. Header
- I upload my code [here](https://github.com/JYYHH/CS536-labs/tree/lab1)
## 1. Usage
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
- in PartA and PartB, you should first launch server, and then client

## 2. Answer for before questions

- `to be done`

## 3. Difficulties
1. I first write each source file one by one seperately, but finally realize that I can write them in a neater way (putting the commonly used function in `src/common.h`), it takes me a lot of time to reconstruct the repo.
2. At first I met some `segmentation fault`, finally I figure it out that this was because my char array was too small to save the file name (designed by me).
target = server client
targetTCP = serverTCP clientTCP
targetUDP = serverUDP clientUDP
targetMul = serverMul clientTCP
targetWeb = serverWeb 

HEADER = src/common.h
FLAGS = -pthread -o

normal: $(target) $(HEADER)
TCP: $(targetTCP) $(HEADER)
UDP: $(targetUDP) $(HEADER)
Mul: $(targetMul) $(HEADER)
Web: $(targetWeb) $(HEADER)

server: src/server.c
	gcc $(FLAGS) $@ $^
client: src/client.c
	gcc $(FLAGS) $@ $^
serverTCP: src/serverTCP.c
	gcc $(FLAGS) $@ $^
clientTCP: src/clientTCP.c
	gcc $(FLAGS) $@ $^
serverUDP: src/serverUDP.c
	gcc $(FLAGS) $@ $^
clientUDP: src/clientUDP.c
	gcc $(FLAGS) $@ $^
serverMul: src/serverMul.c
	gcc $(FLAGS) $@ $^
serverWeb: src/serverWeb.c 
	gcc $(FLAGS) $@ $^

clean:
	rm -rf $(target) $(targetTCP) $(targetUDP) $(targetMul) $(targetWeb)  *.log *.mp4 *.jpeg *.html
target = server client
targetTCP = serverTCP clientTCP
targetUDP = serverUDP clientUDP
targetMul = serverMul clientTCP
targetWeb = serverWeb 

HEADER = src/common.h

normal: $(target) $(HEADER)
TCP: $(targetTCP) $(HEADER)
UDP: $(targetUDP) $(HEADER)
Mul: $(targetMul) $(HEADER)
Web: $(targetWeb) $(HEADER)

server: src/server.c
	gcc -o $@ $^
client: src/client.c
	gcc -o $@ $^
serverTCP: src/serverTCP.c
	gcc -o $@ $^
clientTCP: src/clientTCP.c
	gcc -o $@ $^
serverUDP: src/serverUDP.c
	gcc -o $@ $^
clientUDP: src/clientUDP.c
	gcc -o $@ $^
serverMul: src/serverMul.c
	gcc -pthread -o $@ $^
serverWeb: src/serverWeb.c 
	gcc -o $@ $^

clean:
	rm -rf $(target) $(targetTCP) $(targetUDP) $(targetMul) $(targetWeb)  *.log *.mp4 *.jpeg *.html
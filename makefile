all:serveur client clientUDP serveurUDP cleanO

serveurUDP: connexionthread.o serveurUDP.o connexionthread.h
	gcc -lpthread -o serveurUDP connexionthread.o serveurUDP.o

connexionthread.o: connexionthread.c connexionthread.h
	gcc -Wall -c connexionthread.c -o connexionthread.o

serveurUDP.o: serveurUDP.c connexionthread.h
	gcc -Wall -c serveurUDP.c -o serveurUDP.o

#clientUDP: clientUDP.c
#	gcc -Wall -o clientUDP clientUDP.c

clientUDP: fonctionClient.o clientUDP.o fonctionClient.h
	gcc -Wall -o clientUDP fonctionClient.o clientUDP.o

fonctionClient.o: fonctionClient.c fonctionClient.h
	gcc -Wall -c fonctionClient.c -o fonctionClient.o

clientUDP.o: clientUDP.c fonctionClient.h
	gcc -Wall -c clientUDP.c -o clientUDP.o

cleanO:
	rm *.o

clean:
	rm *.o serveur client clientUDP multithread serveurUDP reception.*

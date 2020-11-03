ALL: bin/server bin/client

bin/server: obj/chatServer.o obj/pserver.o
	g++ -pthread -o bin/server obj/chatServer.o obj/pserver.o

bin/client: obj/chatClient.o obj/pclient.o
	g++ -pthread -o bin/client obj/chatClient.o obj/pclient.o

obj/chatServer.o:
	g++ -I include/ -Wall -c src/chatServer.cpp -o obj/chatServer.o

obj/pserver.o:
	g++ -I include/ -Wall -c src/pserver.cpp -o obj/pserver.o

obj/chatClient.o:
	g++ -I include/ -Wall -c src/chatClient.cpp -o obj/chatClient.o

obj/pclient.o:
	g++ -I include/ -Wall -c src/pclient.cpp -o obj/pclient.o

clean:
	rm -rf obj/*.o
	rm -rf bin/*
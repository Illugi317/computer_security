CLIENT = client
SERVER = server
CC = g++
CCFLAGS = -std=c++11 -Wall
all: $(CLIENT) $(SERVER)

$(CLIENT): $(CLIENT).cpp
	$(CC) $(CCFLAGS) $(CLIENT).cpp -o $(CLIENT)

$(SERVER): $(SERVER).cpp
	$(CC) $(CCFLAGS) $(SERVER).cpp -o $(SERVER)

clean:
	rm $(CLIENT) $(SERVER)

# citation: https://www.cs.swarthmore.edu/~newhall/unixhelp/howto_makefiles.html
# this website above helped me create this makefile
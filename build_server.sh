clang++ -g -std=c++17 -lpthread -lboost_program_options \
	src/server/utils.cpp \
	src/server/session.cpp \
	src/server/server.cpp \
	src/server/main.cpp \
	-lstdc++fs \
	-o ./file-server

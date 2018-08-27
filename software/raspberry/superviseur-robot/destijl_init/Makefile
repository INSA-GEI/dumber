
CC=g++
CFLAGS=-D_GNU_SOURCE -D_REENTRANT -fasynchronous-unwind-tables -D__MERCURY__ -I/usr/xenomai/include/alchemy -c -g -D_WITH_TRACE_ -I/usr/xenomai/include/ -I/usr/xenomai/include/mercury -MMD -MP
LDFLAGS=-D_GNU_SOURCE -D_REENTRANT -fasynchronous-unwind-tables -D__MERCURY__ -I/usr/xenomai/include/alchemy -L/usr/xenomai/lib -lalchemy -lcopperplate -lmercury -L/opt/vc/lib -I/usr/local/include -lopencv_highgui -lopencv_core -lopencv_imgproc -Wl,--no-as-needed -lalchemy -lcopperplate /usr/xenomai/lib/xenomai/bootstrap.o -Wl,--wrap=main -Wl,--dynamic-list=/usr/xenomai/lib/dynlist.ld -L/usr/xenomai/lib -lmercury -lpthread -lrt -Wl,-rpath /usr/xenomai/lib -lopencv_highgui -lopencv_core -lopencv_imgcodecs -lraspicam_cv -lopencv_imgproc -lpthread
EXEC=superviseur
SRC= $(wildcard *.cpp ./src/*.cpp ./../src/*.cpp)
OBJ= $(SRC:.cpp=.o)


all: $(EXEC)


$(EXEC): $(OBJ)
	@$(CC) -o $@ $^ $(LDFLAGS)


main.o: main.cpp


%.o: %.cpp
	@$(CC) -o $@ -c $< $(CFLAGS)


.PHONY: clean mrproper


clean:
	@rm -rf *.o
	@rm -rf *.d
	@rm -rf ./src/*.o
	@rm -rf ./src/*.d
	@rm -rf ./../src/*o
	@rm -rf ./../src/*.d

mrproper: clean
	@rm -rf $(EXEC)
LDFLAGS= -L/opt/vc/lib  -I/usr/local/include -lopencv_highgui -lopencv_core   -lopencv_imgcodecs -lraspicam_cv -lopencv_imgproc -lpthread

all :
	make serial
	make video
	make ui
	make rt

serial :
	g++ serialExample.cpp -o serialExample.elf  ../obj/serial.o
video :
	g++ videoExample.cpp -o videoExample.elf ../obj/serial.o ../obj/imagerie.o ../obj/tcpServer.o $(LDFLAGS)

ui :
	g++ uiExample.cpp -o uiExample.elf ../obj/serial.o ../obj/imagerie.o ../obj/tcpServer.o $(LDFLAGS)

rt :
	g++ -o rtvideoExample.elf rtvideoExample.cpp -I/usr/xenomai/include/mercury -I/usr/xenomai/include -D_GNU_SOURCE -D_REENTRANT -D__MERCURY__ -I/usr/xenomai/include/alchemy ../obj/serial.o ../obj/imagerie.o ../obj/tcpServer.o -lalchemy  -lcopperplate /usr/xenomai/lib/xenomai/bootstrap.o -Wl,--wrap=main -Wl,--dynamic-list=/usr/xenomai/lib/dynlist.ld -L/usr/xenomai/lib -lmercury -lpthread -lrt    -Wl,-R/usr/xenomai/lib  -L/opt/vc/lib  -I/usr/local/include -lopencv_highgui -lopencv_core -lopencv_imgcodecs -lraspicam_cv -lopencv_imgproc

clean : 
	rm *.elf

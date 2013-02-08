
# just in case
export PKG_CONFIG_PATH=/usr/lib/pkgconfig

#OPT_FLAGS = -O3 -ffast-math -fomit-frame-pointer -funroll-loops
CC = gcc
CFLAGS = `pkg-config --cflags gtk+-2.0` -Wall
#CFLAGS += -DDEBUG_LEVEL=1
LIB_FLAGS = `pkg-config --libs gtk+-2.0`
SRCS = Click-o-Square.c MainWindow.c Playground.c Prefs.c
OBJS = $(SRCS:.c=.o)
TARGET = clickosquare
INSTALL = /usr/bin/install -c
INSTALL_PATH = /usr/local/bin

$(TARGET): $(OBJS)
	$(CC) $(LIB_FLAGS) -o $@ $^

install: $(TARGET)
	$(INSTALL) $(TARGET) $(INSTALL_PATH)

depend: .depend
.depend:
	$(CC) -MM $(CFLAGS) $(SRCS) > .depend

%.o: %.c
	$(CC) $(CFLAGS) $(OPT_FLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS) .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif

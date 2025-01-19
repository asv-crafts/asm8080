
OBJS=Binarizer.o FileBinarizer.o Preprocessor.o Source.o \
     StringBinarizer.o DebugInfo.o common.o i8080asm.o main.o

PREFIX=/usr/local/bin
TARGET_BINARY=i8080asm

CPPFLAGS=-Wall

$(TARGET_BINARY): $(OBJS)
	g++ $(OBJS) -o $@

all: $(TARGET_BINARY)

install:
	[ -d $(PREFIX) ] || mkdir -p $(PREFIX)
	cp $(TARGET_BINARY) $(PREFIX)
	chmod 755 $(PREFIX)/$(TARGET_BINARY)
	chown root:0 $(PREFIX)/$(TARGET_BINARY)

clean:
	@rm *.o $(TARGET_BINARY)

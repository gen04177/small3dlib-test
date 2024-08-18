ifdef PS5_PAYLOAD_SDK
include $(PS5_PAYLOAD_SDK)/toolchain/prospero.mk
else
    $(error PS5_PAYLOAD_SDK is undefined)
endif

CFLAGS := -O3

CFLAGS += $(shell $(PS5_PAYLOAD_SDK)/target/bin/sdl2-config --cflags)
LDADD  += $(shell $(PS5_PAYLOAD_SDK)/target/bin/sdl2-config --libs)

SRCS := alligator.c
OBJS := $(SRCS:.c=.o)
ELF := small3dlib-alligator.elf

all: $(ELF)

$(ELF): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDADD)

clean:
	rm -f $(OBJS) $(ELF)
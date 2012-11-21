# Makefile for the UNIX library.
# Please see LICENSE for licensing information.
# 


# ---------	CURRENTLY SUPPORTED PLATFORMS ---------- 

PLATFORMS="LikeOS"#`find ./os/ -type d | sed "s/.*\///" | sort`

# --------- FLAGS AND VARIABLES --------------------

CFLAGS=-O2 -nostdlib -nodefaultlibs -nostdinc -fno-builtin -static -Wall -static -fno-rtti -ffreestanding -fno-exceptions -Wall -I../include -Ic:/LikeNew/Like/src/src/include/ -Ic:/LikeNew/Like/src/src/include/c++/ -Ic:/LikeNew/Like/src/src/lib/unix/include/unix/ -Wl,-Lc:/LikeNew/Like/src/src/lib/  -Ic:/LikeNew/Like/src/src/lib/unix/include/unix/ -Ic:/LikeNew/Like/src/src/apps -Ic:/LikeNew/Like/src/src/.
CCFLAGS=-O2 -nostdlib -nodefaultlibs -nostdinc -fno-builtin -static -Wall -static -fno-rtti -ffreestanding -fno-exceptions -Wall -I../include -Ic:/LikeNew/Like/src/src/include/ -Ic:/LikeNew/Like/src/src/include/c++/ -Ic:/LikeNew/Like/src/src/lib/unix/include/unix/ -Wl,-Lc:/LikeNew/Like/src/src/lib/  -Ic:/LikeNew/Like/src/src/lib/unix/include/unix/ -Ic:/LikeNew/Like/src/src/apps -Ic:/LikeNew/Like/src/src/.
HEADERPATH=-I./include/unix/



# ---------  GENERIC MAKE RULES --------------------

all:
	@echo "Makefile for the UNIX library."
	@echo "Please see LICENSE for licensing information."
	@echo 
	@echo "Include files: ./include/unix/ "
	@echo "Output should be: libunix.a "
	@echo 
	@echo "Usage: make [ all | clean | <platform> ] "
	@echo 
	@echo "Currently supported platforms:"
	@echo
	@echo $(PLATFORMS)
	@echo
	@echo


clean:
	rm -f ./*.o
	rm -f ./*.a


# -------------- OS DEPENDENT BUILDS ---------------------------

spoon:
	gcc -D__x86__$(HEADERPATH) -I/spoon/system/source/include/ -I./os/$@/ $(CFLAGS) *.c support/*.c sys/*.c ./os/$@/*.c -c
	g++ -D__x86__ $(HEADERPATH) -I/spoon/system/source/include/ -I./os/$@/ $(CCFLAGS)./os/$@/*.cpp -c
	ar -rc libunix.a  *.o

%: 
	#@if [ ! -d ./os/$@ ]; then echo "There is no such platform. Supported platforms are: $(PLATFORMS)"; exit 255; fi;
	gcc $(HEADERPATH) -I./os/$@/ $(CFLAGS) *.c support/*.c sys/*.c ./os/$@/*.c -c
	#gcc -D__x86__ $(HEADERPATH) -I./os/$@/ $(CFLAGS) *.c sys/*.c ./os/$@/*.c -c
	ar -rc libunix.a  *.o
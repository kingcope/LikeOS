cd F:\UBUNTUBACKUP-OPT\LikeNew\Like\src\src\boot

del like.iso

copy ..\build\kernel.bin kernel.bin

mkisofs -r -eltorito-boot stage2_eltorito -c boot/boot.img -no-emul-boot -boot-load-size 4 -boot-info-table -o like.iso .
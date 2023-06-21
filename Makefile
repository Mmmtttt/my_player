tutorial03: tutorial03.c
	gcc -o tutorial03 -g3 tutorial03.c -I${J:/transform/ffmpeg-4.4/include} -I${J:/transform/SDL2-2.0.16/include}  \
	-L${J:/transform/ffmpeg-4.4/lib} -lavutil -lavformat -lavcodec -lswscale -lswresample -lz -lm 

clean:
	rm -rf tutorial03

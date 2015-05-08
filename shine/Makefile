FLASCC:=/Users/xuexinli/flashcc/sdk
FLEX:=/Users/xuexinli/ashole/saybot/sdk/air
AS3COMPILER:=asc2.jar
BASE_CFLAGS:=-Werror -Wno-write-strings -Wno-trigraphs


default:
	"$(FLASCC)/usr/bin/gcc" -O0 $(BASE_CFLAGS)  *.c -emit-swc=im.norm.data.encoders.MP3Encoder.shine -o encoder-mp3.swc


clean:
	rm -f *.swf *.swc *.o *.exe
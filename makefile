index:
  gcc pipeline.c -c
  gcc main.c pipeline.o -o main
clear:
  rm *.o
	rm main

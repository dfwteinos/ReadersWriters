comp:
	@echo "Compile the main program...";
	gcc main.c Semaphores.c SharedMemory.c -o main -lm

clean:
	 rm -f main main.o
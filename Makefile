all:
	gcc parser.c main.c -o moviesearch

clean:
	rm ./moviesearch
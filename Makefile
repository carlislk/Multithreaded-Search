S = -std=c99 -ggdb

search: search.o que.o que.h
	gcc $S search.o que.o -lpthread -o search

que.o: que.c que.h
	gcc -c $S que.c

search.o: search.c que.h
	gcc -c $S search.c

test: search
	./search the /usr/share/dict/american-english /usr/share/dict/american-english /usr/share/dict/american-english
	grep -c the /usr/share/dict/american-english

alttest: search
	./search the smlDict smlDict smlDict
	grep -c the smlDict

clean:
	/bin/rm -rf search.o que.o search

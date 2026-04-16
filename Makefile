main: main.c cJSON/cJSON.c

	gcc -I. main.c cJSON/cJSON.c -o main -lcurl 
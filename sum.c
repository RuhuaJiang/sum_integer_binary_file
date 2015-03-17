#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <inttypes.h>

#define INT_SIZE  4
#define BLOCK_SIZE  (10*1024*1024)  //10M bytes
#define NUMTHREADS  8  //Adjust it according to CPU

typedef struct ThreadData {
	uint64_t start, end, sum;
	FILE *file;
} ThreadData;

uint32_t bytes2int(uint8_t *bytes) {
	uint32_t val;
	val = (uint32_t) bytes[0] + (((uint32_t) bytes[1]) << 8)
			+ (((uint32_t) bytes[2]) << 16) + (((uint32_t) bytes[3]) << 24);
	return val;
}

void* get_partial_sum(ThreadData *data) {
	uint64_t sum = 0;
	uint8_t* buffer = NULL;
	buffer = (uint8_t*) malloc(BLOCK_SIZE);

	int blocks = (data->end - data->start) / BLOCK_SIZE + 1;
	int remain = (data->end - data->start) % BLOCK_SIZE;

	for (int i = 0; i < blocks - 1; ++i) {
		fread(buffer, 1, BLOCK_SIZE, data->file);

		for (int j = 0; j < BLOCK_SIZE; j = j + 4) {
			sum += bytes2int(&buffer[j]);
		}
	}

	fread(buffer, 1, remain, data->file);
	for (int j = 0; j < remain; j = j + 4) {
		sum += bytes2int(&buffer[j]);
	}

	data->sum = sum;
	free(buffer);
	return NULL;
}

int main(int argc, const char * argv[]) {
	FILE *pFileIn = NULL;
	uint64_t sum = 0, file_size;

	if (argc < 2) {
		printf("Please enter a file path\n");
		return -1;
	}

	pFileIn = fopen(argv[1], "rb");

	if (pFileIn == 0) {
		fprintf(stderr, "File error");
		exit(1);
	}

	fseek(pFileIn, 0L, SEEK_END);
	file_size = ftell(pFileIn);
	fseek(pFileIn, 0L, SEEK_SET);
	if (file_size < 10 * BLOCK_SIZE) {
		ThreadData data;
		data.start = 0;
		data.end = file_size;
		data.file = pFileIn;
		data.sum = 0;
		get_partial_sum(&data);
		fprintf(stdout, "%" PRId64 "\n", data.sum);

	} else {
		//Create multiple threads
		//Divide data
		ThreadData data[NUMTHREADS];
		pthread_t thread[NUMTHREADS];
		int taskPerThread = file_size / NUMTHREADS;
		for (int i = 0; i < NUMTHREADS; ++i) {
			data[i].start = i * taskPerThread;
			data[i].end = (i + 1) * taskPerThread;
			data[i].sum = 0;
			data[i].file = pFileIn;
		}
		data[NUMTHREADS - 1].end = file_size;

		//Creates threads
		for (int i = 0; i < NUMTHREADS; ++i) {
			pthread_create(&thread[i], NULL, get_partial_sum, &data[i]);
		}

		//Wait results
		for (int i = 0; i < NUMTHREADS; i++) {
			pthread_join(thread[i], NULL);
		}

		for (int i = 0; i < NUMTHREADS; ++i) {
			sum += data[i].sum;
		}
		fprintf(stdout, "%" PRId64 "\n", sum);

	}
	return 0;
}


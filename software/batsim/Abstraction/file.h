#ifndef FILE_H_
#define FILE_H_

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define FILE_ENTRY_MAX_NAMELENGTH		30

typedef enum {PTR_INT8, PTR_INT16, PTR_INT32, PTR_FLOAT, PTR_STRING} filePointerType_t;

typedef enum {FILE_OK, FILE_PARTIAL, FILE_ERROR} fileResult_t;

typedef struct {
	char name[FILE_ENTRY_MAX_NAMELENGTH];
	void *ptr;
	filePointerType_t type;
} fileEntry_t;

int8_t file_Init(void);
fileResult_t file_WriteParameters(char *filename, const fileEntry_t *paramList, uint8_t length);
fileResult_t file_ReadParameters(char *filename, const fileEntry_t *paramList, uint8_t length);


#endif /* FILE_H_ */

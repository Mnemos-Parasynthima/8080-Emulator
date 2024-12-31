#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "preprocessor.h"

#define COMMENT ';'


/**
 * Removes leading and trailing whitespace from the given line as well as any comments at the end
 * @param line Line to trim
 * @param len Length of line
 * @return The given line removed of leading and trailing space and comments
 */
static char* trimSpaceComments(char* line, int len) {
	char* start = line;
	while (isspace(*start)) {
		start++;
	}

	// In the case that the line is a comment line but the comment start later on
	if (*start == COMMENT) {
		free(line);
		return NULL;
	}

	char* end = line + len;
	while (end > start && isspace(*end)) {
		end--;
	}

	// There may be comments in the line, go from the beginning and search for ';'
	char* commentMark = start;
	while (*commentMark != COMMENT && commentMark <= end) commentMark++;

	size_t newSize = commentMark - start; // commentMark can either be end or before end indicating a comment
	char* trimmed = (char*) malloc(newSize + 1);
	if (!trimmed) handleError(ERR_MEM, FATAL, "Could not allocate space for trimmed line!\n");
	memcpy(trimmed, start, newSize);
	*(trimmed + newSize) = '\0';

	free(line);

	return trimmed;
}


char** preprocess(FILE* sourceFile, int* size) {
	const int DATA_INS_LINES = 128;
	int MAX_LEN = DATA_INS_LINES;
	const size_t SIZE = sizeof(char*) * DATA_INS_LINES;

	char** sourceLines = (char**) malloc(SIZE);
	if (!sourceLines) handleError(ERR_MEM, FATAL, "Could not allocate space for source lines array!\n");

	int sourceLinesIdx = 0;

	char* line = NULL;
	size_t n;

	ssize_t read = getline(&line, &n, sourceFile);
	while (read != -1) {
		// Skip empty lines or complete comment lines
		if (*line != '\n' || *line == COMMENT) {
			// Replace \n with \0
			*(line + read - 1) = '\0';

			// Passing in read - 1 since \n turned to \0, reducing len
			line = trimSpaceComments(line, read - 1);
			
			// line is only null in the case that it is a comment line with leading space
			if (line) {
				// No more space in the array, realloc it
				if (sourceLinesIdx == MAX_LEN) {
					// Increase space by DATA_INS_LINES
					char** temp = (char**) realloc(sourceLines, sizeof(char*) * (MAX_LEN + DATA_INS_LINES));
					if (!temp) handleError(ERR_MEM, FATAL, "Could not reallocate space for source line array!\n");
					sourceLines = temp;

					MAX_LEN += DATA_INS_LINES;
				}

				// Add the line to the array
				// Note that strcpy/memcpy is not done since that would require to allocate new memory
				// Why allocate new memory when getline already allocated memory for that line
				// However, getline allocates a buffer, meaning most likely, there is internal fragmentation
				// as most of that is not occupied
				// So a solution could just to allocate just enough space
				// Thus, a space-time tradeoff:
				// Use more time (to allocate) to use less space or
				// Use more space to use less time (to allocate)
				// Sticking with using more space for less time
				sourceLines[sourceLinesIdx] = line;

				sourceLinesIdx++;
			}
		}

		// Since sourceLines is set to line and getline may reuse(?) the same address,
		// Making it null indirectly will force getline to use a new address
		char** _line = &line;
		*_line = NULL;

		read = getline(&line, &n, sourceFile);
	}

	*size = sourceLinesIdx;

	return sourceLines;
};
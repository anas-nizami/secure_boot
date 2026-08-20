/* Parse NIST SHA-256 short-message response files (SHA256ShortMsg.rsp). */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

static void trim(char *s)
{
	char *p = s;
	while (isspace((unsigned char)*p))
		++p;
	if (p != s)
		memmove(s, p, strlen(p) + 1);

	p = s + strlen(s);
	while (p > s && isspace((unsigned char)p[-1]))
		*--p = '\0';
}

static int is_hex_string(const char *s)
{
	if (*s == '\0')
		return 1;
	for (; *s; ++s) {
		if (!isxdigit((unsigned char)*s))
			return 0;
	}
	return 1;
}

size_t parse_rsp_vectors(const char *path, struct test_vector *vectors, size_t max_vectors)
{
	FILE *file = fopen(path, "r");
	char line[4096];
	unsigned long block = 0;
	int length = -1;
	struct test_vector tv = {0};
	size_t nvectors = 0;

	if (!file) {
		perror(path);
		return 0;
	}

	while (fgets(line, sizeof line, file)) {
		char *equals;
		trim(line);
		if (line[0] == '\0' || line[0] == '#' || line[0] == '[')
			continue;

		equals = strchr(line, '=');
		if (!equals)
			continue;
		*equals++ = '\0';
		trim(line);
		trim(equals);

		if (strcmp(line, "Len") == 0) {
			length = atoi(equals);
			tv.length = length;
			tv.msg[0] = tv.digest[0] = '\0';
		} else if (strcmp(line, "Msg") == 0) {
			if (strlen(equals) >= sizeof tv.msg || !is_hex_string(equals)) {
				fprintf(stderr, "Invalid Msg in block %lu\n", block + 1);
				continue;
			}
			strcpy(tv.msg, equals);
		} else if (strcmp(line, "MD") == 0) {
			if (strlen(equals) >= sizeof tv.digest || !is_hex_string(equals)) {
				fprintf(stderr, "Invalid MD in block %lu\n", block + 1);
				continue;
			}
			strcpy(tv.digest, equals);

			if (length >= 0) {
				if (nvectors < max_vectors) {
					vectors[nvectors++] = tv;
				} else {
					fprintf(stderr, "Too many vectors, dropping block %lu\n", block + 1);
				}
				++block;
				length = -1;
			}
		}
	}

	fclose(file);
	return nvectors;
}

//
// Created by Ali Hussain Hitawala on 1/25/16.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/mman.h>
#include <limits.h>

#define NUM_LINES 10000000

int parseCommandLineArgs(int argc, char *pString[], char **string, long *marker);

void readFileAndStruct(const char *file, long marker, int *pInt);

typedef struct _marker_line {
    char *marker;
    char *line;
} marker_line;

marker_line *line_structs = NULL;

marker_line getStructElement(char *line, long marker);

int compare(const void *, const void *);

int getNumberLines(char *a, off_t length);

int getMaxLengthMarker(int size);

void sort_r(int size, int max_length);

void rad_sort(int size);

void errorOutput(char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    char *inputFile;// = malloc(100 * sizeof(char*));
    long marker = 1;
    int i = 0;
    if (parseCommandLineArgs(argc, argv, &inputFile, &marker) != 0) {
        errorOutput("Error: Bad command line parameters");
    }
    int size_struct = 0;
    readFileAndStruct(inputFile, marker, &size_struct);
    rad_sort(size_struct);
//    qsort(line_structs, size_struct, sizeof(marker_line), compare);
    for (i = 0; i < size_struct; i++) {
        printf("%s\n", line_structs[i].line);
    }
    free(line_structs);
    return 0;
}

int compare(const void *s1, const void *s2) {
    marker_line *r1 = (marker_line *) s1;
    marker_line *r2 = (marker_line *) s2;
    int cmp = strcmp(r1->marker, r2->marker);
    return cmp;
}

void readFileAndStruct(const char *fileName, long marker, int *size_struct) {
    int fp;
    fp = open(fileName, O_RDONLY, S_IRWXU);
    if (fp == -1) {
        fprintf(stderr, "Error: Cannot open file %s\n", fileName);
        exit(1);
    }
    struct stat sb;
    if (fstat(fp, &sb) == -1)
        fprintf(stderr, "Error: fstat - Cannot open file %s\n", fileName);
    off_t length = sb.st_size;
    if (length == 0) //empty file
        return;
    char *addr = mmap(NULL, length, PROT_READ, MAP_PRIVATE, fp, 0);
    madvise(addr, length, POSIX_MADV_SEQUENTIAL);
    if (addr == MAP_FAILED) {
        fprintf(stderr, "Error: Cannot open file %s\n", fileName);
        exit(1);
    }
    char *a = addr;
//    printf("%d\n", num_l);
    int num_lines = getNumberLines(a, length);
//    int num_lines = (int) (length < NUM_LINES ? length : NUM_LINES);
    line_structs = malloc(num_lines * sizeof(marker_line));
    if (line_structs == NULL)
        errorOutput("malloc failed");
    int i = 0;
    int struct_index = 0;
    while (1) {
        int j = 0;
        char *line = malloc(130 * sizeof(char *));
        if (line == NULL)
            errorOutput("malloc failed");
        while (i <= sb.st_size && a[i] != '\n') {
            line[j++] = a[i];
            i++;
            if (j == 128) {
                errorOutput("Line too long");
            }
        }
        line[j] = '\0';
        if (i > sb.st_size) {
            if (strlen(line) > 0) {
                line_structs[struct_index++] = getStructElement(line, marker);
            }
            break;
        }
        if (strlen(line) == 0) {
            line_structs[struct_index].line = line;
            line_structs[struct_index].marker = malloc(1 * sizeof(char *));
        }
        else {
            line_structs[struct_index] = getStructElement(line, marker);
        }
        struct_index++;
        i++;
    }
    *size_struct = struct_index;
    close(fp);
}

int getNumberLines(char *a, off_t length) {
    int i = 0;
    int result = 0;
    while (i <= length) {
        if (a[i++] == '\n')
            result++;
    }
    if (result != 0 && a[length - 1] != '\n')
        result++;
    return result;
}

marker_line getStructElement(char *line, long marker) {
    marker_line result;
    result.line = line;
    char copyLine[strlen(line)];
    strcpy(copyLine, line);
    char *token = strtok(copyLine, " ");
    int j = 1;
    int isAssigned = 0;
    char *lastToken = token;
    while (token) {
        lastToken = token;
        if (j == marker) {
            result.marker = malloc(strlen(token) * sizeof(char *));
            if (result.marker == NULL)
                errorOutput("malloc failed");
            strcpy(result.marker, token);
            isAssigned = 1;
            break;
        }
        token = strtok(NULL, " ");
        j++;
    }
    if (!isAssigned) {
        result.marker = malloc(strlen(lastToken) * sizeof(char *));
        if (result.marker == NULL)
            errorOutput("malloc failed");
        strcpy(result.marker, lastToken);
    }
    return result;
}

static long parseLong(const char *str) {
    errno = 0;
    char *temp;
    long val = strtol(str, &temp, 0);
    if (temp == str || *temp != '\0' || (val == LONG_MIN && errno == ERANGE))
        return -1;
    if (val == LONG_MAX && errno == ERANGE)
        return 1000;
    return val;
}

int parseCommandLineArgs(int argc, char *argv[], char **file, long *marker) {
    if (argc > 3 || argc < 2)
        return 1;
    if (argc == 2) {
        *file = argv[1];
        return 0;
    }
    char dash1 = argv[1][0], dash2 = argv[2][0];
    if (dash1 == '-' || dash2 == '-') {
        char *m;
        if (dash1 == '-') {
            *file = argv[2];
            m = argv[1] + 1;
        }
        else {
            *file = argv[1];
            m = argv[2] + 1;
        }
        long temp = 1;
        if ((temp = parseLong(m)) < 1)
            return 1;
        *marker = temp;
    }
    else {
        return 1;
    }
    return 0;
}

void rad_sort(int size) {
    int max_element = getMaxLengthMarker(size);
    sort_r(size, max_element);
}

int getMaxLengthMarker(int size) {
    int i = 0;
    int max = 0;
    for (i = 0; i < size; i++) {
        int temp = (int) strlen(line_structs[i].marker);
        if (temp > max)
            max = temp;
    }
    return max;
}

void sort_r(int size, int max_length) {
    marker_line *temp = malloc(size * sizeof(marker_line));
    int d;
    for (d = max_length - 1; d >= 0; d--) {
        int count[256];
        memset(count, 0, sizeof(count));
        int i = 0;
        for (; i < size; i++) {
            char *marker = line_structs[i].marker;
            if (strlen(marker) < d + 1) {
                count[1]++;
            }
            else
                count[line_structs[i].marker[d] + 1]++;
        }
        int k = 1;
        for (; k < 256; k++)
            count[k] += count[k - 1];
        i = 0;
        for (; i < size; i++) {
            char *marker = line_structs[i].marker;
            int index = 0;
            if (strlen(marker) < d + 1)
                index = 0;
            else
                index = (int) marker[d];
            temp[count[index]++] = line_structs[i];
        }
        i = 0;
        for (; i < size; i++)
            line_structs[i] = temp[i];
    }
}
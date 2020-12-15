#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "libunix.h"

// read entire file into buffer.  return it.   zero pads to a 
// multiple of 4.
//
// make sure to cleanup!
uint8_t *read_file(unsigned *size, const char *name) {
    struct stat st;
    stat(name, &st);
    *size = st.st_size + (st.st_size % 4 == 0 ? 0 : (4 - st.st_size % 4));

    uint8_t *buf = calloc(*size, sizeof(uint8_t));
    FILE *file;
    size_t nread;

    file = fopen(name, "r");
    if (!file) {
        return 0;
    }
    nread = fread(buf, 1, st.st_size, file);
    fclose(file);
    return buf;
}

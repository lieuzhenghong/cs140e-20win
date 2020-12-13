// engler, cs140e.
#include <assert.h>
#include <fcntl.h>
#include <string.h>

#include "libunix.h"

#define _SVID_SOURCE
#include <dirent.h>
static const char *ttyusb_prefixes[] = {
	"ttyUSB",	// linux
	"cu.SLAB_USB", // mac os
	0
};

static int filter(const struct dirent *d) {
    // scan through the prefixes, returning 1 when you find a match.
    // 0 if there is no match.
    const char* file_name = d->d_name;
    for (int i = 0; i < 2; i++) {
        const char* prefix = ttyusb_prefixes[i];
        if (strstr(file_name, prefix) != NULL) {
            return 1;
        }
    }
    return 0;
}

// find the TTY-usb device (if any) by using <scandir> to search for
// a device with a prefix given by <ttyusb_prefixes> in /dev
// returns:
//  - device name.
// panic's if 0 or more than 1.
//
char *find_ttyusb(void) {
    char *p;

    // use <alphasort> in <scandir>
    // return a malloc'd name so doesn't corrupt.
    struct dirent **fileListTemp;

    int num_dirs = scandir("/dev", &fileListTemp, filter, alphasort);

    if (num_dirs == 0) {
        perror("no file found");
        exit(1);
    } else if (num_dirs > 1) {
        perror("to many file found");
        exit(1);
    }
    p = (char*)malloc(strlen(fileListTemp[0]->d_name)+1);
    strcpy(p, fileListTemp[0]->d_name);

    free(fileListTemp[0]);
    free(fileListTemp);
    return p;
}

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAXFILES    16
#define MAXBLOCKLEN 16

#define DEFAULT_BLOCKLEN 8

unsigned long baseaddr = 0;
int bpr = DEFAULT_BLOCKLEN;    /* block size for comparison */
int files = 0;
char names[MAXFILES][PATH_MAX];

int bindiff(void)
{
unsigned long sz[MAXFILES],i,ii,len,addr;
int iii;
unsigned char v[MAXFILES][MAXBLOCKLEN];
FILE *fh[MAXFILES];
int d;
int df=0;
int result = 0;
int diffs = 0;

    // - open all files
    // - determine length of all files
    for(i = 0; i < files; i++) {

        fh[i] = fopen(names[i], "rb");

        if(fh[i] == NULL) {
            fprintf(stderr, "Can't open \"%s\".\n", names[i]);
            exit(-1);
        }
        if(fseek(fh[i], 0, SEEK_END)<0) {
            fprintf(stderr, "Seek error.\n");
            exit(-1);
        }
        sz[i] = ftell(fh[i]);
        if(fseek(fh[i], 0, SEEK_SET)<0) {
            fprintf(stderr, "Seek error.\n");
            exit(-1);
        }
    }

    // table with all files and their length
    fprintf(stdout,"file: ");
    for(i=0;i<files;i++){
        fprintf(stdout,"%10s ",names[i]);
    }
    fprintf(stdout,"\n");
    fprintf(stdout,"len : ");
    for(i=0;i<files;i++){
        fprintf(stdout,"%10ld ", sz[i]);
        if (sz[0] != sz[i]) {
            result = -1;
        }
    }
    fprintf(stdout,"\n");

    if (result < 0) {
        fprintf(stderr, "error: file sizes are not equal\n");
    }

    // show binary differences
    fprintf(stdout,"offset     ");
    for(i=0;i<files;i++){
        fprintf(stdout,"%10s ",names[i]);
    }
    fprintf(stdout,"\n");

    len = sz[0];

    for(ii = 0; ii < (len / bpr); ii++){

        // load 'bpr' bytes from each file
        for(iii = 0; iii < bpr; iii++){
            for(i = 0; i < files; i++){
                v[i][iii] = fgetc(fh[i]);
            }
        }

        // compare loaded blocks
        df = 0;
        for(iii = 0; iii < bpr; iii++) {
            d = v[0][iii];
            for(i = 1; i < files; i++){
                if(v[i][iii] != d){
                    df = 1;
                     // i = files;
                     // iii = bpr;
                    diffs++;
                    result = -1;
                }
            }
        }

        // if the previous comparison detected a difference, show output
        if(df == 1) {

            addr = baseaddr + (ii * bpr);

            fprintf(stdout,"0x%08lx: ", addr);

            for(i = 0; i < files; i++) {
                // show hex value(s)
                for(iii = 0; iii < bpr; iii++) {
                    printf("%02x ",v[i][iii]);
                }

                for(iii = 0; iii < bpr; iii++) {
                    int val = v[i][iii];
                    if (!isgraph(val)) {
                        val = '.';
                    }
                    printf("%c", val);
                }
                if (i < (files - 1)) {
                    printf("  ");
                }
            }

            fprintf(stdout,"\n");
        }

    }

    if (result < 0) {
        fprintf(stderr, "error: files are not equal (%d differences).\n", diffs);
    }

    for(i=0;i<files;i++){
        fclose(fh[i]);
    }

    return result;
}

void usage(char *argv)
{
    printf(
        "bindiff v0.2 - compare binary files\n"
        "Usage: %s <options> [files] \n"
        "--addr <addr>          set base address\n"
        "--blocklen <bytes>     number of bytes per block\n"
        , argv
    );
}

int main(int argc, char *argv[]) {

int i;
int first;

    if(argc < 3) {
        usage(argv[0]);
        exit(-1);
    }

    for(i = 1; i < argc; i++) {
        if ((argv[i][0] == '-') && (argv[i][1] == '-')) {
            if (!strcmp("--addr", argv[i])) {
                i++;
                baseaddr = strtoul(argv[i], NULL, 0);
            } else if (!strcmp("--blocklen", argv[i])) {
                i++;
                bpr = strtoul(argv[i], NULL, 0);
                if (bpr > MAXBLOCKLEN) {
                    fprintf(stderr, "error: please do not use more than %d bytes.\n", MAXBLOCKLEN);
                }
            } else {
                fprintf(stderr, "error: unknown option '%s'\n", argv[i]);
                exit(-1);
            }
        } else {
            break;
        }
    }

    first = i;
    files = argc - i;

    if (files > MAXFILES) {
        fprintf(stderr, "error: please do not use more than %d files.\n", MAXFILES);
    }

    for(; i < argc; i++){
        strcpy(names[i - first],argv[i]);
    }

    return bindiff();
}

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include "compiler.h"
#include "opt_parser.h"


int main(int argc, char** argv) {
    //if (argc < 2) {
    //    fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
    //    exit(EXIT_FAILURE);
    //}
    opts_t* opts = parse_opts(argc, argv);
    compile(opts);
    exit(EXIT_SUCCESS);
}
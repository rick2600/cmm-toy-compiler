#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include "compiler.h"
#include "opt_parser.h"


int main(int argc, char** argv) {
    opts_t* opts = parse_opts(argc, argv);
    compile(opts);
    exit(EXIT_SUCCESS);
}
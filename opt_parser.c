#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include "opt_parser.h"

opts_t opts;

static void print_help(const char* prog_name) {
    printf(
        "Usage: %s [options] <filename>\n"       \
        "    --help         Print help menu\n"   \
        "    --token        Show tokens\n"       \
        "    --ast          Show generated AST\n"\
        "    --symbols      Show symbol table\n",\
        prog_name
    );
}

opts_t* parse_opts(int argc, char** argv) {
    opts.tokens = false;
    opts.ast = false;
    opts.symbols = false;
    opts.filename = NULL;

    static struct option long_opts[] = {
        {"help",      no_argument, 0, 'h'},
        {"tokens",    no_argument, 0, 't'},
        {"ast",       no_argument, 0, 'a'},
        {"symbols",   no_argument, 0, 's'},
        {0,           0,           0,  0 }
    };

    int opt = 0;
    int long_idx = 0;

    while ((opt = getopt_long(argc, argv, "htas", long_opts, &long_idx)) != -1) {
        switch (opt) {
            case 'h' :
                print_help(argv[0]);
                exit(EXIT_SUCCESS);
                break;

            case 't' : opts.tokens  = true; break;
            case 'a' : opts.ast     = true; break;
            case 's' : opts.symbols = true; break;

            default:
                exit(EXIT_FAILURE);
        }
    }

    opts.filename = argv[optind];

    return &opts;
}

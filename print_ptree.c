#include <stdio.h>
#include <stdlib.h>

#include "ptree.h"


int main(int argc, char **argv) {
    // TODO: Update error checking and add support for the optional -d flag
//    printf("Usage:\n\tptree [-d N] PID\n");
    if (!(argc == 2 || argc == 4) || (argc == 4 && argv[1][1] != 'd')){
        fprintf(stderr, "Usage:\n\tptree [-d N] PID\n");
        return 1;
    }

    // NOTE: This only works if no -d option is provided and does not
    // error check the provided argument or generate_ptree. Fix this!

    struct TreeNode *root = NULL;
    int return_check = 0;
    if (argc == 2) {
        return_check = generate_ptree(&root, strtol(argv[1], NULL, 10));
        print_ptree(root, 0);
    } else {
        return_check = generate_ptree(&root, strtol(argv[3], NULL, 10));
        print_ptree(root, strtol(argv[2], NULL, 10));
    }
    if (return_check != 0){
        return 2;
    }

    return 0;
}


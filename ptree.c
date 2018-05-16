#include <stdio.h>
// Add your other system includes here.
#include <sys/stat.h>
#include <stdlib.h>
#include "ptree.h"

// Defining the constants described in ptree.h
const unsigned int MAX_PATH_LENGTH = 1024;

// If TEST is defined (see the Makefile), will look in the tests
// directory for PIDs, instead of /proc.
#ifdef TEST
    const char *PROC_ROOT = "tests";
#else
    const char *PROC_ROOT = "/proc";
#endif

/*
 * Returns void and assigns the rightmost sibling from get_sib_from to
 * sib_to_be.
 */
void get_sibling(struct TreeNode *get_sib_from, struct TreeNode *sib_to_be){
    if (get_sib_from->sibling == NULL){
        sib_to_be->sibling =  get_sib_from->sibling;
    }
    else {
        get_sibling(get_sib_from->sibling, sib_to_be);
    }
}

/*
 * Makes sibling_to_be the sibling of node or the node's rightmost sibling.
 * The function does not return anything.
 */
void make_sibling(struct TreeNode *node, struct TreeNode *sibling_to_be){

    if (node->sibling == NULL){
        node->sibling = sibling_to_be;
    } else {
        make_sibling(node->sibling, sibling_to_be);
    }
}

/*
 * Creates a PTree rooted at the process pid.
 * The function returns 0 if the tree was created successfully
 * and 1 if the tree could not be created or if at least
 * one PID was encountered that could not be found or was not an
 * executing process.
 */
int generate_ptree(struct TreeNode **root, pid_t pid) {
    // Here's a way to generate a string representing the name of
    // a file to open. Note that it uses the PROC_ROOT variable.

    char procfile[MAX_PATH_LENGTH + 1];
    if (sprintf(procfile, "%s/%d/exe", PROC_ROOT, pid) < 0) {
        fprintf(stderr, "sprintf failed to produce a filename\n");
        return 1;
    }
    int total_errors = 0; // keeps track of the total errors so that the function can return either 1 or 0.
    *root = malloc(sizeof(struct TreeNode));

    struct stat buf;
    int return_value;

    return_value = lstat(procfile, &buf); // check for the link
    if (return_value != 0){
        fprintf(stderr, "The file is not linked\n");
        *root = NULL;
        return 1;
    }
    (*root)->pid = pid;
    if (sprintf(procfile, "%s/%d/cmdline", PROC_ROOT, pid) < 0) {
        fprintf(stderr, "sprintf failed to produce a filename for the cmdline file\n");
        return 1;

    }
    int file_close_check = 0;
    FILE *file = fopen(procfile, "rb");
    if (file == NULL){
        fprintf(stderr, "the file cmdline does not exist\n");
        (*root)->name = NULL;
        total_errors++;
    } else {  // execute if the file exists
        (*root)->name = malloc(sizeof(char) * (MAX_PATH_LENGTH + 1));
        int items_read = (int) fread((*root)->name, sizeof(char), MAX_PATH_LENGTH, file); // read the name from the
        // cmdline file
        if (items_read == 0) {
            (*root)->name = NULL;
        } else {
            (*root)->name[items_read] = '\0';
        }

        file_close_check = fclose(file);
        if (file_close_check != 0) {
            fprintf(stderr, "The file cmdline did not close properly\n");
            return 1;
        }
    }
    if (sprintf(procfile, "%s/%d/task/%d/children", PROC_ROOT, pid, pid) < 0) {
        fprintf(stderr, "sprintf failed to produce a filename for the children file\n");
        return 1;

    }
    (*root)->sibling = NULL;
    file = fopen(procfile, "r");
    if (file == NULL){
        fprintf(stderr, "the file children does not exist\n");
        total_errors++;
        (*root)->child = NULL;

    } else { // execute if the children file exists

        int pid1;
        int child_counter = 0; // counts the number of children


        while (fscanf(file, "%d", &pid1) > 0) { // read ints from the file until there are none to read
            if (child_counter == 0) {
                (*root)->child = malloc(sizeof(struct TreeNode));
                child_counter = generate_ptree(&((*root)->child), pid1);
                if (child_counter == 0) { // assign the child if and only if it's a valid child
                    child_counter++;
                } else {
                    child_counter = 0;
                    total_errors++;
                }
            } else { // makes use of the helper functions to get the appropriate sibling and assign that sibling to the
                // rightmost sibling currently present in the tree
                struct TreeNode *new_sib = malloc(sizeof(struct TreeNode));
                get_sibling((*root)->child, new_sib);
                int value_checker = generate_ptree(&new_sib, pid1);
                if (value_checker == 0) {
                    make_sibling((*root)->child, new_sib); // make a sibling only if it is valid
                } else {
                    total_errors++;
                }
                child_counter++;  // to keep track which is the first child and which are its siblings
            }
        }

        file_close_check = fclose(file);
        if (file_close_check != 0) {
            fprintf(stderr, "The file children did not close properly\n");
            return 1;
        }

        if (child_counter == 0) { // means there are no children
            (*root)->child = NULL;

        }
    }
    if (total_errors != 0){
        return 1;
    }
    return 0;
}


/*
 * Prints the TreeNodes encountered on a preorder traversal of an PTree
 * to a specified maximum depth. If the maximum depth is 0, then the
 * entire tree is printed.
 */
void print_ptree(struct TreeNode *root, int max_depth) {
    // Here's a way to keep track of the depth (in the tree) you're at
    // and print 2 * that many spaces at the beginning of the line.
    if (root == NULL){
        return;
    }
    static int depth = 0;
    if (max_depth >= 0) {

        if (max_depth == 0) {
            printf("%*s", depth * 2, "");
            if (root->child == NULL) {
                printf("%d: %s\n", root->pid, root->name);
            } else {
                int i = depth;  // keeps track of the depth before
                depth++;
                printf("%d: %s\n", root->pid, root->name);
                print_ptree(root->child, 0); // make the tree of the child
                depth = i;  // assigns the current depth
            }
            if (root->sibling != NULL) {
                print_ptree(root->sibling, 0);
            }
        } else {
            if (max_depth >= depth) {
                printf("%*s", depth * 2, "");
                if (root->child == NULL) {
                    printf("%d: %s\n", root->pid, root->name);
                } else {
                    int i = depth;
                    depth++;
                    printf("%d: %s\n", root->pid, root->name);
                    print_ptree(root->child, max_depth);
                    depth = i;
                }
                if (root->sibling != NULL) {
                    print_ptree(root->sibling, max_depth);
                }
            }
        }
    }
}

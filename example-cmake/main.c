#include <stdlib.h>

#include "lib.h"


int main(int argc, char** argv)
{
    (void)argc;

    int r = print_data(argv[0]);
    if(r == 0)
        return EXIT_FAILURE;
    else
        return EXIT_SUCCESS;
}

#include "serial_blue.h"

//#define SERIAL_BLUE_SHOW_MSG

void serial_blue_write_string(char *c_str)
{
    #ifdef SERIAL_BLUE_SHOW_MSG
    printf("serial blue:");
    printf(c_str);
    printf("\n");
    #endif // SERIAL_BLUE_SHOW_MSG
}

/* compile:
 * gcc time_console.c -o time_console
 * ./time_console
 */


#include <stdio.h>
#include <time.h>

int main()
{
    //get time with built in C time(). Then convert time to
    //string using ctime()
    time_t timer;
    time(&timer);

    printf("Local time is: %s", ctime(&timer));

    return 0;
}

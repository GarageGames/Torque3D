#include <sys/types.h>

#define KB ((off_t)(1024))
#define MB ((off_t)(KB*1024))
#define GB ((off_t)(MB*1024))
int tb[((GB+GB+GB) > GB) ? 1 : -1];

int main()
{ return 0; }

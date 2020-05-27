# write bin file(pe or elf) to c file or header file
# support android & ios & win & linux & mac os x & athor like unix'os
# write in pure c 
# build on win
open bin2c.sln with vs 2015
# build on other's os
make 
# upx:
please install upx if you can pack the bin file  
# output
bin_data.h:  
bin_data.c
```c
const int k_bin_file_size = 99840;  
const unsigned char g_bin_data[] = {}  
```
# usage:
```c
#include "bin_data.h"
size_t wrote_size = fwrite(g_bin_data, 1, k_bin_file_size, f);
assert(wrote_size == k_bin_file_size)
```

    
    

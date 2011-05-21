/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2009 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#define ENABLE(FEATURE) (defined ENABLE_##FEATURE && ENABLE_##FEATURE)



//this can be replace by hw buffer!
#define BufferAlloc(n) malloc(n)
#define BuffersAlloc(n, s) calloc(n, s)
#define BufferFree(p) free(p)
#define BufferCopy(d, s, n) memcpy(d, s, n)


//math PI value
#define PI 3.14159265358979323846f

#define MAX(x, y)           (((x) > (y))?(x):(y))
#define MIN(x, y)           (((x) < (y))?(x):(y))



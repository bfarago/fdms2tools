/*Writed by Barna Farago <brown@weblapja.com>
*/
#ifndef _FDMS2COMMON_H
#define _FDMS2COMMON_H

#ifdef __APPLE__
//#include <machine/types.h>
//#include <alloc.h>
#else
// For linux open()
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifndef WIN32
// For unix standard close
#include <unistd.h>
// For mmap
#include <sys/mman.h>
#else
#define MAP_FAILED NULL
#include <windows.h>
#endif
// For printf()
#include <stdio.h>
// For atoi, atol
#include <stdlib.h>
// For errno
#include <errno.h>
#endif //notapple

#ifndef WIN32
//apple and linux
#define t1_toffset long long
#define MAPPAGELENGTH 0x7FFFFFFF
#else  //WIN32
#define t1_toffset __int64
#define MAPPAGELENGTH 0x07FFFFFF
#endif //WIN32

#define VUMETERMAX 20
#define VUMETERDIV (0x7FFF/VUMETERMAX)
#define FOSTEXFRAMERATE (25)
#define FOSTEXSAMPLERATE 44100
#define FOSTEXMAXCHANNELS 8
#define FOSTEXMAXPROGRAMM 5
//#define VERBOSE_MAPPING
#endif

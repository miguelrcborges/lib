w32(u32) GetStdHandle(u32 nhandle);

#define getStdIn() (GetStdHandle((u32)-10))
#define getStdOut() (GetStdHandle((u32)-11))
#define getStdErr() (GetStdHandle((u32)-12))

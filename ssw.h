#ifndef ssw_h__
#define ssw_h__

extern struct cache {
char t;
char b;
int txt;
int x;
int y;
int w;
int h;
};

extern unsigned long LHEX(char v[7]); /* These could be regular longs */
extern unsigned long HEX(char v[7]);  /* but don't want to change now */
extern unsigned long RGB(int, int, int);

extern void XI(const char *, const char *, const int[4], char, char, int, int);
extern void XX(char);
extern void RegisterTemplate(void (*func)(void *), int);
extern void Align(int *, int);
extern void AlignID(int *, int);

extern void makeBuffer(void *, int, int, int, int, char);
extern void copySiblingBuffer(void *);
extern void EditBuffer(void *);
extern void EditMask(void *);
extern void RegionFill(void *, int, int, int, int, long long);
extern void RegionScarf(void *, int, int, int, int, long long);
extern void DrawPixel(void *, int, int);
extern void RegionFromBits(void *, char *);
extern void Default(void *);
#endif

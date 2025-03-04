#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <stdio.h>

struct coord {
int x, y;
};

struct gcontext {
GC gc;
char uses_mask;
Drawable planet;
Pixmap   planet_mask;
struct coord planet_loca;
int height, width;
};

struct user_context {
char isEditingBuffer;
struct orbital *current_orbital;
};

struct orbital {
Display *dis;
struct gcontext *graphics;
// may need to steal this pointer
// in the future (think orange tile)

XEvent event;
KeySym key;
char text[255];

void (*select)(void *);
void (*unselect)(void *);
void (*draw)(void *); // looking less and less useful
void (*instantiate)(void *);
int identity;

struct orbital *next;
struct orbital *suborbital;
struct orbital *root_orbital;

};

struct cache {
char t;
char b;
int txt;
int x;
int y;
int w;
int h;
};

static struct orbital *head_orbital = 0;

void Default(void *);

void XI(const char *title, const char *subtitle, const int dimensions[4], char isRoot, char isWindow, char base, int id) {

  struct orbital **base_orbital = &head_orbital;
  // to permawrite allocation onto head_orbital
  // we need head_orbital's address, not its value

  while (base--) {
  base_orbital = &( (*base_orbital)->suborbital );
  }

  struct orbital *allocation;

  allocation = malloc(sizeof(struct orbital));
  allocation->graphics = malloc(sizeof(struct gcontext));

  allocation->graphics->planet_loca.x = dimensions[0]; // will these be max screen size for all windows?
  allocation->graphics->planet_loca.y = dimensions[1];
  allocation->graphics->width  = dimensions[2];
  allocation->graphics->height = dimensions[3];

  allocation->suborbital = 0;
  allocation->identity = id;

  allocation->root_orbital = head_orbital ? head_orbital : allocation;
  allocation->instantiate = head_orbital ? head_orbital->instantiate : &Default;

  allocation->dis = head_orbital ? head_orbital->dis : XOpenDisplay(":0");

  if (isWindow || isRoot) { // create window or use instantiate
  allocation->graphics->planet =
      isRoot
      ? RootWindow(allocation->dis, DefaultScreen(allocation->dis))
      : XCreateWindow(allocation->dis, DefaultRootWindow(allocation->dis), dimensions[0],
                                                                           dimensions[1],
                                                                           dimensions[2],
                                                                           dimensions[3],
      /*border*/5, /*depth*/24, /*class*/0, /*Visual*/0, /*attr values*/0, (XSetWindowAttributes *)0);

    if (!isRoot) {
    XSetStandardProperties(allocation->dis, allocation->graphics->planet, title, subtitle, 0, 0, 0, 0);
    XSelectInput(allocation->dis, allocation->graphics->planet, ExposureMask|ButtonPressMask|KeyPressMask);
    XClearWindow(allocation->dis, allocation->graphics->planet);
    XMapRaised(allocation->dis,   allocation->graphics->planet);
    }

  XFlush(allocation->dis);
  } else {
  struct user_context context = {1, allocation}; // default is not editing mask
  (*allocation->root_orbital->instantiate)( &context ); // function should copy or create Pixmap
  }
  // if Drawable is still not set, should do something here
  allocation->graphics->gc = XCreateGC(allocation->dis, allocation->graphics->planet, /*GC values*/0, (XGCValues *)0);


  if (*base_orbital == 0) {
  *base_orbital = allocation;
  allocation->next = allocation;
  return;
  }

  allocation->next = (*base_orbital)->next;
  (*base_orbital)->next = allocation;

  *base_orbital = allocation; // hook this as the new suborbital

}

void Align(int *alignment, int align_granularity) { // aligns by delta
  struct orbital **base = &head_orbital;
  for (int i = 0; i < align_granularity; i++) {
    while (alignment[i]--) {
    *base = (*base)->next;
    }
  base = &( (*base)->suborbital );
  }
}

void AlignID(int *alignment, int align_granularity) { // may spin forever looking for a given ID
  struct orbital **base = &head_orbital;
  for (int i = 0; i < align_granularity; i++) {
    while (alignment[i] != (*base)->identity) {
    *base = (*base)->next;
    }
  base = &( (*base)->suborbital );
  }
}

int DDepth(Display *dis, Drawable intake) {
XWindowAttributes attr;
XGetWindowAttributes(dis, intake, &attr);
return attr.depth;
}

int WW(Display *dis, Drawable intake) {
XWindowAttributes attr;
XGetWindowAttributes(dis, intake, &attr);
return attr.width;
}

int WH(Display *dis, Drawable intake) {
XWindowAttributes attr;
XGetWindowAttributes(dis, intake, &attr);
return attr.height;
}

void makeBuffer(void *handle, int width, int height, int parentX, int parentY, char needMask) {
struct user_context *cxt = (struct user_context *)handle;
cxt->current_orbital->graphics->planet =
  XCreatePixmap(
    cxt->current_orbital->dis,
    cxt->current_orbital->root_orbital->graphics->planet,
    width, height,
    DDepth(cxt->current_orbital->dis, cxt->current_orbital->root_orbital->graphics->planet)
  );

  cxt->current_orbital->graphics->width  = width;
  cxt->current_orbital->graphics->height = height;
  cxt->current_orbital->graphics->planet_loca.x = parentX; // for XCopyArea calls
  cxt->current_orbital->graphics->planet_loca.y = parentY;

  if (needMask) {
    cxt->current_orbital->graphics->planet_mask =
    XCreatePixmap(
      cxt->current_orbital->dis,
      cxt->current_orbital->root_orbital->graphics->planet,
      width, height,
      2
    );
  }
}

void copySiblingBuffer(void *handle) {
struct user_context *cxt = (struct user_context *)handle;

}

void EditBuffer(void *handle) {
struct user_context *cxt = (struct user_context *)handle;
cxt->isEditingBuffer = 1;
}

void EditMask(void *handle) {
struct user_context *cxt = (struct user_context *)handle;
cxt->isEditingBuffer = 0;
}

void RegionFill(void *handle, int x, int y, int height, int width, long long rgb) {
struct user_context *cxt = (struct user_context *)handle;

}

void RegionScarf(void *handle, int x, int y, int height, int width, long long rgb) {
struct user_context *cxt = (struct user_context *)handle;

}

void DrawPixel(void *handle, int x, int y) {
struct user_context *cxt = (struct user_context *)handle;

}

void RegionFromBits(void *handle, char *bits) {
struct user_context *cxt = (struct user_context *)handle;

}

void RegisterTemplate(void (*template)(void *), int rootID) {
AlignID((int [1]){rootID}, 1);
head_orbital->instantiate = template;
}

void Default(void *handle) { // For now, assumes root orbital is Window
struct user_context *cxt = (struct user_context *)handle;
makeBuffer(handle, WW(cxt->current_orbital->dis, cxt->current_orbital->root_orbital->graphics->planet),
                   WH(cxt->current_orbital->dis, cxt->current_orbital->root_orbital->graphics->planet), 0, 0, 0);
}


void Graft(); // how would this even work with head_orbital?

void Draw() {
// draw all to buffer just above head_orbital by ascending twice and 
}

// add routines for clearing background
// changing background
// or returning orbital handle for exterior functions

void XX(char op) {
exit(op);
}



/*
void Clean(char d){
XFreeGC(context[d].dis, context[d].gc);
XDestroyWindow(context[d].dis, context[d].win);
XCloseDisplay(context[d].dis);
}

void RegionFill(int x, int y, int h, int w, unsigned long color, char d){
XSetForeground(context[d].dis, context[d].gc, color);
XFillRectangle(context[d].dis, context[d].win, context[d].gc, x , y, h, w);
}

void RegionScarf(int x, int y, int h, int w, unsigned long color, char d){
XSetForeground(context[d].dis, context[d].gc, color);
XDrawRectangle(context[d].dis, context[d].win, context[d].gc, x-1, y-1, h, w);
}

void Point(int x, int y, unsigned long color, char d) {
XSetForeground(context[d].dis, context[d].gc, color);
XDrawPoint(context[d].dis, context[d].win, context[d].gc, x, y);
}

// oh good god
void Eve(struct cache *cc, char d){
char kill = 0; 

cc->t =   0;
cc->b =   0;
cc->txt = 0;
cc->x =   0;
cc->y =   0;

XNextEvent(context[d].dis, &context[d].event);
	if (context[d].event.type==Expose) {
	cc->t = 3;
	}
        if (context[d].event.type==ButtonPress) {
        cc->t = 2;
        cc->b = context[d].event.xbutton.button;
        cc->x = context[d].event.xbutton.x;
        cc->y = context[d].event.xbutton.y;
        }
        if (context[d].event.type==2) {
	XLookupString(&context[d].event.xkey,context[d].text,255,&context[d].key,0);
                if (context[d].text[0]==27) { // failsafe quit
                        kill = 1;
                } else {
                cc->t = 1;
                cc->txt = context[d].key;
                }
        }
	if (kill){
	Clean(d);
	exit(0);
	}
}

int Pend(char d){
return XPending(context[d].dis);
}
*/


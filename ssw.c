#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <stdio.h>

/*
 *
 * Author: Matthew Louden
 *
 * Dates:
 *  March 2nd 2025 {XI Start}
 *  March 3rd 2025 {XI successful window}
 *  March 4th 2025 {Draw}
 *  March 5th 2025 {AlignXY, makeMask, GlorbHandle}
 *
 *
 * Notes:
 *  This library assumes all windows are the exact width and height of the root window
 *
 *  Question: How do we obtain the width and height of a window (on screen)?
 *    The answer to this question defeats the above notion
 *
 *  What if the answer is X11 exposure events? {Mar 5th: went with this idea}
 *
 *  Most tiling window managers make the dimensions field for XSetAttributes obsolete
 *
 *  Assumption: XSync, XPending, and XFlush will all eventually need to be used
 *
 *  Assumption: an exposure event is always generated at window creation
 *
 *  Assumption: The visable part of any window will never exceed the total root window width, height
 *
 *  Assumption: Can draw outside of Pixmap borders in the same way that one can draw outside a real window's bounds
 *
 *  Assumption: GXor will just work
 *
 *
 */

struct coord {
int x, y;
};

struct gcontext {
Drawable asset;
GC gc;
};

struct user_context {
char isEditingBuffer;
struct orbital *current_orbital;
};

struct event_node {
KeySym key;
XEvent event;
struct event_node *anterior;
struct event_node *posterior;
};

struct orbital {
Display *dis;

struct coord parent_mapping;
int asset_height, asset_width;
struct gcontext *planet; // prev graphics
struct gcontext *mask; // is already a boolean
// may need to steal this pointer
// in the future (think orange tile)

struct event_node *history;

void (*select)(void *);
void (*unselect)(void *);
void (*instantiate)(void *);
int identity;

struct orbital *next;
struct orbital *suborbital;
struct orbital *root_orbital;

};

enum funcType {Glorb, UnGlorb, InstaGlorb};

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
int WW(Display *, Drawable);
int WH(Display *, Drawable);

void XI(const char *title, const char *subtitle, const int dimensions[4], char isRoot, char isWindow, char base, int id) {

  struct orbital **base_orbital = &head_orbital;
  // to permawrite allocation onto head_orbital
  // we need head_orbital's address, not its value

  while (base--) {
  base_orbital = &( (*base_orbital)->suborbital );
  }

  struct orbital *allocation;

  allocation = malloc(sizeof(struct orbital));
  allocation->planet = malloc(sizeof(struct gcontext));
  allocation->mask   = 0;

  allocation->parent_mapping.x = dimensions[0]; // will these be max screen size for all windows?
  allocation->parent_mapping.y = dimensions[1];
  allocation->asset_width  = dimensions[2];
  allocation->asset_height = dimensions[3];

  allocation->suborbital = 0;
  allocation->identity = id;

  allocation->root_orbital = head_orbital ? head_orbital : allocation;
  allocation->instantiate = head_orbital ? head_orbital->instantiate : &Default;
  allocation->unselect = head_orbital ? head_orbital->unselect : 0;
  allocation->select = head_orbital ? head_orbital->select : 0;


  allocation->dis = head_orbital ? head_orbital->dis : XOpenDisplay(":0");

  if (isWindow || isRoot) { // create window or use instantiate
  allocation->planet->asset = RootWindow(allocation->dis, DefaultScreen(allocation->dis));
  allocation->asset_width  = WW(allocation->dis, allocation->planet->asset);
  allocation->asset_height = WH(allocation->dis, allocation->planet->asset);

    if (!isRoot) {
    allocation->planet->asset =
        XCreateWindow(allocation->dis, DefaultRootWindow(allocation->dis), dimensions[0],
                                                                           dimensions[1],
                                                                           dimensions[2],
                                                                           dimensions[3],
      /*border*/5, /*depth*/24, /*class*/0, /*Visual*/0, /*attr values*/0, (XSetWindowAttributes *)0);

    XSetStandardProperties(allocation->dis, allocation->planet->asset, title, subtitle, 0, 0, 0, 0);
    XSelectInput(allocation->dis, allocation->planet->asset, ExposureMask|ButtonPressMask|KeyPressMask);
    XClearWindow(allocation->dis, allocation->planet->asset);
    XMapRaised(allocation->dis,   allocation->planet->asset);
    }

  allocation->planet->gc = XCreateGC(allocation->dis, allocation->planet->asset, 0, (XGCValues *)0);

  XFlush(allocation->dis);

  allocation->history = (struct event_node *)1; // TODO: make this point to first exposure event

  } else {
  struct user_context context = {1, allocation}; // default is not editing mask // should we hand this back to user?
  (*allocation->root_orbital->instantiate)( &context ); // function should copy or create Pixmap
  }
  // if Drawable is still not set, should do something here


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

char within(int x, int y, struct orbital *child) {
  x -= child->parent_mapping.x;
  y -= child->parent_mapping.y;
  return !( x < 0 || y < 0 || x > child->asset_width || y > child->asset_height );
}

void AlignXY(int x, int y) {
  struct orbital **base = &( head_orbital->suborbital ); // don't grab root
  struct orbital *origin;
  struct orbital *result;
  struct orbital *scan;

  char isWithin;

  while (*base) {
    scan = *base;
    origin = scan;

    while ( scan->next != origin ) {
    scan = scan->next;
    isWithin = within(x, y, scan);
      if ( isWithin && !result ) {
      x -= scan->parent_mapping.x;
      y -= scan->parent_mapping.y;
      result = scan;
      } else if ( isWithin && result ) {
      return;
      }
    }

  isWithin = within(x, y, origin);

    if ( isWithin && !result ) {
    x -= origin->parent_mapping.x;
    y -= origin->parent_mapping.y;
    result = scan;
    } else if ( isWithin && result ) {
    return;
    }

    if (result) {
    *base = result;
    }

  base = &( (*base)->suborbital );
  }
}

void AscendSelect() {
struct orbital *climb = head_orbital;
  while (climb) {
    if (climb->select) {
    ( *(climb->select) )(climb);
    }
  climb = climb->suborbital;
  }
}

void AscendUnselect() {
struct orbital *climb = head_orbital;
  while (climb) {
    if (climb->unselect) {
    ( *(climb->unselect) )(climb);
    }
  climb = climb->suborbital;
  }
}

void *actuateTier(enum funcType type, void *handle) {
struct user_context *cxt = (struct user_context *)handle;
struct orbital *scan = cxt->current_orbital;
struct orbital *original = scan;
struct user_context *ret = malloc(sizeof(struct user_context));
ret->isEditingBuffer = 1;

void (*temporary)(void *);

  while (scan->next != original) {
  scan = scan->next;
  temporary = (type == Glorb) ? scan->select : scan->unselect;
    if (temporary) {
    ret->current_orbital = scan;
    ( *(temporary) )(scan);
    }
  }

return ret;

}

void *TierSelect(void *handle) { // except for self
return actuateTier(Glorb, handle);
}

void *TierUnselect(void *handle) { // excepting self
return actuateTier(UnGlorb, handle);
}

void TradeRoutine(enum funcType type, void *h1, void *h2) {
struct user_context *cxt1 = (struct user_context *)h1;
struct user_context *cxt2 = (struct user_context *)h2;
void (* temporary)(void *);

  switch (type) {
    case Glorb:
    temporary = cxt2->current_orbital->select;
    cxt2->current_orbital->select = cxt1->current_orbital->select;
    cxt1->current_orbital->select = temporary;
    break;
    case UnGlorb:
    temporary = cxt2->current_orbital->unselect;
    cxt2->current_orbital->unselect = cxt1->current_orbital->unselect;
    cxt1->current_orbital->select = temporary;
    break;
    case InstaGlorb:
    temporary = cxt2->current_orbital->instantiate;
    cxt2->current_orbital->instantiate = cxt1->current_orbital->instantiate;
    cxt1->current_orbital->select = temporary;
    break;
  }

}

void *GlorbHandle(int tier) {
struct orbital *climb = head_orbital;
struct user_context *ret = malloc(sizeof(struct user_context));
  while (tier--) {
  climb = climb->suborbital;
  }
ret->isEditingBuffer = 1;
ret->current_orbital = climb;
return ret;
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

void makeMask(void *handle) {
struct user_context *cxt = (struct user_context *)handle;
XGCValues attributes;
attributes.function = GXor;
attributes.background = 0;
attributes.foreground = 1;
cxt->current_orbital->mask = malloc(sizeof(struct gcontext));

cxt->current_orbital->mask->asset =
  XCreatePixmap(
   cxt->current_orbital->dis,
   cxt->current_orbital->root_orbital->planet->asset, // TODO: should only do this if root_orbital is Window 
   cxt->current_orbital->asset_width, cxt->current_orbital->asset_height,
   2
  );

cxt->current_orbital->mask->gc = 
  XCreateGC(cxt->current_orbital->dis, cxt->current_orbital->mask->asset, GCFunction | GCBackground | GCForeground, (XGCValues *)0);

}

void makeBuffer(void *handle, int parentX, int parentY, int width, int height) {
struct user_context *cxt = (struct user_context *)handle;
cxt->current_orbital->planet->asset =
  XCreatePixmap(
    cxt->current_orbital->dis,
    cxt->current_orbital->root_orbital->planet->asset,
    width, height,
    DDepth(cxt->current_orbital->dis, cxt->current_orbital->root_orbital->planet->asset)
  );

  cxt->current_orbital->planet->gc = 
    XCreateGC(cxt->current_orbital->dis, cxt->current_orbital->planet->asset, /*GC values*/0, (XGCValues *)0);

  cxt->current_orbital->asset_width  = width;
  cxt->current_orbital->asset_height = height;
  cxt->current_orbital->parent_mapping.x = parentX; // for XCopyArea calls
  cxt->current_orbital->parent_mapping.y = parentY;

}

// These two are useful for copying window information without recreating window
void referenceSiblingBuffer(void *handle) { // TODO: All buffer related work
struct user_context *cxt = (struct user_context *)handle;

}

void referenceSiblingMask(void *handle) {
struct user_context *cxt = (struct user_context *)handle;

}
// These two are useful for copying window information without recreating window

void copySiblingBuffer(void *handle) {
struct user_context *cxt = (struct user_context *)handle;

}

void copySiblingMask(void *handle) {
struct user_context *cxt = (struct user_context *)handle;

}

void EditBuffer(void *handle) {
struct user_context *cxt = (struct user_context *)handle;
cxt->isEditingBuffer = 1;
}

void EditMask(void *handle) {
struct user_context *cxt = (struct user_context *)handle;
cxt->isEditingBuffer = cxt->current_orbital->mask ? 0 : 1;
}

void RegionFill(void *handle, int x, int y, int height, int width, long long rgb) { 
struct user_context *cxt = (struct user_context *)handle;
GC gc = cxt->isEditingBuffer ? cxt->current_orbital->planet->gc
                             : cxt->current_orbital->mask->gc;

Drawable asset = cxt->isEditingBuffer ? cxt->current_orbital->planet->asset
                                      : cxt->current_orbital->mask->asset;

XSetForeground(cxt->current_orbital->dis, gc, rgb);
XFillRectangle(cxt->current_orbital->dis, asset, gc, x, y, width, height);
}

void RegionScarf(void *handle, int x, int y, int height, int width, long long rgb) {
struct user_context *cxt = (struct user_context *)handle;
GC gc = cxt->isEditingBuffer ? cxt->current_orbital->planet->gc
                             : cxt->current_orbital->mask->gc;

Drawable asset = cxt->isEditingBuffer ? cxt->current_orbital->planet->asset
                                      : cxt->current_orbital->mask->asset;

XSetForeground(cxt->current_orbital->dis, gc, rgb);
XDrawRectangle(cxt->current_orbital->dis, asset, gc, x, y, width, height);
}

void DrawPixel(void *handle, int x, int y) { // TODO: 
struct user_context *cxt = (struct user_context *)handle;

}

void RegionFromBits(void *handle, int scale, char *bits, int width, int height) { // TODO: 
struct user_context *cxt = (struct user_context *)handle;

}

void RegionFromRotatedBits(void *handle, int scale, char *bits, int width, int height) {
struct user_context *cxt = (struct user_context *)handle;

}

void RegisterFunc(enum funcType type, void (*template)(void *)) {
  switch (type) {
    case Glorb:
    head_orbital->select = template;
    break;
    case UnGlorb:
    head_orbital->unselect = template;
    break;
    case InstaGlorb:
    head_orbital->instantiate = template;
    break;
  }
}

void RegisterFuncID(enum funcType type, void (*template)(void *), int rootID) {
int prevID = head_orbital->identity;
AlignID((int [1]){rootID}, 1);
RegisterFunc(type, template);
AlignID((int [1]){prevID}, 1);
}

void Default(void *handle) { // For now, assumes root orbital is Window
struct user_context *cxt = (struct user_context *)handle;
makeBuffer(handle, 0, 0, WW(cxt->current_orbital->dis, cxt->current_orbital->root_orbital->planet->asset),
                         WH(cxt->current_orbital->dis, cxt->current_orbital->root_orbital->planet->asset));
}


void Graft(void *h1, void *h2) {
struct user_context *cxt1 = (struct user_context *)h1;
struct user_context *cxt2 = (struct user_context *)h2;
struct orbital *temporary;

temporary = cxt1->current_orbital->suborbital;
cxt1->current_orbital->suborbital = cxt2->current_orbital->suborbital;
cxt2->current_orbital->suborbital = temporary;

}

void effectuate(struct orbital *p, struct orbital *c) {
char parentNeedsMask = p->mask ? 0 : 1;
char childNeedsMask  = c->mask ? 0 : 1;

  if (!childNeedsMask && parentNeedsMask) {
  XGCValues attributes;
  attributes.function = GXor;
  attributes.foreground = 1;
  attributes.background = 0;
  p->mask = malloc(sizeof(struct gcontext));
  p->mask->asset = XCreatePixmap(p->dis, p->root_orbital->planet->asset, p->asset_width, p->asset_height, 2);
  p->mask->gc = XCreateGC(p->dis, p->mask->asset, GCFunction | GCBackground | GCForeground, &attributes);
  // the above should or the two together
  }

  if (!childNeedsMask) {
  XCopyArea(p->dis, c->mask->asset, p->mask->asset, p->mask->gc, 0, 0,
            c->asset_width, c->asset_height, c->parent_mapping.x, c->parent_mapping.y);
  XSetClipMask(p->dis, p->planet->gc, p->mask->asset);
  }


  XCopyArea(p->dis, c->planet->asset, p->planet->asset, p->planet->gc, 0, 0,
            c->asset_width, c->asset_height, c->parent_mapping.x, c->parent_mapping.y);
  // the above draws child into parent
  XFlush(p->dis);

  if (!childNeedsMask) { // unset mask
  XSetClipMask(p->dis, p->planet->gc, (Drawable)0);
  }

  if (!childNeedsMask && parentNeedsMask) {
  // deallocate
  p->mask = 0;
  }
}

void recurse(struct orbital *parent) {
  struct orbital *child = parent->suborbital;
  struct orbital *original = parent->suborbital;

  while (child->next != original) {
    child = child->next;
    if (child->suborbital) {
      recurse(child);
    }
    effectuate(parent, child);
  }
  if (original->suborbital) {
    recurse(original);
  }
  effectuate(parent, original);
}

void Draw() {
  if (head_orbital) {
    if (head_orbital->suborbital) {
    recurse(head_orbital);
    }
  }
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


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
char isEditingBuffer;
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
  allocation->isEditingBuffer = 1;

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

  allocation->history = malloc(sizeof(struct event_node)); // TODO: make this point to first exposure event

  } else {
  (*allocation->root_orbital->instantiate)( allocation ); // function should copy or create Pixmap
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

void RemapTier(void *handle, int *x, int *y, int n, int tier) {
struct orbital *base = head_orbital;
struct orbital *scan;

  if (!tier) {
  // maybe perform on all tiers?
  }

  while (--tier && base) {
  base = base->suborbital;
  }

  if (!base) {
  return;
  }

  scan = base;
  for (int i = 0; i < n; i++) {
  scan->parent_mapping.x = x[i];
  scan->parent_mapping.y = y[i];
  scan = scan->next;
  }

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

  int result_x, result_y;

  char isWithin;

  while (*base) {
    result = 0;
    scan = *base;
    origin = scan;

    while ( scan->next != origin ) {
    scan = scan->next;
    isWithin = within(x, y, scan);
      if ( isWithin && !result ) {
      result_x = x - scan->parent_mapping.x;
      result_y = y - scan->parent_mapping.y;
      result = scan;
      } else if ( isWithin && result ) {
      return;
      }
    }

  isWithin = within(x, y, origin);

    if ( isWithin && !result ) {
    result_x = x - origin->parent_mapping.x;
    result_y = y - origin->parent_mapping.y;
    result = scan;
    } else if ( isWithin && result ) {
    return;
    }

    if (result) {
    *base = result;
    }

  x = result_x;
  y = result_y;

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
struct orbital *cxt = (struct orbital *)handle;
struct orbital *scan = cxt;
struct orbital *original = scan;
struct orbital *ret = 0;

void (*temporary)(void *);

  while (scan->next != original) {
  scan = scan->next;
  temporary = (type == Glorb) ? scan->select : scan->unselect;
    if (temporary) {
    ret = scan;
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
  if (h1 == 0 || h2 == 0) {
  return;
  } 

struct orbital *cxt1 = (struct orbital *)h1;
struct orbital *cxt2 = (struct orbital *)h2;
void (* temporary)(void *);

  switch (type) {
    case Glorb:
    temporary = cxt2->select;
    cxt2->select = cxt1->select;
    cxt1->select = temporary;
    break;
    case UnGlorb:
    temporary = cxt2->unselect;
    cxt2->unselect = cxt1->unselect;
    cxt1->select = temporary;
    break;
    case InstaGlorb:
    temporary = cxt2->instantiate;
    cxt2->instantiate = cxt1->instantiate;
    cxt1->select = temporary;
    break;
  }

}

void *GlorbHandle(int tier) { // tier is NOT zero-index
                              // both tier = 0 and tier = 1, return head_orbit
struct orbital *climb = head_orbital;
  if (!tier) {
  return climb;
  }

  while (--tier && climb) {
  climb = climb->suborbital;
  }
return climb;
}

int bitfill(int depth) {
return (1<<depth) - 1;
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
struct orbital *cxt = (struct orbital *)handle;
XGCValues attributes;
attributes.function = GXor;
attributes.background = 0;
attributes.foreground = 1;
cxt->mask = malloc(sizeof(struct gcontext));

cxt->mask->asset =
  XCreatePixmap(
   cxt->dis,
   cxt->root_orbital->planet->asset, // TODO: should only do this if root_orbital is Window 
   cxt->asset_width, cxt->asset_height,
   2
  );

cxt->mask->gc = 
  XCreateGC(cxt->dis, cxt->mask->asset, GCFunction | GCBackground | GCForeground, (XGCValues *)0);

}

void makeBuffer(void *handle, int parentX, int parentY, int width, int height) {
struct orbital *cxt = (struct orbital *)handle;
cxt->planet->asset =
  XCreatePixmap(
    cxt->dis,
    cxt->root_orbital->planet->asset,
    width, height,
    DDepth(cxt->dis, cxt->root_orbital->planet->asset)
  );

  cxt->planet->gc = 
    XCreateGC(cxt->dis, cxt->planet->asset, /*GC values*/0, (XGCValues *)0);

  cxt->asset_width  = width;
  cxt->asset_height = height;


}

// These two are useful for copying window information without recreating window
void referenceSiblingBuffer(void *handle) {
struct orbital *cxt = (struct orbital *)handle;
cxt->parent_mapping.x = cxt->next->parent_mapping.x;
cxt->parent_mapping.y = cxt->next->parent_mapping.y;
cxt->asset_height = cxt->next->asset_height;
cxt->asset_width  = cxt->next->asset_width;
cxt->planet = cxt->next->planet;
}

void referenceSiblingMask(void *handle) {
struct orbital *cxt = (struct orbital *)handle; // this causes a potential freeing nightmare
                                                // but let's not worry about that right now 
cxt->mask = cxt->next->mask;
}
// These two are useful for copying window information without recreating window

void copySiblingBuffer(void *handle) {
struct orbital *cxt = (struct orbital *)handle;
makeBuffer(handle, cxt->parent_mapping.x, cxt->parent_mapping.y, cxt->asset_width, cxt->asset_height);
XCopyArea(cxt->root_orbital->dis, cxt->planet->asset, cxt->next->planet->asset,
          cxt->planet->gc, 0, 0, cxt->asset_width, cxt->asset_height, 0, 0);
}

void copySiblingMask(void *handle) {
struct orbital *cxt = (struct orbital *)handle;
makeMask(handle);
XCopyArea(cxt->root_orbital->dis, cxt->mask->asset, cxt->next->mask->asset,
          cxt->mask->gc, 0, 0, cxt->asset_width, cxt->asset_height, 0, 0);
}

void EditBuffer(void *handle) {
struct orbital *cxt = (struct orbital *)handle;
cxt->isEditingBuffer = 1;
}

void EditMask(void *handle) {
struct orbital *cxt = (struct orbital *)handle;
cxt->isEditingBuffer = cxt->mask ? 0 : 1;
}

void InvertMask(void *handle) {
struct orbital *cxt = (struct orbital *)handle;
  XGCValues attributes;
  attributes.function = GXxor;
  Pixmap inv = XCreatePixmap(cxt->dis, cxt->root_orbital->planet->asset,
                             cxt->asset_width, cxt->asset_height, 2);

  GC gc = XCreateGC(cxt->dis, inv, GCFunction, &attributes);

  XCopyArea(cxt->dis, inv, cxt->mask->asset, gc, 0, 0, cxt->asset_width, cxt->asset_height, 0, 0);

  // deallocate pixmap, gc, etc
}

void RegionFill(void *handle, int x, int y, int height, int width, long long rgb) { 
struct orbital *cxt = (struct orbital *)handle;
GC gc = cxt->isEditingBuffer ? cxt->planet->gc
                             : cxt->mask->gc;

Drawable asset = cxt->isEditingBuffer ? cxt->planet->asset
                                      : cxt->mask->asset;

XSetForeground(cxt->dis, gc, rgb);
XFillRectangle(cxt->dis, asset, gc, x, y, width, height);
}

void RegionScarf(void *handle, int x, int y, int height, int width, long long rgb) {
struct orbital *cxt = (struct orbital *)handle;
GC gc = cxt->isEditingBuffer ? cxt->planet->gc
                             : cxt->mask->gc;

Drawable asset = cxt->isEditingBuffer ? cxt->planet->asset
                                      : cxt->mask->asset;

XSetForeground(cxt->dis, gc, rgb);
XDrawRectangle(cxt->dis, asset, gc, x, y, width, height);
}

void DrawPixel(void *handle, int x, int y, long long rgb) { // TODO: 
struct orbital *cxt = (struct orbital *)handle;
GC gc = cxt->isEditingBuffer ? cxt->planet->gc
                             : cxt->mask->gc;

Drawable asset = cxt->isEditingBuffer ? cxt->planet->asset
                                      : cxt->mask->asset;

XSetForeground(cxt->dis, gc, rgb);
XDrawPoint(cxt->dis, asset, gc, x, y);
}

void MaskFromBits(void *handle, int scale, char *bits, int width, int height) { // TODO:
struct orbital *cxt = (struct orbital *)handle;
  if (scale == 1) {
  cxt->mask->asset = XCreatePixmapFromBitmapData(cxt->dis, cxt->root_orbital->planet->asset, bits, width, height, 1, 0, 2);
  } else {
  XGCValues attributes;
  attributes.foreground = 1;
  attributes.background = 0;
  cxt->mask->gc = XCreateGC(cxt->dis, 0, GCForeground | GCBackground, &attributes);
  cxt->mask->asset = XCreatePixmap(cxt->dis, cxt->root_orbital->planet->asset, width, height, 2);
    for (int i = 0; i < height; i++) {
      for (int j = 0; j < width; j++) {
        if ( !( ( bits[i * (width / 8) + j / 8]>>(j&7) )&1 ) ) {
        XDrawRectangle(cxt->dis, cxt->mask->asset, cxt->mask->gc, i, j, scale, scale);
        }
      }
    }
  }
}

void BitDraw(void *handle, int scale, char rotation, char *bits, int x, int y,
             int width, int height, long long fore, long long back) {

struct orbital *cxt = (struct orbital *)handle;

Pixmap asset = cxt->isEditingBuffer ? cxt->planet->asset : cxt->mask->asset;
GC gc = cxt->isEditingBuffer ? cxt->planet->gc : cxt->mask->gc;

int i_offx, i_offy, offx, offy, maxx, maxy;
char condx = (rotation&1)  ^ (rotation>>1);
char condy = (rotation&1) == (rotation>>1);

maxx = condx ? height : width;
maxy = condy ? height : width;

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
    i_offx = condx ? i : j;
    i_offy = condy ? i : j;
    offx = rotation==1 ? maxx - i_offx - 1 : i_offx;
    offy = rotation==3 ? maxy - i_offy - 1 : i_offy;
    XSetForeground(cxt->dis, gc, ( bits[i * (width / 8) + j / 8]>>(j&7) )&1 ? back : fore);
    XDrawRectangle(cxt->dis, asset, gc, x + scale * offx, y + scale * offy, scale, scale);
    }
  }

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
struct orbital *cxt = (struct orbital *)handle;
makeBuffer(handle, 0, 0, WW(cxt->dis, cxt->root_orbital->planet->asset),
                         WH(cxt->dis, cxt->root_orbital->planet->asset));
}


void Graft(void *h1, void *h2) {
struct orbital *cxt1 = (struct orbital *)h1;
struct orbital *cxt2 = (struct orbital *)h2;
struct orbital *temporary;

temporary = cxt1->suborbital;
cxt1->suborbital = cxt2->suborbital;
cxt2->suborbital = temporary;

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

void Eve(struct cache *cc){
// head_orbital->history = ;
// allocated line 155

cc->t =   0;
cc->b =   0;
cc->txt = 0;
cc->x =   0;
cc->y =   0;

XWindowEvent(head_orbital->dis, head_orbital->planet->asset, ExposureMask|/*causes event on flush*/ButtonPressMask|KeyPressMask, &(head_orbital->history->event));

  while (1) {
        if (head_orbital->history->event.type == Expose) {
        cc->t = 3;
        return;
        } else if (head_orbital->history->event.type == ButtonPress) {
        cc->t = 2;
        cc->b = head_orbital->history->event.xbutton.button;
        cc->x = head_orbital->history->event.xbutton.x;
        cc->y = head_orbital->history->event.xbutton.y;
        return;
        } else if (head_orbital->history->event.type == KeyPress) {
        XLookupString(&(head_orbital->history->event.xkey), malloc(1), 1/*buffersize*/, &(head_orbital->history->key), 0);
        cc->t = 1;
        cc->txt = head_orbital->history->key;
        return;
        }
  XWindowEvent(head_orbital->dis, head_orbital->planet->asset, ExposureMask|ButtonPressMask|KeyPressMask, &(head_orbital->history->event));
  }

}

Bool invisible(Display *display, XEvent *event, XPointer arg) {
  if (event->xany.window == head_orbital->planet->asset && (event->type == Expose || event->type == ButtonPress || event->type == KeyPress) ) {
  return 1;
  }
return 0;
}

char EvePending() {
char ret = XCheckIfEvent(head_orbital->dis, &(head_orbital->history->event), invisible, 0);
XPutBackEvent(head_orbital->dis, &(head_orbital->history->event));
return ret;
}

// add:
// ExposeEvent(&c)
// MouseUpEvent(&c)
// KeyUpEvent(&c)
// MouseDownEvent(&c)
// while maintaining our own queues of user-set length

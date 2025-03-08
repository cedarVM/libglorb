#include "ssw.h"
#include "ranks.xbm"
#include <stdio.h>

void ace(void *handle) {
makeBuffer(handle, 0, 0, 32, 48);
}

void buffer(void *handle) {
makeBuffer(handle, 0, 0, 1000, 1000);
RegionFill(handle, 0, 0, 1000, 1000, 0x000000);
}

void unselect(void *handle) {
  RegisterFunc(UnGlorb, 0);
  TradeRoutine(UnGlorb, GlorbHandle(0), handle);
  BitDraw(handle, 1, 0, rank1_bits, 0, 0, 32, 48, 0xFFFFFF, 0x000000);
}

void select(void *handle) {
void *ret = TierUnselect(handle);

  if (!ret) {
  RegisterFunc(UnGlorb, unselect);
  TradeRoutine(UnGlorb, GlorbHandle(0), handle);
  }
  BitDraw(handle, 1, 0, rank0_bits, 0, 0, 32, 48, 0xFFFFFF, 0x000000);

}

int main () {
struct cache event;

XI("", "", (int [4]){0, 0, 500, 500}, 0, 1, 0, 2);
RegisterFunc(InstaGlorb, buffer);
XI("", "", (int [4]){0, 0, 500, 500}, 0, 0, 1, 1);

RegisterFunc(InstaGlorb, ace);
RegisterFunc(Glorb, select);

  for (int i = 0; i < 13; i++) {
  XI("", "", (int [4]){i * 32, 0, -1, -1}, 0, 0, 2, i);
  }

  void *handle;

  Align((int [3]){0, 0, 1}, 3);
  handle = GlorbHandle(3);
  BitDraw(handle, 1, 0, rank0_bits, 0, 0, 32, 48, 0xFFFFFF, 0x000000);
  Align((int [3]){0, 0, 1}, 3);
  handle = GlorbHandle(3);
  BitDraw(handle, 1, 0, rank1_bits, 0, 0, 32, 48, 0xFFFFFF, 0x000000);
  Align((int [3]){0, 0, 1}, 3);
  handle = GlorbHandle(3);
  BitDraw(handle, 1, 0, rank2_bits, 0, 0, 32, 48, 0xFFFFFF, 0x000000);
  Align((int [3]){0, 0, 1}, 3);
  handle = GlorbHandle(3);
  BitDraw(handle, 1, 0, rank3_bits, 0, 0, 32, 48, 0xFFFFFF, 0x000000);
  Align((int [3]){0, 0, 1}, 3);
  handle = GlorbHandle(3);
  BitDraw(handle, 1, 0, rank4_bits, 0, 0, 32, 48, 0xFFFFFF, 0x000000);
  Align((int [3]){0, 0, 1}, 3);
  handle = GlorbHandle(3);
  BitDraw(handle, 1, 0, rank5_bits, 0, 0, 32, 48, 0xFFFFFF, 0x000000);
  Align((int [3]){0, 0, 1}, 3);
  handle = GlorbHandle(3);
  BitDraw(handle, 1, 0, rank6_bits, 0, 0, 32, 48, 0xFFFFFF, 0x000000);
  Align((int [3]){0, 0, 1}, 3);
  handle = GlorbHandle(3);
  BitDraw(handle, 1, 0, rank7_bits, 0, 0, 32, 48, 0xFFFFFF, 0x000000);
  Align((int [3]){0, 0, 1}, 3);
  handle = GlorbHandle(3);
  BitDraw(handle, 1, 0, rank8_bits, 0, 0, 32, 48, 0xFFFFFF, 0x000000);
  Align((int [3]){0, 0, 1}, 3);
  handle = GlorbHandle(3);
  BitDraw(handle, 1, 0, rank9_bits, 0, 0, 32, 48, 0xFFFFFF, 0x000000);
  Align((int [3]){0, 0, 1}, 3);
  handle = GlorbHandle(3);
  BitDraw(handle, 1, 0, rank10_bits, 0, 0, 32, 48, 0xFFFFFF, 0x000000);
  Align((int [3]){0, 0, 1}, 3);
  handle = GlorbHandle(3);
  BitDraw(handle, 1, 0, rank11_bits, 0, 0, 32, 48, 0xFFFFFF, 0x000000);
  Align((int [3]){0, 0, 1}, 3);
  handle = GlorbHandle(3);
  BitDraw(handle, 1, 0, rank12_bits, 0, 0, 32, 48, 0xFFFFFF, 0x000000);



XI("", "", (int [4]){0, 0, 500, 500}, 0, 1, 0, 1);
RegisterFunc(InstaGlorb, buffer);
XI("", "", (int [4]){0, 0, 500, 500}, 0, 0, 1, 1);

RegisterFunc(InstaGlorb, ace);
RegisterFunc(Glorb, select);

  for (int i = 0; i < 13; i++) {
  XI("", "", (int [4]){i * 32, 0, -1, -1}, 0, 0, 2, i);
  }

  Align((int [3]){0, 0, 1}, 3);
  handle = GlorbHandle(3);
  BitDraw(handle, 1, 0, rank0_bits, 0, 0, 32, 48, 0xFFFFFF, 0x000000);
  Align((int [3]){0, 0, 1}, 3);
  handle = GlorbHandle(3);
  BitDraw(handle, 1, 0, rank1_bits, 0, 0, 32, 48, 0xFFFFFF, 0x000000);
  Align((int [3]){0, 0, 1}, 3);
  handle = GlorbHandle(3);
  BitDraw(handle, 1, 0, rank2_bits, 0, 0, 32, 48, 0xFFFFFF, 0x000000);
  Align((int [3]){0, 0, 1}, 3);
  handle = GlorbHandle(3);
  BitDraw(handle, 1, 0, rank3_bits, 0, 0, 32, 48, 0xFFFFFF, 0x000000);
  Align((int [3]){0, 0, 1}, 3);
  handle = GlorbHandle(3);
  BitDraw(handle, 1, 0, rank4_bits, 0, 0, 32, 48, 0xFFFFFF, 0x000000);
  Align((int [3]){0, 0, 1}, 3);
  handle = GlorbHandle(3);
  BitDraw(handle, 1, 0, rank5_bits, 0, 0, 32, 48, 0xFFFFFF, 0x000000);
  Align((int [3]){0, 0, 1}, 3);
  handle = GlorbHandle(3);
  BitDraw(handle, 1, 0, rank6_bits, 0, 0, 32, 48, 0xFFFFFF, 0x000000);
  Align((int [3]){0, 0, 1}, 3);
  handle = GlorbHandle(3);
  BitDraw(handle, 1, 0, rank7_bits, 0, 0, 32, 48, 0xFFFFFF, 0x000000);
  Align((int [3]){0, 0, 1}, 3);
  handle = GlorbHandle(3);
  BitDraw(handle, 1, 0, rank8_bits, 0, 0, 32, 48, 0xFFFFFF, 0x000000);
  Align((int [3]){0, 0, 1}, 3);
  handle = GlorbHandle(3);
  BitDraw(handle, 1, 0, rank9_bits, 0, 0, 32, 48, 0xFFFFFF, 0x000000);
  Align((int [3]){0, 0, 1}, 3);
  handle = GlorbHandle(3);
  BitDraw(handle, 1, 0, rank10_bits, 0, 0, 32, 48, 0xFFFFFF, 0x000000);
  Align((int [3]){0, 0, 1}, 3);
  handle = GlorbHandle(3);
  BitDraw(handle, 1, 0, rank11_bits, 0, 0, 32, 48, 0xFFFFFF, 0x000000);
  Align((int [3]){0, 0, 1}, 3);
  handle = GlorbHandle(3);
  BitDraw(handle, 1, 0, rank12_bits, 0, 0, 32, 48, 0xFFFFFF, 0x000000);


event.txt = ' ';
while (event.txt != 'q') {
Draw();
  if (EvePending()) { // this generates an exposure event
    Eve(&event);
    Eve(&event);
    printf("%d\n", event.t);

    if (event.t == 2) {
    AlignXY(event.x, event.y);
    AscendSelect();
    } else if (event.t == 1 && event.txt == ' ') {
    Align((int [1]){1}, 1);
    }


  }

}

}

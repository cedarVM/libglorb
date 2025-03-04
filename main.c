#include "ssw.h"
#include <stdio.h>

void template(void *handle) {
makeBuffer(handle, 10, 10, 0, 0, 0);
RegionFill(handle, 0, 0, 10, 10, 0xFF0000);
}

int main () {
XI("", "", (int [4]){0, 0, 10, 10}, 0, 1, 0, 1);
XI("", "", (int [4]){0, 0, 10, 10}, 0, 1, 0, 2);
XI("", "", (int [4]){0, 0, 10, 10}, 0, 1, 0, 3);
XI("", "", (int [4]){0, 0, 10, 10}, 0, 1, 0, 4);
XI("", "", (int [4]){0, 0, 0, 0}, 0, 0, 1, 72);
XI("", "", (int [4]){0, 0, 0, 0}, 0, 0, 1, 73);
XI("", "", (int [4]){0, 0, 0, 0}, 0, 0, 2, 74);
XI("", "", (int [4]){0, 0, 0, 0}, 0, 0, 3, 75);
XI("", "", (int [4]){0, 0, 10, 10}, 0, 1, 0, 5);
AlignID((int [4]){4, 73, 74, 75}, 4);
RegisterTemplate(template, 4);
XI("", "", (int [4]){0, 0, 0, 0}, 0, 0, 4, 89);
AlignID((int [5]){4, 73, 74, 75, 89}, 5);
Draw();
fgetc(stdin);
Draw();
fgetc(stdin);
}

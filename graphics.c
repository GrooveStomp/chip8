#include <string.h> // memset

struct graphics {
        unsigned int displayWidth;
        unsigned int displayHeight;

};

typedef void *(*allocator)(size_t);
typedef void (*deallocator)(void *);

static allocator ALLOCATOR = malloc;
static deallocator DEALLOCATOR = free;

void SystemMemControl(allocator Alloc, deallocator Dealloc) {
        ALLOCATOR = Alloc;
        DEALLOCATOR = Dealloc;
}

struct graphics *GraphicsInit(unsigned int displayScale) {
        struct graphics *g = (struct graphics *)malloc(sizeof(struct graphics));

        memset(g, 0, sizeof(struct graphics));

        g->displayWidth = 64 * displayScale;
        g->displayHeight = 32 * displayScale;
}

void Embiggen(unsigned int *buf, unsigned int x, unsigned int y, unsigned int pixel) {
        for (int sy = y; sy < y + DISPLAY_SCALE; sy++) {
                for (int sx = x; sx < x + DISPLAY_SCALE; sx++) {
                        buf[sy * DISPLAY_WIDTH + sx] = pixel;
                }
        }
}

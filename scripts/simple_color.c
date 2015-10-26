#include <fcntl.h>
#include <stdio.h>

int main(int argc, char** argv) {
    int fd = open("gradient.h", O_RDWR | O_CREAT);
    dprintf(fd, "int gradient[GRANDE_CHUNK_SIZE] = {\n");
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 16; k++) {
                dprintf(fd, "\t%d,\n", k);
            }
        }
    }
    dprintf(fd, "};\n");
    dprintf(fd, "int gradient_palette[PALETTE_SIZE] = {\n");
    dprintf(fd, "\t0x00FF0000,\n");
    dprintf(fd, "\t0x00CC0000,\n");
    dprintf(fd, "\t0x00990000,\n");
    dprintf(fd, "\t0x00660000,\n");
    dprintf(fd, "\t0x00330000,\n");
    dprintf(fd, "\t0x0000FF00,\n");
    dprintf(fd, "\t0x0000CC00,\n");
    dprintf(fd, "\t0x00009900,\n");
    dprintf(fd, "\t0x00006600,\n");
    dprintf(fd, "\t0x00003300,\n");
    dprintf(fd, "\t0x000000FF,\n");
    dprintf(fd, "\t0x000000CC,\n");
    dprintf(fd, "\t0x00000099,\n");
    dprintf(fd, "\t0x00000066,\n");
    dprintf(fd, "\t0x00000033,\n");
    dprintf(fd, "};\n");
}


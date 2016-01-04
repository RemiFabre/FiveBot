#include <fcntl.h>
#include <cstdlib>
#include <iostream>

int PHASE[2][2] = { { 0, 1 }, { 3, 2 } };

int main() {
    fd_set fds;

    int fd_a = open("/sys/class/gpio/gpio17/value", O_RDONLY),
        fd_b = open("/sys/class/gpio/gpio18/value", O_RDONLY);
    if (fd_a < 0 or fd_b < 0)
        abort();

    int last_phase = -1,
        position = 0;
    float loops = 0,
        errors = 0;

    while (loops < 50000) { // XXX
        ++loops;

        FD_ZERO(&fds);
        FD_SET(fd_a, &fds);
        FD_SET(fd_b, &fds);
        if (select(fd_b + 1, NULL, NULL, &fds, NULL) <= 0)
            abort();

        char a, b;
        lseek(fd_a, 0, SEEK_SET);
        lseek(fd_b, 0, SEEK_SET);
        if (read(fd_a, &a, 1) < 1 or read(fd_b, &b, 1) < 1)
            abort();
        a -= '0';
        b -= '0';

        const int phase = PHASE[a][b];
        if (phase != last_phase) {
            int dir = 0;
            if (phase == (last_phase + 1) % 4)
                dir = 1;
            else if (phase == (last_phase + 3) % 4)
                dir = -1;
            else if (phase != last_phase && last_phase != -1)
                //std::cerr << "error: phase " << phase << " came after phase " << last_phase << std::endl;
                ++errors;
            position += dir;
            std::cout << position << '\r';
            last_phase = phase;
        }
    }
    std::cout << "errors: " << 100 * errors / loops << "% (" << errors << "/" << loops << ")" << std::endl;
}

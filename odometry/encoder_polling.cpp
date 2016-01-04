#include <cassert>
#include <iostream>
#include <fstream>

int PHASE[2][2] = { { 0, 1 }, { 3, 2 } };

int main() {
    std::fstream gpio_a("/sys/class/gpio/gpio17/value"),
        gpio_b("/sys/class/gpio/gpio18/value");
    int last_phase = -1,
        position = 0;
    while (true) {
        gpio_a.seekg(0);
        gpio_b.seekg(0);
        int a = gpio_a.get(),
            b = gpio_b.get();
        assert(a != -1 and b != -1);
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
                std::cerr << "error: phase " << phase << " came after phase " << last_phase << std::endl;
            position += dir;
            std::cout << position << std::endl;
            last_phase = phase;
        }
    }
}

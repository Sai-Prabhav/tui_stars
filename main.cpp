#include <chrono>
#include <csignal>
#include <iostream>
#include <ncurses.h>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

struct Color {
    int red;
    int green;
    int blue;
};

void sleep(int time) {
    std::this_thread::sleep_for(
        std::chrono::milliseconds(time));
}

void printch(char ch, int x, int y, int pair) {

    attron(COLOR_PAIR(pair));
    mvaddch(y, x, ch);
    attroff(COLOR_PAIR(pair));
}

int add_color(Color color) {
    static int color_index =
        7; // 0-7 index are standard colors
    init_color(++color_index, color.red, color.green,
               color.blue);
    return color_index;
}

int add_pair(int fg, int bg) {
    static int pair_index = 0;
    init_pair(++pair_index, fg, bg);
    return pair_index;
}

namespace paletteColor {

static std::vector<int> red = {};
static std::vector<int> green = {};
static std::vector<int> blue = {};
void init() {
    for (int i = 0; i < 10; i++) {
        red.push_back(add_color(
            {1000, 1000 / 10 * i, 1000 / 10 * i}));
        blue.push_back(add_color(
            {1000 / 10 * i, 1000, 1000 / 10 * i}));
        green.push_back(add_color(
            {1000 / 10 * i, 1000 / 10 * i, 1000}));
    }
}
} // namespace paletteColor

namespace palettePair {
int white;
std::vector<int> red(10);
std::vector<int> blue(10);
std::vector<int> green(10);
void init() {
    int colwhite = add_color({1000, 1000, 1000});
    white = add_pair(colwhite, colwhite);
    for (int i = 0; i < 10; ++i) {
        palettePair::red[i] =
            (add_pair(paletteColor::red[i],
                      paletteColor::red[i]));
        palettePair::green[i] =
            (add_pair(paletteColor::green[i],
                      paletteColor::green[i]));
        palettePair::blue[i] =
            (add_pair(paletteColor::blue[i],
                      paletteColor::blue[i]));
    }
};
} // namespace palettePair

auto getColor(int x) {

    switch (x) {
    case 1:
        return palettePair::red;
    case 2:
        return palettePair::blue;
    case 3:
        return palettePair::green;
    default:
        throw std::runtime_error("Invalid color code" +
                                 std::to_string(x));
    }
}

void drawStar(std::vector<int> &point,
              std::mt19937 &gen, int width,
              int height) {
    if (point[3] > 0) {
        printch(' ', point[0], point[1],
                getColor(point[2])[10 - (point[3]--)]);
        if (gen() % 3 == 0) {
            point[3]--;
        }
    } else {

        printch(' ', point[0], point[1],
                palettePair::white);
        point[0] = static_cast<int>(gen() % width);
        point[1] = static_cast<int>(gen() % height);
        point[3] = 9;
        point[2] = static_cast<int>(gen() % 3) + 1;
        printch(' ', point[0], point[1],
                getColor(point[2])[10 - point[3]]);
    }
}

int main(int argc, char *argv[]) {
    constexpr int MAX_COLOR = 3;

    // intilalise random
    std::random_device rd;
    std::mt19937 gen(rd());

    // initlalise ncurses
    initscr();
    curs_set(0);
    int height, width;
    getmaxyx(stdscr, height, width);
    start_color();

    // create colors
    paletteColor::init();
    palettePair::init();

    // make screen white
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            printch(' ', j, i, palettePair::white);
        }
    }
    refresh();

    // create stars
    int MAXSTARS = height * width / 50;
    std::vector<std::vector<int>> stars(
        MAXSTARS); // Each star is stored in a vector of
                   // 4 values x y color current_state
    for (int i; i < MAXSTARS; i++) {
        stars[i] = std::vector<int>{
            static_cast<int>(gen() % height),
            static_cast<int>(gen() % width),
            static_cast<int>(gen() % MAX_COLOR) + 1,
            static_cast<int>(gen() % 9)};
    }

    while (true) {
        for (auto &star : stars) {
            drawStar(star, gen, width, height);
            sleep(50);
            refresh();
        }
    }
    getch();
    endwin();
}

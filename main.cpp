#include <iostream>
#include <ncurses.h>
#include <thread>
#include <vector>
#include <random>

int carId = 0;
int shortSize, longSize, shortStart, shortEnd, longStart, longEnd;
bool running = true, ready = false;

class Car{
public:
    bool constant{};
    int id{};
    int lap{};
    int sleepTime{};
    int colPos{};
    int rowPos{};
    bool onTrack = false;

    void run(){
        onTrack = true;
        if (constant){
            while (running) {
                std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
                if (rowPos == longStart){
                    if (colPos == shortEnd - 1){
                        colPos = shortEnd;
                        rowPos = longStart + 1;
                        continue;
                    }
                    colPos++;
                }
                if (colPos == shortEnd){
                    if (rowPos == longEnd - 1){
                        rowPos = longEnd;
                        colPos = shortEnd - 1;
                        continue;
                    }
                    rowPos++;
                }
                if (rowPos == longEnd){
                    if (colPos == shortStart + 1) {
                        colPos = shortStart;
                        rowPos = longEnd - 1;
                        continue;
                    }
                    colPos--;
                }
                if (colPos == shortStart){
                    if (rowPos == longStart + 1) {
                        rowPos = longStart;
                        colPos = shortStart + 1;
                        continue;
                    }
                    rowPos--;
                }
            }
        }
        else {
            colPos = longStart + 1;
            rowPos = shortStart;
            lap = 0;
            while (running) {
                std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
                if (rowPos == shortStart) {
                    if (colPos == longEnd - 1) {
                        colPos = longEnd;
                        rowPos = shortStart + 1;
                        continue;
                    }
                    colPos++;
                }
                if (colPos == longEnd) {
                    if (rowPos == shortEnd - 1) {
                        rowPos = shortEnd;
                        colPos = longEnd - 1;
                        continue;
                    }
                    rowPos++;
                }
                if (rowPos == shortEnd) {
                    if (colPos == longStart + 1) {
                        if (lap == 2) {
                            onTrack = false;
                            break;
                        }
                        colPos = longStart;
                        rowPos = shortEnd - 1;
                        continue;
                    }
                    colPos--;
                }
                if (colPos == longStart) {
                    if (rowPos == shortStart + 1) {
                        rowPos = shortStart;
                        colPos = longStart + 1;
                        lap++;
                        continue;
                    }
                    rowPos--;
                }
            }
        }
    }

    Car(bool constant, int id, int lap, int sleepTime, int colPos, int rowPos) {
        this->constant = constant;
        this->id = id;
        this->lap = lap;
        this->sleepTime = sleepTime;
        this->colPos = colPos;
        this->rowPos = rowPos;
    }

    std::thread carThread(){
        return std::thread(&Car::run, this);
    }
};

std::vector<Car> cars;

void redraw(){
    while (running) {
        mvprintw(longStart, shortStart, "@");
        mvprintw(longStart, shortEnd, "@");
        mvprintw(longEnd, shortStart, "@");
        mvprintw(longEnd, shortEnd, "@");
        mvprintw(shortStart, longStart, "@");
        mvprintw(shortStart, longEnd, "@");
        mvprintw(shortEnd, longStart, "@");
        mvprintw(shortEnd, longEnd, "@");
        for (int i = shortStart + 1; i < shortEnd; i++) {
            mvprintw(i, longStart, "|");
            mvprintw(i, longEnd, "|");
            mvprintw(longStart, i, "-");
            mvprintw(longEnd, i, "-");
        }
        for (int i = longStart + 1; i < longEnd; i++) {
            mvprintw(i, shortStart, "|");
            mvprintw(i, shortEnd, "|");
            mvprintw(shortStart, i, "-");
            mvprintw(shortEnd, i, "-");
        }
        for (Car car: cars) {
            if (car.onTrack) {
                char c = (char) (car.id + 65);
                mvprintw(car.rowPos, car.colPos, "%c", c);
            }
        }
        refresh();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

std::vector<int> generateInitialPosition(){
    std::random_device rd;
    std::mt19937 mt(rd());
    std::vector<int> position;
    std::uniform_int_distribution<int> sideRand(0, 3);
    int side = sideRand(mt);
    int verticalPos;
    int horizontalPos;
    std::uniform_int_distribution<int> verticalRand(longStart + 1, longEnd - 1);
    std::uniform_int_distribution<int> horizontalRand(shortStart + 1, shortEnd - 1);
    switch (side){
        case 1:
            verticalPos = verticalRand(mt);
            position.push_back(shortEnd);
            position.push_back(verticalPos);
            break;
        case 3:
            verticalPos = verticalRand(mt);
            position.push_back(shortStart);
            position.push_back(verticalPos);
            break;
        case 0:
            horizontalPos = horizontalRand(mt);
            position.push_back(horizontalPos);
            position.push_back(longStart);
            break;
        case 2:
            horizontalPos = horizontalRand(mt);
            position.push_back(horizontalPos);
            position.push_back(longEnd);
            break;
        default:
            break;
    }
    return position;
}

void spawnCars(){
    std::vector<std::thread> threads;
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> sleepDist(100, 300);
    std::uniform_int_distribution<int> delayDist(5, 10);
    auto position = generateInitialPosition();
    Car car1(true, 0, 0, 500, position[0], position[1]);
    cars.emplace_back(car1);
    position = generateInitialPosition();
    Car car2(true, 1, 0, 500, position[0], position[1]);
    cars.emplace_back(car2);
    position = generateInitialPosition();
    Car car3(true, 2, 0, 500, position[0], position[1]);
    cars.emplace_back(car3);
    carId = 3;
    for (int i = 0; i < 10; i++){
        Car carN(false, carId++, 0, sleepDist(mt), longStart + 1, shortStart);
        cars.emplace_back(carN);
    }
    threads.emplace_back(cars.at(0).carThread());
    threads.emplace_back(cars.at(1).carThread());
    threads.emplace_back(cars.at(2).carThread());
    std::uniform_int_distribution<int> randomCar(0, (int) cars.size() - 1);
    while (running){
        int i = randomCar(mt);
        if (!cars.at(i).onTrack && !cars.at(i).constant) {
            threads.emplace_back(cars.at(i).carThread());
            std::this_thread::sleep_for(std::chrono::seconds(delayDist(mt)));
        }
    }
    for (int i = 0; i < threads.size(); i++){
        threads.at(i).join();
    }
}

void checkSpace(){
    while (running) {
        int t = getch();
        if (t == ' ') {
            running = false;
        }
    }
}

int main() {
    initscr();
    int screenRows, screenColumns;
    getmaxyx(stdscr, screenRows, screenColumns);
    shortSize = 6;
    longSize = 13;
    shortStart = (longSize - shortSize)/2;
    shortEnd = shortStart + shortSize;
    longStart = 0;
    longEnd = longStart + longSize - 1;
    if (screenColumns < longSize || screenRows < longSize){
        endwin();
        std::cout << "Okno jest zbyt małe\n";
        return 0;
    }
    std::thread drawThread(redraw);
    while (!ready) {}
    std::thread end(checkSpace);
    std::thread spawnerThread(spawnCars);
    end.join();
    spawnerThread.join();
    drawThread.join();
    endwin();
    return 0;
}

/*
 * Zadaniem jest zaimplementowanie systemu "świateł drogowych" - przez dane skrzyżowanie mogą przejeżdżać pojazdy z
 * tylko jednego kierunku, po czasie kierunek się zmienia
 */

#include <iostream>
#include <ncurses.h>
#include <thread>
#include <vector>
#include <random>
#include <shared_mutex>
#include <mutex>
#include <condition_variable>

int carId = 0;
int shortSize, longSize, shortStart, shortEnd, longStart, longEnd;
bool running = true, ready = false;
bool dir1, dir2, dir3, dir4;

std::condition_variable cv1, cv2, cv3, cv4;
std::mutex mtx1;
std::mutex mtx2;
std::mutex mtx3;
std::mutex mtx4;
std::mutex m1;
std::mutex m2;
std::mutex m3;
std::mutex m4;

void changeDir(std::mutex* m, std::condition_variable* cv, bool* direction){
    std::lock_guard lk(*m);

    *direction = !*direction;
    cv->notify_all();
}

void runChangeDir(std::mutex* m, std::condition_variable* cv, bool* direction){
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> sleepDist(2000, 3000);
    while (running){
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepDist(mt)));
        changeDir(m, cv, direction);
    }
}

class Crossroad{
    bool taken{};
    int carId{};
private:
    int id{};
    std::mutex* mtx;

public:

    explicit Crossroad(int id){
        this->id = id;
        switch (this->id){
            case 0:
                mtx = &mtx1;
                break;
            case 1:
                mtx = &mtx2;
                break;
            case 2:
                mtx = &mtx3;
                break;
            case 3:
                mtx = &mtx4;
                break;
        }
    }

    void getIntoCrossroad(int carI){
        mtx->lock();

        taken = true;
        this->carId = carI;
    }

    void leaveCrossroad(){
        taken = false;

        mtx->unlock();
    }
};

std::vector<Crossroad> crossroads;

class Car2{
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
                int nextRow = 0;
                int nextCol = 0;
                if (rowPos == longStart){
                    if (colPos == shortEnd - 1){
                        nextCol = shortEnd;
                        nextRow = longStart + 1;
                    } else {
                        nextCol = colPos + 1;
                        nextRow = rowPos;
                    }
                } else if (colPos == shortEnd){
                    if (rowPos == longEnd - 1){
                        nextRow = longEnd;
                        nextCol = shortEnd - 1;
                    } else {
                        nextRow = rowPos + 1;
                        nextCol = colPos;
                    }
                } else if (rowPos == longEnd){
                    if (colPos == shortStart + 1) {
                        nextCol = shortStart;
                        nextRow = longEnd - 1;
                    } else {
                        nextCol = colPos - 1;
                        nextRow = rowPos;
                    }
                } else if (colPos == shortStart){
                    if (rowPos == longStart + 1) {
                        nextRow = longStart;
                        nextCol = shortStart + 1;
                    } else {
                        nextRow = rowPos - 1;
                        nextCol = colPos;
                    }
                }
                if (nextRow == shortStart){
                    if (nextCol == shortStart){
                        if (dir1 == constant)
                            crossroads[0].getIntoCrossroad(id);
                        else {
                            std::unique_lock lk(m1);
                            cv1.wait(lk);
                        }
                    } else if (nextCol == shortEnd){
                        if (dir3 == constant)
                            crossroads[2].getIntoCrossroad(id);
                        else {
                            std::unique_lock lk(m3);
                            cv3.wait(lk);
                        }
                    }
                } else if (nextRow == shortEnd){
                    if (nextCol == shortStart){
                        if (dir2 == constant)
                            crossroads[1].getIntoCrossroad(id);
                        else {
                            std::unique_lock lk(m2);
                            cv2.wait(lk);
                        }
                    } else if (nextCol == shortEnd){
                        if (dir4 == constant)
                            crossroads[3].getIntoCrossroad(id);
                        else {
                            std::unique_lock lk(m4);
                            cv4.wait(lk);
                        }
                    }
                }
                if (rowPos == shortStart){
                    if (colPos == shortStart){
                        crossroads[0].leaveCrossroad();
                    } else if (colPos == shortEnd){
                        crossroads[2].leaveCrossroad();
                    }
                } else if (rowPos == shortEnd){
                    if (colPos == shortStart){
                        crossroads[1].leaveCrossroad();
                    } else if (colPos == shortEnd){
                        crossroads[3].leaveCrossroad();
                    }
                }
                colPos = nextCol;
                rowPos = nextRow;
            }
        }
        else {
            colPos = longStart + 1;
            rowPos = shortStart;
            lap = 0;
            while (running) {
                std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
                int nextRow = 0;
                int nextCol = 0;
                if (rowPos == shortStart) {
                    if (colPos == longEnd - 1) {
                        nextCol = longEnd;
                        nextRow = shortStart + 1;
                    } else {
                        nextCol = colPos + 1;
                        nextRow = rowPos;
                    }
                } else if (colPos == longEnd) {
                    if (rowPos == shortEnd - 1) {
                        nextRow = shortEnd;
                        nextCol = longEnd - 1;
                    } else {
                        nextRow = rowPos + 1;
                        nextCol = colPos;
                    }
                } else if (rowPos == shortEnd) {
                    if (colPos == longStart + 1) {
                        if (lap == 2) {
                            onTrack = false;
                            break;
                        }
                        nextCol = longStart;
                        nextRow = shortEnd - 1;
                    } else {
                        nextCol = colPos - 1;
                        nextRow = rowPos;
                    }
                } else if (colPos == longStart) {
                    if (rowPos == shortStart + 1) {
                        nextRow = shortStart;
                        nextCol = longStart + 1;
                        lap++;
                    } else {
                        nextRow = rowPos - 1;
                        nextCol = colPos;
                    }
                }
                if (nextRow == shortStart){
                    if (nextCol == shortStart){
                        if (dir1 == constant)
                            crossroads[0].getIntoCrossroad(id);
                        else {
                            std::unique_lock lk(m1);
                            cv1.wait(lk);
                        }
                    } else if (nextCol == shortEnd){
                        if (dir2 == constant)
                            crossroads[1].getIntoCrossroad(id);
                        else {
                            std::unique_lock lk(m2);
                            cv2.wait(lk);
                        }
                    }
                } else if (nextRow == shortEnd){
                    if (nextCol == shortStart){
                        if (dir3 == constant)
                            crossroads[2].getIntoCrossroad(id);
                        else {
                            std::unique_lock lk(m3);
                            cv3.wait(lk);
                        }
                    } else if (nextCol == shortEnd){
                        if (dir4 == constant)
                            crossroads[3].getIntoCrossroad(id);
                        else {
                            std::unique_lock lk(m4);
                            cv4.wait(lk);
                        }
                    }
                }
                if (rowPos == shortStart){
                    if (colPos == shortStart){
                        crossroads[0].leaveCrossroad();
                    } else if (colPos == shortEnd){
                        crossroads[1].leaveCrossroad();
                    }
                } else if (rowPos == shortEnd){
                    if (colPos == shortStart){
                        crossroads[2].leaveCrossroad();
                    } else if (colPos == shortEnd){
                        crossroads[3].leaveCrossroad();
                    }
                }
                rowPos = nextRow;
                colPos = nextCol;
            }
        }
    }

    Car2(bool constant, int id, int lap, int sleepTime, int colPos, int rowPos) {
        this->constant = constant;
        this->id = id;
        this->lap = lap;
        this->sleepTime = sleepTime;
        this->colPos = colPos;
        this->rowPos = rowPos;
    }

    std::thread carThread(){
        return std::thread(&Car2::run, this);
    }
};

std::vector<Car2> cars;

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
        if (dir1) mvprintw(shortStart, shortStart, "|");
        else mvprintw(shortStart, shortStart, "-");
        if (dir2) mvprintw(shortStart, shortEnd, "|");
        else mvprintw(shortStart, shortEnd, "-");
        if (dir3) mvprintw(shortEnd, shortStart, "|");
        else mvprintw(shortEnd, shortStart, "-");
        if (dir4) mvprintw(shortEnd, shortEnd, "|");
        else mvprintw(shortEnd, shortEnd, "-");
        for (Car2 car: cars) {
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
    Car2 car1(true, 0, 0, 500, position[0], position[1]);
    cars.emplace_back(car1);
    position = generateInitialPosition();
    Car2 car2(true, 1, 0, 500, position[0], position[1]);
    cars.emplace_back(car2);
    position = generateInitialPosition();
    Car2 car3(true, 2, 0, 500, position[0], position[1]);
    cars.emplace_back(car3);
    carId = 3;
    for (int i = 0; i < 10; i++){
        Car2 carN(false, carId++, 0, sleepDist(mt), longStart + 1, shortStart);
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
    shortSize = 6+5;
    longSize = 13+5;
    shortStart = (longSize - shortSize)/2;
    shortEnd = shortStart + shortSize;
    longStart = 0;
    longEnd = longStart + longSize - 1;
    if (screenColumns < longSize || screenRows < longSize){
        endwin();
        std::cout << "Okno jest zbyt małe\n";
        return 0;
    }
    for (int i = 0; i < 4; i++){
        Crossroad c(i);
        crossroads.push_back(c);
    }
    std::thread cross1(runChangeDir, &m1, &cv1, &dir1);
    std::thread cross2(runChangeDir, &m2, &cv2, &dir2);
    std::thread cross3(runChangeDir, &m3, &cv3, &dir3);
    std::thread cross4(runChangeDir, &m4, &cv4, &dir4);
    std::thread drawThread(redraw);
    while (!ready) {}
    std::thread end(checkSpace);
    std::thread spawnerThread(spawnCars);
    end.join();
    cross1.join();
    cross2.join();
    cross3.join();
    cross4.join();
    spawnerThread.join();
    drawThread.join();
    endwin();
    return 0;
}

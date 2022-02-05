#include <iostream>
#include <memory>
#include <random>
#include <vector>
#include <thread>
#include "TestBPM.h"

int main() {
    auto BPM = CreateManager();
    vector<unique_ptr<thread>> tasks;

    for(int i = 0; i < 4; i++) {

        auto task = make_unique<thread> (Task <decltype(BPM)>, BPM, i);
        tasks.emplace_back(move(task));
    }

    for(int i = 0; i < 4; i++) {
        tasks[i]->join();
    }
    return 0;
}

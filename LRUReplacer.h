//
// Created by ccx on 2021/12/17.
//

#ifndef OS_LRUREPLACER_H
#define OS_LRUREPLACER_H
#include <unordered_map>
#include <list>
#include <mutex>
#include <optional>
#include "Type.h"
using namespace std;

class LRUReplacer{
public:
    optional<FrameId> Victim();
    void Pin(FrameId frame_id);
    void Unpin(FrameId frame_id);
    Size GetSize() const;
private:
    mutex latch_;
    list<FrameId> frame_list_;
    unordered_map<FrameId, list<FrameId>::iterator> table_;
};
#endif //OS_LRUREPLACER_H

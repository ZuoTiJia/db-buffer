//
// Created by ccx on 2021/12/17.
//
#include <cassert>
#include "LRUReplacer.h"

optional<FrameId> LRUReplacer::Victim() {
    lock_guard<mutex> lock_guard(latch_);
    if(frame_list_.empty()) {
        return {};
    } else {
        FrameId res_frame = frame_list_.back();
        frame_list_.pop_back();
        table_.erase(res_frame);
        return res_frame;
    }
}

void LRUReplacer::Pin(FrameId frame_id) {
    lock_guard<mutex> lock_guard(latch_);
    if(table_.contains(frame_id)) {
        auto iter = table_[frame_id];
        frame_list_.erase(iter);
        table_.erase(frame_id);
    }


}

void LRUReplacer::Unpin(FrameId frame_id) {
    lock_guard<mutex> lock_guard(latch_);
    if(table_.contains(frame_id)) {
        return;
    }
    frame_list_.push_front(frame_id);
    table_[frame_id] = frame_list_.begin();
}

Size LRUReplacer::GetSize() const {
    return frame_list_.size();
}
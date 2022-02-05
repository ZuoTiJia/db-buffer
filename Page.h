//
// Created by ccx on 2021/12/16.
//

#ifndef OS_PAGE_H
#define OS_PAGE_H
#include <array>
#include <memory>
#include <shared_mutex>
#include <atomic>
#include "Type.h"
#include "cassert"

using namespace std;
class Page {
public:
    const array<char, PAGE_SIZE>* GetDate() const {
        return &data_;
    }
    array<char, PAGE_SIZE>* GetDate() {
        return &data_;
    }
    bool IsDirty() const {
        return dirty_flag_;
    }
    unsigned int GetPinCount() const {
        return pin_count_;
    }
    void WLatch() {
        rw_latch_.lock();

    }
    void WUnlatch() {
        rw_latch_.unlock();
    }
    void RLatch() {
        pin_count_++;
        rw_latch_.lock_shared();

    }
    void RUnLatch() {
        pin_count_--;
        rw_latch_.unlock_shared();
    }

    void SetDirty() {
        dirty_flag_ = true;
    }
    void SetFrameId(FrameId frame_id) {
        frame_id_ = frame_id;
    }
    FrameId GetFrameId() const {
        return frame_id_;
    }

protected:
private:
    array<char, PAGE_SIZE> data_ = {};
    bool dirty_flag_ = false;
    atomic_uint pin_count_ {};
    shared_mutex rw_latch_;
    int frame_id_;
};

#endif //OS_PAGE_H

//
// Created by ccx on 2021/12/18.
//

#ifndef OS_DISKMANAGER_H
#define OS_DISKMANAGER_H
#include "Type.h"
#include <list>
#include <mutex>
#include <array>
#include <memory>
#include <iostream>
#include <atomic>
#include <fmt/format.h>
#include <cassert>
using namespace std;

class DiskManager {
public:
    DiskManager() = default;
    void WritePage(PageId page_id, const array<char, PAGE_SIZE>* page_data) {
        assert(page_id != INVALID_PAGE_ID);
        io_count_++;
    }
    void LoadPage(PageId page_id, array<char, PAGE_SIZE>* page_data) {
        assert(page_id != INVALID_PAGE_ID);
        io_count_++;
    }
    PageId AllocatePage() {
        lock_guard<mutex> lock_guard(latch_);
        if(free_pages_.empty()) {
            return next_page_++;
        } else {
            PageId page_id = free_pages_.front();
            free_pages_.pop_front();
            return page_id;
        }
    }
    void DeletePage(PageId page_id) {
        lock_guard<mutex> lock_guard(latch_);
        free_pages_.push_front(page_id);
    }
    uint GetIO() const {
        return io_count_;
    }
private:
    mutex latch_;
    list<PageId> free_pages_;
    atomic_uint32_t next_page_ = 0;
    atomic_uint io_count_ = 0;
};


#endif //OS_DISKMANAGER_H

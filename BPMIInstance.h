//
// Created by ccx on 2021/12/17.
//

#ifndef OS_BPMIINSTANCE_H
#define OS_BPMIINSTANCE_H
#include <mutex>
#include <vector>
#include <unordered_map>
#include <list>
#include "Page.h"
#include "DiskManager.h"
// #include "BufferPoolManagerImp.h"

using namespace std;
template <typename Replacer, Size pool_size = 10>
class BPMIInstance {
public:
    BPMIInstance();
    
    Page *FetchPgImp(PageId page_id);  

    bool UnpinPgImp(PageId page_id, bool is_dirty, bool isDeleteBuffer);

    bool FlushPgImp(PageId page_id);
    pair<Page *, PageId> NewPgImp();
    bool DeletePgImp(PageId page_id);
    void FlushAllPgsImp();
    Size GetPoolSize() const {
        return pool_size;
    }
    int GetIO() const {
        return disk_manager_->GetIO();
    }
    int GetVisit() const {
        return visit_count_;
    }
private:
    optional<FrameId> AllocateFrame(PageId);
    PageId AllocatePage();
    bool DeallocatePage(PageId page_id);
    array<Page, pool_size> pages_;
    unordered_map<PageId, FrameId> page_table_;
    unique_ptr<Replacer> replacer_;
    unique_ptr<DiskManager> disk_manager_;
    list<FrameId> free_list_;
    mutex latch_;
    atomic_uint visit_count_ = 0;
};

template <typename Replacer, Size pool_size>
BPMIInstance<Replacer, pool_size>::BPMIInstance()
: replacer_(make_unique<Replacer> ())
, disk_manager_(make_unique<DiskManager>())
{
    for(Size i = 0; i < pool_size; i++) {
        free_list_.push_back(i);
    }
    for(int i = 0; i < pool_size; i++) {
        pages_[i].SetFrameId(i);
    }
}

template <typename Replacer, Size pool_size>
Page *BPMIInstance<Replacer, pool_size>::FetchPgImp(PageId page_id) {
    visit_count_++;
    assert(page_id != INVALID_PAGE_ID);
    lock_guard<mutex> lock_guard(latch_);
    if(page_table_.contains(page_id)) {
        FrameId frame_id = page_table_[page_id];
        replacer_->Pin(frame_id);
        page_table_[page_id] = frame_id;
        return &pages_[frame_id];
    }
    auto optional_frame = AllocateFrame(page_id);
    if(optional_frame.has_value()) {
        FrameId frame_id = optional_frame.value();
        disk_manager_->LoadPage(page_id,pages_[frame_id].GetDate());
        replacer_->Pin(frame_id);
        return &pages_[frame_id];
    } else {
        return nullptr;
    }
}

template <typename Replacer, Size pool_size>
bool BPMIInstance<Replacer, pool_size>::UnpinPgImp(PageId page_id, bool is_dirty, bool isDeleteBuffer) {
    assert(page_id != INVALID_PAGE_ID);
    lock_guard<mutex> lock_guard(latch_);
    if(page_table_.contains(page_id)) {
        FrameId frame_id = page_table_[page_id];
        if(is_dirty) {
            pages_[frame_id].SetDirty();
        }
        replacer_->Unpin(frame_id);
        return true;
    }

//    assert(page_table_.contains(page_id));
    return true;
}


template <typename Replacer, Size pool_size>
pair<Page *, PageId> BPMIInstance<Replacer, pool_size>::NewPgImp() {
    visit_count_++;
    lock_guard<mutex> lock_guard(latch_);
    PageId page_id = AllocatePage();
    auto option_frame = AllocateFrame(page_id);
    if(option_frame.has_value()) {
        FrameId frame_id = option_frame.value();
        replacer_->Pin(frame_id);
        return make_pair(&pages_[frame_id], page_id);
    } else {
        return {nullptr, INVALID_PAGE_ID};
    }
}

template <typename Replacer, Size pool_size>
bool BPMIInstance<Replacer, pool_size>::DeletePgImp(PageId page_id) {
    visit_count_++;
    assert(page_id != INVALID_PAGE_ID);
    lock_guard<mutex> lock_guard(latch_);
    if(page_table_.contains(page_id)) {
        FrameId frame_id = page_table_[page_id];
        const auto page = &pages_[frame_id];
        if(page->GetPinCount() > 0) {
            return false;
        } else {
            page_table_.erase(page_id);
            free_list_.push_front(frame_id);
            disk_manager_->DeletePage(page_id);
            return true;
        }
    } else {
        disk_manager_->DeletePage(page_id);
        return true;
    }

}

template <typename Replacer, Size pool_size>
bool BPMIInstance<Replacer, pool_size>::FlushPgImp(PageId page_id) {
    assert(page_id != INVALID_PAGE_ID);
    lock_guard<mutex> lock_guard(latch_);
    if(page_table_.contains(page_id)) {
        FrameId frame_id = page_table_[page_id];
        const auto page = pages_[frame_id];
        if(page.IsDirty()) {
            disk_manager_->WritePage(page_id, page.GetDate());
        }
    }
    return true;
}

template <typename Replacer, Size pool_size>
void BPMIInstance<Replacer, pool_size>::FlushAllPgsImp() {
    assert(page_table_.size() <= pool_size);
    for(auto [page_id, frame_id]: page_table_) {
        const auto page = pages_[frame_id];
        if(page.IsDirty()) {
            disk_manager_->WritePage(page_id, page.GetDate());
        }
    }
}

template <typename Replacer, Size pool_size>
optional<FrameId> BPMIInstance<Replacer, pool_size>::AllocateFrame(PageId page_id) {
    assert(page_id != INVALID_PAGE_ID);
    if(!free_list_.empty()) {
        FrameId frame_id = free_list_.front();
        free_list_.pop_front();
        page_table_[page_id] = frame_id;
        return frame_id;
    } else {
        auto option = replacer_->Victim();
        if(option.has_value()) {
            FrameId victim_frame_id = option.value();
            const Page * victim_page = &pages_[victim_frame_id];

            PageId victim_page_id = INVALID_PAGE_ID;

            for(auto [_page_id, _frame_id]: page_table_) {
                if(victim_frame_id == _frame_id) {
                    victim_page_id = _page_id;
                }
            }


            assert(victim_frame_id == page_table_[victim_page_id]);
            assert(victim_page_id != INVALID_PAGE_ID);
            if(victim_page->IsDirty()) {
                disk_manager_->WritePage(victim_page_id, victim_page->GetDate());
            }
            page_table_.erase(victim_page_id);
            page_table_[page_id] = victim_frame_id;
            return victim_frame_id;
        }
    }
    return optional<FrameId> {};
}

template <typename Replacer, Size pool_size>
PageId BPMIInstance<Replacer, pool_size>::AllocatePage() {
    return disk_manager_->AllocatePage();
}

template <typename Replacer, Size pool_size>
bool BPMIInstance<Replacer, pool_size>::DeallocatePage(PageId page_id) {
    disk_manager_->DeletePage(page_id);
    return true;
}
#endif //OS_BPMIINSTANCE_H
//
// Created by ccx on 2021/12/18.
//

#ifndef OS_TESTBPM_H
#define OS_TESTBPM_H
#include <memory>
#include <random>
#include <thread>
#include "BufferPoolManager.h"
#include "BPMIInstance.h"
#include "Type.h"
#include "LRUReplacer.h"
using namespace std;
auto CreateManager() {
    return make_shared<BufferPoolManager<BPMIInstance<LRUReplacer>>> ();
}

template <typename BufferPoolManager>
class TestBPM {
public:
    TestBPM(std::shared_ptr<BufferPoolManager> buffer_pool_manager, int number)
    : buffer_pool_manager_(std::move(buffer_pool_manager))
    , number_(number)
    {}

    void Run() {
        for(int i = 1; ;i++) {
            auto state = GetRandomUInt(0, 2);
            if (state == 0 && create_pages_.size() <= 100) {
                TestNewPage(number_);
            } else if (state == 1) {
                TestDeletePage(number_);
            } else if (state == 2) {
                TestFetchPage(number_);
            }
            if(i % 100 == 0) {
                int io_count = buffer_pool_manager_->GetIO();
                int visit_count = buffer_pool_manager_->GetVisit();
                fmt::print("Task {} IO {} Visit {} Hit {}\n", number_, io_count, visit_count, double(visit_count - io_count) / visit_count);
            }
        }
    }
    void TestNewPage(int number) {
        auto [page, page_id] = buffer_pool_manager_->NewPage();
        if(page != nullptr) {
            assert(page_id != INVALID_PAGE_ID);
            create_pages_.push_back(page_id);
            fmt::print("Task {} NewPage    Success Page {:#06X} Frame {:#06X}\n", number, page_id, page->GetFrameId());
            RandomWR(page, page_id);
        } else {
            fmt::print("Task {} NewPage    Fail\n", number);
        }
    }
    void TestFetchPage(int number) {
        if(!create_pages_.empty()) {
            auto index = create_pages_.begin() + GetRandomUInt(0, create_pages_.size() - 1);
            PageId page_id = *index;
            auto page = buffer_pool_manager_->FetchPage(page_id);
            while(page == nullptr) {
//                this_thread::sleep_for(chrono::seconds(1));
                fmt::print("Task {} FetchPage  Fail    Page {:#06X}\n", number, page_id);
            }
                fmt::print("Task {} FetchPage  Success Page {:#06X} Frame {:#06X}\n", number, page_id, page->GetFrameId());
            RandomWR(page, page_id);
        }
    }
    void TestDeletePage(int number) {
        if(!create_pages_.empty()) {
            auto index = create_pages_.begin() + GetRandomUInt(0, create_pages_.size() - 1);
            PageId page_id = *index;
            create_pages_.erase(index);
            while(!buffer_pool_manager_->DeletePage(page_id)) {
//                this_thread::sleep_for(chrono::seconds(1));
                fmt::print("Task {} DeletePage Fail    Page {:#06X}\n", number, page_id);
            }
                fmt::print("Task {} DeletePage Success Page {:#06X}\n", number, page_id);
        }

    }
private:
    void RandomWR(Page *page, PageId page_id) {
        if(GetRandomUInt(0, 1) == 0) {
            WritePage(page);
            buffer_pool_manager_->UnpinPage(page_id, true, false);
        } else {
            ReadPage(page);
            buffer_pool_manager_->UnpinPage(page_id, false, false);
        }

    }
    void WritePage(Page *page) {
        page->WLatch();
//        this_thread::sleep_for(GetRandomSleepTime());
        page->WUnlatch();
    }
    void ReadPage(Page *page) {
        page->RLatch();
//        this_thread::sleep_for(GetRandomSleepTime());
        page->RUnLatch();
    }

    unsigned int GetRandomUInt(int begin, int end) {
        uniform_int_distribution<unsigned int> u(begin, end);
        return u(random_engine_);
    }
    chrono::seconds GetRandomSleepTime() {
        return chrono::seconds (GetRandomUInt(0, 3));
    }
    shared_ptr<BufferPoolManager> buffer_pool_manager_;
    default_random_engine random_engine_;
    vector<PageId> create_pages_;
    int number_;
};
template <typename BufferPoolManagerSharedPtr>
void Task(BufferPoolManagerSharedPtr buffer_pool_manager, int number) {
    TestBPM test_bpm(std::move(buffer_pool_manager), number);
    test_bpm.Run();
}
#endif //OS_TESTBPM_H

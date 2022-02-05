//
// Created by ccx on 2021/12/16.
//

#ifndef OS_BUFFERPOOLMANAGER_H
#define OS_BUFFERPOOLMANAGER_H

#include <optional>
#include <utility>
#include <memory>
#include <fmt/format.h>
#include "Page.h"
using namespace std;
template <typename BufferPoolManagerImp>
class BufferPoolManager {
public:
    Page * FetchPage(PageId page_id) {
        return buffer_pool_manager_imp_.FetchPgImp(page_id);
    }
    bool UnpinPage(PageId page_id, bool dirty_flag, bool is_delete_buffer) {
        return buffer_pool_manager_imp_.UnpinPgImp(page_id, dirty_flag, is_delete_buffer);
    }
    bool FlushPage(PageId page_id) {
        return buffer_pool_manager_imp_.FlushPgImp(page_id);
    }
    pair<Page *, PageId> NewPage() {
        return buffer_pool_manager_imp_.NewPgImp();
    }
    bool DeletePage(PageId page_id) {
        return buffer_pool_manager_imp_.DeletePgImp(page_id);
    }
    void FlushAllPages() {
        return buffer_pool_manager_imp_.FlushAllPgsImp();
    }
    Size GetPoolSize() const {
        return buffer_pool_manager_imp_.GetPoolSize();
    }
    int GetIO() const {
        return buffer_pool_manager_imp_.GetIO();
    }
    int GetVisit() const {
        return buffer_pool_manager_imp_.GetVisit();
    }
protected:
private:
    BufferPoolManagerImp buffer_pool_manager_imp_;
};



#endif //OS_BUFFERPOOLMANAGER_H

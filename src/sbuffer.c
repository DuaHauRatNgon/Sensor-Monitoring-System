#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "sbuffer.h"

// Định nghĩa Macro cho Reader ID (Có thể đưa vào file datamgr.h và storagemgr.h sau)
#define READER_DATA_MGR 1
#define READER_STORAGE_MGR 2


int sbuffer_init(sbuffer_t **buffer) {
    *buffer = (sbuffer_t *)malloc(sizeof(sbuffer_t));

    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;

    if (pthread_mutex_init(&((*buffer)->mutex), NULL) != 0) {
        free(*buffer);
        return -1;
    }
    if (pthread_cond_init(&((*buffer)->cond_new_data), NULL) != 0) {
        pthread_mutex_destroy(&((*buffer)->mutex));
        free(*buffer);
        return -1;
    }
    return 0;
}


int sbuffer_free(sbuffer_t **buffer) {

    // Phải khóa Mutex trước khi dọn dẹp để chắc chắn không luồng nào đang truy cập
    pthread_mutex_lock(&((*buffer)->mutex));

    sbuffer_node_t *dummy;
    while ((*buffer)->head != NULL) {
        dummy = (*buffer)->head;
        (*buffer)->head = (*buffer)->head->next;
        free(dummy);
    }

    pthread_mutex_unlock(&((*buffer)->mutex));

    pthread_mutex_destroy(&((*buffer)->mutex));
    pthread_cond_destroy(&((*buffer)->cond_new_data));
    
    free(*buffer);
    *buffer = NULL;
    return 0;
}


int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data) {
    if (buffer == NULL || data == NULL) return -1;

    sbuffer_node_t *new_node = (sbuffer_node_t *)malloc(sizeof(sbuffer_node_t));

    new_node->data = *data;
    new_node->next = NULL;
    new_node->read_by_data_mgr = false;
    new_node->read_by_storage_mgr = false;

    // lock
    pthread_mutex_lock(&(buffer->mutex));

    // add to end of list
    if (buffer->tail == NULL) {
        // is list is empty
        buffer->head = new_node;
        buffer->tail = new_node;
    } 
    else {
        buffer->tail->next = new_node;
        buffer->tail = new_node;
    }

    // unlock
    pthread_mutex_unlock(&(buffer->mutex));

    // wake up sleep threads
    pthread_cond_broadcast(&(buffer->cond_new_data));

    return 0;
}


int sbuffer_read(sbuffer_t *buffer, sensor_data_t *data, int reader_id) {
    if (buffer == NULL || data == NULL) return -1;

    pthread_mutex_lock(&(buffer->mutex));

    while (1) {
        sbuffer_node_t *curr = buffer->head;
        sbuffer_node_t *prev = NULL;
        bool found_unread_data = false;

        // Duyệt qua danh sách để tìm Node chưa được luồng này đọc
        while (curr != NULL) {
            bool can_read = (reader_id == READER_DATA_MGR && !curr->read_by_data_mgr) ||
                            (reader_id == READER_STORAGE_MGR && !curr->read_by_storage_mgr);

            if (can_read) {
                // Đã tìm thấy dữ liệu! Copy nội dung ra ngoài
                *data = curr->data;
                
                // Đánh dấu luồng hiện tại đã đọc xong
                if (reader_id == READER_DATA_MGR) curr->read_by_data_mgr = true;
                if (reader_id == READER_STORAGE_MGR) curr->read_by_storage_mgr = true;

                // KIỂM TRA ĐIỀU KIỆN XÓA NODE:
                // Một node chỉ bị xóa khi CẢ HAI luồng đều đã đọc xong nó.
                if (curr->read_by_data_mgr && curr->read_by_storage_mgr) {
                    if (prev == NULL) { // Đang xóa ở đầu danh sách (head)
                        buffer->head = curr->next;
                        if (buffer->head == NULL) buffer->tail = NULL;
                    } else { // Xóa ở giữa hoặc cuối danh sách
                        prev->next = curr->next;
                        if (curr->next == NULL) buffer->tail = prev;
                    }
                    free(curr); // Giải phóng bộ nhớ
                }
                
                found_unread_data = true;
                break; // Thoát khỏi vòng lặp duyệt (while curr != NULL)
            }
            prev = curr;
            curr = curr->next;
        }

        // Xử lý sau khi duyệt danh sách
        if (found_unread_data) {
            break; // Thoát vòng lặp while(1) vì đã lấy được dữ liệu, chuẩn bị return
        } else {
            // NẾU KHÔNG TÌM THẤY DỮ LIỆU CHƯA ĐỌC: Luồng sẽ vào trạng thái ngủ.
            // pthread_cond_wait sẽ tự động nhả (unlock) mutex và đưa thread vào Sleep.
            // Khi bị đánh thức bởi pthread_cond_broadcast ở sbuffer_insert, 
            // nó sẽ tự động khóa (lock) lại mutex và tiếp tục vòng lặp while(1).
            pthread_cond_wait(&(buffer->cond_new_data), &(buffer->mutex));
        }
    }

    pthread_mutex_unlock(&(buffer->mutex));
    return 0;
}
#ifndef LOGGER_H
#define LOGGER_H

// Hàm này chứa vòng lặp vô hạn đọc từ FIFO và ghi ra file
void run_log_process();

// API để các threads khác gọi khi muốn ghi log vào FIFO
void write_log_event(const char *msg);

#endif 
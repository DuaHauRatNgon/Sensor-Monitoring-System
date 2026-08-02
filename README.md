# Sensor-Monitoring-System

[ START PROGRAM ]
       │
       ▼
 1. Tạo Named Pipe (FIFO) "logFifo" nếu chưa tồn tại (mkfifo)
       │
       ▼
 2. Gọi fork() để tạo Tiến trình Con (Log Process)
       ├─────────────────────────────────┐
       ▼ [Child Process]                 ▼ [Parent Process - Main Process]
  Chạy logic log_process()           3. Khởi tạo sbuffer (thread-safe)
  (Mở FIFO chế độ Read ->                 │
   Ghi ra gateway.log)                   ▼
                                     4. Khởi tạo kết nối SQLite (hoặc để StorageMgr tự mở)
                                          │
                                          ▼
                                     5. pthread_create() -> Storage Manager Thread
                                     6. pthread_create() -> Data Manager Thread
                                     7. pthread_create() -> Connection Manager Thread (truyền Port từ CLI)
                                          │
                                          ▼
                                     8. pthread_join() -> Chờ các luồng kết thúc khi có tín hiệu dừng
                                          │
                                          ▼
                                     9. Dọn dẹp: free(sbuffer), hủy FIFO, kill/wait Child Process
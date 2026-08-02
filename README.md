# Sensor-Monitoring-System

sensor_monitoring_system/
├── Makefile                # Script đóng gói, compile, test và chạy Valgrind
├── README.md               # Hướng dẫn build, chạy và mô tả kiến trúc
├── room_sensor.map         # File cấu hình map ID sensor <-> Số phòng (Req 5 & Exercise 1)
├── gateway.log             # Log file do Log Process sinh ra (Req 8)
├── Sensor.db               # File database SQLite lưu trữ dữ liệu sensor (Req 6)
│
├── inc/                    # Các file Header (.h) chứa khai báo prototype, struct, macro
│   ├── config.h            # Các cấu hình hệ thống (PORT mặc định, tên FIFO, size buffer...)
│   ├── sbuffer.h           # Header của Shared Buffer (đặc tả thread-safe queue)
│   ├── connmgr.h           # Connection Manager (xử lý TCP, multithread clients)
│   ├── datamgr.h           # Data Manager (đọc file map, tính running average)
│   ├── sensor_db.h         # Storage Manager / SQLite wrapper library
│   ├── logger.h            # Logger interface (ghi message vào logFifo)
│   └── log_process.h       # logic của tiến trình Log Process (đọc FIFO -> ghi file)
│
├── src/                    # Các file mã nguồn (.c) triển khai logic
│   ├── main.c              # Hàm main: fork Log Process, khởi tạo sbuffer, spawn 3 threads
│   ├── sbuffer.c           # Hiện thực linked-list sbuffer + mutex / condition variable
│   ├── connmgr.c           # Lắng nghe TCP socket (poll/select/epoll hoặc thread-per-client)
│   ├── datamgr.c           # Logic nghiệp vụ kiểm tra nhiệt độ nóng/lạnh
│   ├── sensor_db.c         # Kết nối SQLite, cơ chế retry 3 lần mỗi 5 giây (Req 6)
│   ├── logger.c            # API cho các thread Main Process gửi log vào FIFO (Req 7)
│   └── log_process.c       # Vòng lặp chính của tiến trình Log Process
│
└── test/                   # Code kiểm thử tự động (Unit Test / Stress Test)
    ├── test_sbuffer.c      # Kiểm thử riêng tính thread-safe của sbuffer
    └── sensor_node_mock.c  # Mạng giả lập (client giả gửi gói tin TCP dồn dập)
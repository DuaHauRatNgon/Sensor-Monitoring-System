#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>
#include <errno.h>

#include "connmgr.h"
#include "sbuffer.h"
#include "logger.h" 

#define MAX_SENSORS 10 

void *connmgr_run(void *arg) {
    // Ép kiểu tham số truyền vào
    // Tham số thực tế chúng ta cần thiết kế lại một struct chứa port và sbuffer
    // Để đơn giản theo main.c, ta tạm hiểu arg là con trỏ chứa thông tin port.
    // Dưới đây tôi fix cứng port để bạn dễ test, hoặc bạn có thể pass qua struct.
    
    // Giả sử arg là một struct tự định nghĩa (cần update trong main.c):
    // typedef struct { int port; sbuffer_t *buffer; } conn_args_t;
    // conn_args_t *args = (conn_args_t *)arg;
    
    // Tạm thời fix cứng port và giả định arg là shared_buffer
    int port = 1234; // Bạn nên lấy từ argv trong main truyền vào
    sbuffer_t *shared_buffer = (sbuffer_t *)arg;

    int server_fd;
    struct sockaddr_in server_addr;



    // 1 tao socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // Cho phép reuse port ngay lập tức sau khi tắt server
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);



    // 2 bind 
    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));



    // 3 listen
    listen(server_fd, MAX_SENSORS);

    printf("[ConnMgr] Lắng nghe kết nối TCP trên port %d...\n", port);

    // nhiều client -> nhiều fd -> io multiplex (slect, poll, epoll) -> poll
    struct pollfd fds[MAX_SENSORS + 1];
    memset(fds, 0, sizeof(fds));

    fds[0].fd = server_fd; // Index 0 luôn là server socket để nhận kết nối mới
    fds[0].events = POLLIN;

    for (int i = 1; i <= MAX_SENSORS; i++) {
        fds[i].fd = -1; // đặt cờ -1 nghĩa là slot trống
    }

    while (1) {
        int activity = poll(fds, MAX_SENSORS + 1, -1); 

        // Kiểm tra xem có kết nối MỚI đến server socket không
        if (fds[0].revents & POLLIN) {
            int new_socket = accept(server_fd, NULL, NULL);
            if (new_socket >= 0) {
                // Tìm một slot trống trong mảng pollfd để lưu client mới
                for (int i = 1; i <= MAX_SENSORS; i++) {
                    if (fds[i].fd == -1) {
                        fds[i].fd = new_socket;
                        fds[i].events = POLLIN;
                        printf("[ConnMgr] Sensor mới đã kết nối! (fd=%d)\n", new_socket);
                        break;
                    }
                }
            }
        }

        // Kiểm tra dữ liệu ĐẾN từ các sensor đang kết nối
        for (int i = 1; i <= MAX_SENSORS; i++) {
            if (fds[i].fd > 0 && (fds[i].revents & POLLIN)) {
                sensor_data_t data;
                int valread = read(fds[i].fd, &data, sizeof(sensor_data_t));

                if (valread > 0) {
                    // Đã nhận được dữ liệu hoàn chỉnh, đẩy vào sbuffer
                    printf("[ConnMgr] Nhận từ Sensor ID %d: Temp=%.2f\n", data.id, data.value);
                    sbuffer_insert(shared_buffer, &data);
                } 
                else if (valread == 0) {
                    // Sensor đã ngắt kết nối
                    printf("[ConnMgr] Sensor (fd=%d) đã ngắt kết nối.\n", fds[i].fd);
                    close(fds[i].fd);
                    fds[i].fd = -1; // Giải phóng slot
                }
            }
        }
    }

    close(server_fd);
    pthread_exit(NULL);
}
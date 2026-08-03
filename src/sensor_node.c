// giả lập sensor - tcp client
// k liên quan tới main proccess
// argv: <IP> <PORT> <SENSOR_ID>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>



typedef struct {
    uint16_t id;
    double value;
    time_t ts;
} sensor_data_t;



int main(int argc, char *argv[]) {

    char *ip = argv[1];
    int port = atoi(argv[2]);
    uint16_t sensor_id = (uint16_t)atoi(argv[3]);

    int sock = 0;
    struct sockaddr_in serv_addr;



    sock = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    inet_pton(AF_INET, ip, &serv_addr.sin_addr);



    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }

    printf("Đã kết nối tới Server. Sensor ID: %d bắt đầu gửi dữ liệu...\n", sensor_id);



    // loop bắn 5 bản tin rồi thoát
    for (int i = 0; i < 5; i++) {
        sensor_data_t data;
        data.id = sensor_id;
        
        // giả lập temp
        data.value = 20.0 + ((double)rand() / RAND_MAX) * 5.0 - 2.5; 
        data.ts = time(NULL);

        send(sock, &data, sizeof(sensor_data_t), 0);
        
        sleep(2); 
    }

    close(sock);
    
    return 0;
}
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pigpio.h>
#include <math.h>

#define BUFFER_LENGTH 1000
#define PORT 2000

#define LEFT_REAR 6
#define RIGHT_REAR 26
#define LEFT_FRONT 13
#define RIGHT_FRONT 19

int main() {
  if (gpioInitialise() < 0) {
    return 1;
  }

  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd == -1) {
    fprintf(stderr, "socket error\n");
    return 1;
  }

  struct sockaddr_in server_address = { .sin_family = AF_INET,
                                        .sin_port = htons(PORT),
                                        .sin_addr.s_addr = htonl(INADDR_ANY) };

  if (bind(sock_fd, (struct sockaddr*) &server_address, sizeof(server_address))) {
    fprintf(stderr, "bind error\n");
    return 1;
  }
  
  if ((listen(sock_fd, 1)) != 0) { 
    fprintf(stderr, "listen error\n"); 
    return 1;
  }

  struct sockaddr_in client_address;
  socklen_t client_address_length = sizeof(client_address); 
  
  int client_fd = accept(sock_fd, (struct sockaddr*) &client_address, &client_address_length);
  if (client_fd < 0) {
    fprintf(stderr, "acccept error\n");
    return 1;
  }

  int result;
  char buffer[BUFFER_LENGTH];

  float current_speed = 0;
  float current_direction = 0;

  int left_motor, left_motor_speed = 0;
  int right_motor , right_motor_speed = 0;

  while (1) {
    memset(buffer, 0, BUFFER_LENGTH);
    result = read(client_fd, buffer, BUFFER_LENGTH);
    if (result <= 0) {
      break;
    }
    char *token = strtok(buffer, "\n");
    while (token != NULL) {
      if (strlen(token) < 9) {
        // Concatenated TCP packets will get discarded
        token = strtok(NULL, "\n");
        continue;
      }
      if (token[0] == 'S') {
        // Process speed
        if (token[2] == ',') {
          token[2] = '.';
        } else {
          token[3] = '.';
        }
        current_speed = strtof(token + 1, NULL);
        // Update motors accordingly
        if (current_speed > 0) {
          // Front
          left_motor = LEFT_FRONT;
          gpioPWM(LEFT_REAR, 0);
          right_motor = RIGHT_FRONT;
          gpioPWM(RIGHT_REAR, 0);
          // update left_motor, right_motor - FRONT
        } else {
          // Back
          left_motor = LEFT_REAR;
          gpioPWM(LEFT_FRONT, 0);
          right_motor = RIGHT_REAR;
          gpioPWM(RIGHT_FRONT, 0);
          // update left_motor, right_motor - REAR
        }
        left_motor_speed = (fabs(current_speed) * 205) + 50;
        right_motor_speed = (fabs(current_speed) * 205) + 50;
      } else if (token[0] == 'D') {
        // Process direction
        if (token[2] == ',') {
          token[2] = '.';
        } else {
          token[3] = '.';
        }
        current_direction = strtof(token + 1, NULL);
        // Update motors accordingly
        if (current_direction > 0) {
          right_motor_speed *= (1 - fabs(current_direction));
        } else {
          left_motor_speed *= (1 - fabs(current_direction));
        }
      } else {
        // Concatenated TCP packets will get discarded
        token = strtok(NULL, "\n");
        continue;
      }
      gpioPWM(left_motor, left_motor_speed);
      gpioPWM(right_motor, right_motor_speed);
      token = strtok(NULL, "\n");
    }
  }
  
  close(sock_fd);
  gpioPWM(LEFT_FRONT, 0);
  gpioPWM(LEFT_REAR, 0);
  gpioPWM(RIGHT_FRONT, 0);
  gpioPWM(RIGHT_REAR, 0);
}

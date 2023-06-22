#include "rs.hpp"
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <ostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
using namespace std;
// NEED to establish what size of ecc we want as well as how much padding
#define ECC_LENGTH 32
#define msglen 233

int open_serial_connection(std::string port) {
  // https://stackoverflow.com/questions/22544751/how-to-send-and-read-data-from-serial-port-in-c
  int fd;
  // char port[] = "/dev/ttyACM0";
  // char cPort[port.size()];
  // std::strncpy(cPort, port.c_str(), sizeof(cPort));
  printf("Starting\n");
  fd = open(port.c_str(), O_RDWR | O_NONBLOCK);
  if (fd == -1) {
    printf("Failed to open serial connection\n");
    exit(1);
  }
  printf("Serial connection open\n");
  struct termios tios;
  tcgetattr(fd, &tios);
  tios.c_iflag = IGNBRK | IGNPAR;
  tios.c_oflag = 0;
  tios.c_lflag = 0;
  cfsetspeed(&tios, B9600);
  if (tcsetattr(fd, TCSAFLUSH, &tios) == -1) {
    printf("tcsetattr failed");
    exit(2);
  }
  usleep(1000);
  printf("Serial connection configured\n");
  return fd;
};
void printBinary(uint8_t value) {
  for (int8_t bit = 7; bit >= 0; --bit) {
    uint8_t mask = 1 << bit;
    std::cout << ((value & mask) ? "1" : "0");
  }
}
void fixArray(const unsigned char *inputArray, size_t inputSize,
              unsigned char *outputArray, size_t outputSize) {
  // Copy input elements up to the minimum of inputSize and outputSize
  size_t copySize = (inputSize < outputSize) ? inputSize : outputSize;
  std::memcpy(outputArray, inputArray, copySize);

  // Fill the remaining space with zeros
  if (outputSize > inputSize) {
    std::memset(outputArray + inputSize, 0, outputSize - inputSize);
  }
}

void natArray(const unsigned char *encodedArray, size_t encodedSize,
              unsigned char *decodedArray) {
  unsigned char actualSize =
      encodedArray[3]; // Assuming the size is stored in the fourth element
  for (size_t i = 0; i < actualSize; ++i) {
    decodedArray[i] = encodedArray[i];
  }
}
void encodeArray(const unsigned char *inputArray, unsigned char *encodedArray) {
  RS::ReedSolomon<4, 4> rs;
  rs.Encode(inputArray, encodedArray);
}
void decodeArray(const unsigned char *encodedArray,
                 unsigned char *decodedArray) {
  RS::ReedSolomon<4, 4> rs;
  rs.Decode(encodedArray, decodedArray);
}
int main() {
  const unsigned char inputArray[] = {0x01, 0x02, 0x03, 0x04};
  const size_t inputSize = sizeof(inputArray) / sizeof(inputArray[0]);

  const size_t fixedSize = 4; // Change this to your desired fixed size
  unsigned char fixedArray[fixedSize];

  // Call the function to encode the input array
  fixArray(inputArray, inputSize, fixedArray, fixedSize);
  // Now we encode with RS codecs
  unsigned char enArr[8];
  encodeArray(fixedArray, enArr);
  /* This is where we implement the sending and recieving of data
   * We expect there to be noise and corruption
   * Once we recieve we can decode
   */

  int send_fd = open_serial_connection("/dev/ttyACM0");
  int recv_fd = open_serial_connection("/dev/ttyACM1");
  // now we can send data and recv data
  // lets establish a way to send the data now
  // write(fd,sendbuff,4)
  // read(fd,j)
  // int bytes = read(fd, recv_buff, sizeof(recv_buff));
  for (;;) {
    write(send_fd, enArr, 8);
    uint8_t recv_buff[8];
    int bytes = read(recv_fd, recv_buff, 8);
    if (bytes > 0) {
      unsigned char decArr[fixedSize];
      decodeArray(recv_buff, decArr);
      unsigned char normArray[decArr[3]]; // Creating an array based on
      natArray(decArr, fixedSize, normArray);
      cout << "Original: ";
      for (size_t i = 0; i < 4; ++i) {
        printBinary(inputArray[i]);
        cout << " ";
      }
      std::cout << std::endl;
      cout << "Decoded: ";
      // Print the decoded array
      for (size_t i = 0; i < 4; ++i) {
        printBinary(decArr[i]);
        cout << " ";
      }
      std::cout << std::endl;
    }
  }
  return 0;
}

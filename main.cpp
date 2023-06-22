#include "Reed-Solomon/include/rs.hpp"
#include <cstring>
#include <fec.h>
#include <iostream>
// NEED to establish what size of ecc we want as well as how much padding
#define ECC_LENGTH 32
#define msglen 233

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
  RS::ReedSolomon<223, 32> rs;
  rs.Encode(inputArray, encodedArray);
}
void decodeArray(const unsigned char *encodedArray,
                 unsigned char *decodedArray) {
  RS::ReedSolomon<223, 32> rs;
  rs.Decode(encodedArray, decodedArray);
}
int main() {
  const unsigned char inputArray[] = {0x01, 0x02, 0x03, 0x04};
  const size_t inputSize = sizeof(inputArray) / sizeof(inputArray[0]);

  const size_t fixedSize = 223; // Change this to your desired fixed size
  unsigned char fixedArray[fixedSize];

  // Call the function to encode the input array
  fixArray(inputArray, inputSize, fixedArray, fixedSize);
  // Now we encode with RS codecs
  unsigned char enArr[255];
  encodeArray(fixedArray, enArr);
  /* This is where we implement the sending and recieving of data
   * We expect there to be noise and corruption
   * Once we recieve we can decode
   */
  enArr[0] = 0xa9;
  // Decode the encoded array
  unsigned char decArr[fixedSize];
  decodeArray(enArr, decArr);
  // naturalize the array
  unsigned char normArray[decArr[3]]; // Creating an array based on
                                      // the actual size
  natArray(decArr, fixedSize, normArray);

  // Print the decoded array
  for (size_t i = 0; i < normArray[3]; ++i) {
    std::cout << std::hex << static_cast<int>(normArray[i]) << " ";
  }
  std::cout << std::endl;

  return 0;
}

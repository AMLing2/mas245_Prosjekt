// K. M. Knausgård / MAS234 2017
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <string>
#include <iostream>
#include <time.h>


struct can_frame_edit { //Can.h default frame gives compile error when using .len - made custom frame to circumvent problem
    canid_t can_id;  /* 32 bit CAN_ID + EFF/RTR/ERR flags */
    __u8 len;
    __u8 __pad; /* padding */
    __u8 __res0; /* reserved / padding */
    __u8 len8_dlc; /* optional DLC for 8 byte payload length (9 .. 15) */
    __u8 data[CAN_MAX_DLEN] __attribute__((aligned(8)));
};



struct can_frame_edit sendFrame; //Outbound frame
struct can_frame_edit receiveFrame; //Inbound frame

// Can bus configuration.
const char *ifname = "can0";
int canSocketDescriptor;

// Task rate control.
//const double periodicTaskRate = 2.0; // Run task at 0.5 Hz.
const int64_t periodicTaskDtNs = static_cast<int64_t>((1.0/0.5) * 1.0e9);



bool createCanSocket(int& socketDescriptor);
void sendCanMessage(int socketDescriptor);

void myPeriodicTask() //Reads can message from buffer, multiplies data by two, returns CAN message
{
    int nbytes = read(canSocketDescriptor, &receiveFrame, sizeof(struct can_frame_edit)); //Read CAN from can buffer.
    int meldingLengde = receiveFrame.len;

    std::cout << "Received from teensy: "; //Print mottatt data

    for (int j = 0; j < meldingLengde; j++) {
        std::cout << receiveFrame.data[j];
    }
    std::cout << std::endl;

    std::string hexTilString;

    for (int j = 0; j < meldingLengde; j++) {
        hexTilString[j] = receiveFrame.data[j];
    }
    
    std::cout << "Message length: " << meldingLengde << std::endl;

    float tallFraImu = std::stof(hexTilString); //Convert string to float
    
    if (tallFraImu < 10.0 && tallFraImu > 4.99) {
    meldingLengde = 5; //Sets message length to 5 if operation increases number of digits (Example: 9.04 * 2 = 18.08, 4 chars to 5 chars.)
    }

    tallFraImu = tallFraImu*2.0;     //Arbitrary operation on CAN message data

    std::cout << "Data after operation: " << tallFraImu; //Confirm operation
    std::cout << std::endl;

    std::string sendTilTeensy = std::to_string(tallFraImu); //Float to string conversion



    sendFrame.can_id = 13;
    
    
 
    sendFrame.len = meldingLengde;
  

    //Load operated value into return CAN message
    for (int j = 0; j < meldingLengde; j++) {
        sendFrame.data[j] = sendTilTeensy[j];
    }


    //Confirm sent data

    std::cout << "Sent to teensy: ";
    for (int i = 0; i < meldingLengde; i++) {
        std::cout << sendFrame.data[i];

    }
    std::cout << std::endl;



   static uint64_t ii(0U);
   ++ii;
   std::cout << "Tick " << ii << ".." << std::endl;

   //Send CAN message
   const int bytesWritten = write(canSocketDescriptor, &sendFrame, sizeof(struct can_frame_edit));
}



int main()
{

  std::cout << "Periodic tick example for MAS234 using clock_nanosleep.." << std::endl;

  // Set up CAN.
  if (!createCanSocket(canSocketDescriptor)) // Passed by reference; canSocketDescriptor is the output variable!
  {
     std::cout << "Could not create CAN socket" << std::endl;
     return 0;
  }

  bool running = true;
  while(running)
  {
    // USE CLOCK_MONOTONIC, and NOT NOT NOT CLOCK_REALTIME.
    // "Realtime" sounds tempting but is non-monotonic and actually a
    // wall-time-realtime, not usable for real time tasks.
    // For hard real time systems, use a RTOS or at least linux with real time extensions.
    // Note: This example has NO error checking (not good).

    // 1. Get current time stamp.
    struct timespec nextTimerDeadline;
    clock_gettime(CLOCK_MONOTONIC, &nextTimerDeadline);


    // 2. Run the periodic task.
    myPeriodicTask();


    // Add number of nanoseconds the task needs to sleep to the initial timestamp.
    // Could add logic here to check if the task used too much time.
    const int64_t nextTvNsec = static_cast<int64_t>(nextTimerDeadline.tv_nsec) + periodicTaskDtNs;
    std::cout << "  " << nextTvNsec << ", " << nextTimerDeadline.tv_nsec << std::endl << std::endl;

    if (nextTvNsec >= 1000000000L)
    {
        // The timespec struct has one part for nanoseconds and one for seconds,
        // nsec part must be less than one second.
        nextTimerDeadline.tv_sec += static_cast<long int>(nextTvNsec / 1000000000L);
        nextTimerDeadline.tv_nsec = static_cast<long int>(nextTvNsec % 1000000000L);
    }

    // 3. Sleep until next deadline.
    // See http://man7.org/linux/man-pages/man2/clock_nanosleep.2.html for documentation.
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &nextTimerDeadline, NULL);
  }


  return 0;
}


bool createCanSocket(int& socketDescriptor)
{

  struct ifreq ifr;

  if((socketDescriptor = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
          perror("Error while opening CAN socket.");
          return false;
  }

  strcpy(ifr.ifr_name, ifname);
  ioctl(socketDescriptor, SIOCGIFINDEX, &ifr);


  struct sockaddr_can addr;
  addr.can_family  = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;

  if(bind(socketDescriptor, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("Error in socketcan bind.");
    return false;
  }

  return true;
}






void sendCanMessage(int socketDescriptor)
{

  // The frame should probably be passed to this function somehow..
  sendFrame.can_id  = 0x234;
  //sendFrame.can_dlc = 2;
  //sendFrame.len = 6;


  const int bytesWritten = write(socketDescriptor, &sendFrame, sizeof(struct can_frame));
  std::cout << "sendte can fra modifisert program" << std::endl;
  // Should handle errors here...
}

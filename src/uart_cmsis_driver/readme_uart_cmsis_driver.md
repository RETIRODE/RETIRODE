UART CMSIS-Driver Sample Code
=============================

NOTE: If you use this sample application for your own purposes, follow
      the licensing agreement specified in `Software_Use_Agreement.rtf`
      in the home directory of the installed RSL10 Software Development Kit
      (SDK).

Overview
--------

This sample project demonstrates the basic functionality of the RSL10 
UART CMSIS-Driver using very simple operations:

1)  Transmits a UART frame containing the string message 
    "RSL10 UART TEST" when the DIO5 button on the Evaluation and Development
    Board is pressed.

2)  When a frame is received containing the string message "RSL10 UART TEST", 
    the LED (GPIO6) is toggled.
  
3)  Can be tested with two Evaluation and Development Boards linked with a 
    UART connection, or using a loopback between RX and TX.
    
4.) Monitors the content being received through UART. If the bytes 
    received do not match what is expected, demonstrates how to use 
    the `ABORT_RECEIVE` command to stop reception and re-start. 

To use this sample application, the `ARM.CMSIS` pack must be installed in your 
IDE. In **CMSIS Pack Manager**, on the right panel, you can see the **Packs 
and Examples** view. In the **Packs** view, you will see **CMSIS packs**. Find 
`ARM.CMSIS` and click on the **Install** button.

Hardware Requirements
---------------------
The UART application needs to be connected to another UART application.

The following connections are required:

    1st UART board        2nd UART board
    UART_RX(DIO2)   ->    UART_TX(DIO3)
    UART_TX(DIO3)   ->    UART_RX(DIO2)
  
Importing a Project
-------------------
To import the sample code into your IDE workspace, refer to the 
*Getting Started Guide* for your IDE for more information.
  
Verification
------------
To verify that the application is working correctly, connect an Evaluation and
Development Board with the UART application configured to a baud rate of 
115200, and use it to send the string message "RSL10 UART TEST". A loopback 
can be used, or another RSL10 Evaluation and Development Board with the UART 
program loaded. When the button is pressed, it sends "RSL10 UART TEST". Each 
time "RSL10 UART TEST" is received, the LED blinks. If two UART applications 
are used, pressing the button on one Evaluation and Development Board causes 
the LED of the other  Evaluation and Development Board to blink.
 
The source code exists in the `app.c` and `app.h` files.

Notes
-----
Sometimes the firmware in RSL10 cannot be successfully re-flashed, due to the
application going into Sleep Mode or resetting continuously (either by design 
or due to programming error). To circumvent this scenario, a software recovery
mode using DIO12 can be implemented with the following steps:

1)  Connect DIO12 to ground.

2)  Press the RESET button (this restarts the application, which pauses at the
    start of its initialization routine).

3)  Re-flash RSL10. After successful re-flashing, disconnect DIO12 from
    ground, and press the RESET button so that the application can now work
    properly.

***
Copyright (c) 2019 Semiconductor Components Industries, LLC
(d/b/a ON Semiconductor).

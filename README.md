# Wave Generator 

FW for sin waveform generator based on STM32 MCU with DAC (PA4) output
- samples = 100
- DC level = 1650[mv]
- Amp. max = 1650[mv]
- Freq. = 500:1500[Hz]  

Several AT commands has been implemented:  
COM port baud rate 115200 (Virtual COM port from NUCLEO)

- Set parameters 

CMD+SET=< AMP >&< Freq >+END     

Amp - Amplitude in [mv]  
Freq - Frequency in [Hz]   

Example:  
Send:    CMD+SET=1650&1000+END  
Receive: CMD+ARG:
         amp=1650[mv]
         freq=1000[Hz]
         samples=100
         wave:
         2048
         2176
         2304
         2431
         2557
         2680
         2801
         2919
         3034
         3145
         3251
         3353
         3449
         3540
         3626
         3704
         3777
         3842
         3901
         3952
         3995
         4031
         4059
         4079
         4091
         4095
         4091
         4079
         4059
         4031
         3995
         3952
         3901
         3842
         3777
         3704
         3626
         3540
         3449
         3353
         3251
         3145
         3034
         2919
         2801
         2680
         2557
         2431
         2304
         2176
         2048
         1919
         1791
         1664
         1538
         1415
         1294
         1176
         1061
         950
         844
         742
         646
         555
         469
         391
         318
         253
         194
         143
         100
         64
         36
         16
         4
         0
         4
         16
         36
         64
         100
         143
         194
         253
         318
         391
         469
         555
         646
         742
         844
         950
         1061
         1176
         1294
         1415
         1538
         1664
         1791
         1919
         +OK+END 

- Get parameters  

CMD+GET+END  

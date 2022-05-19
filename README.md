# How to open a safe

### TL;DR

A vulnerability allow to open electronic safes from Sentry Safe and Master Lock without any pin code.

The company was notified but never responded.

I created an alternative PCB and firmware to patch this issue, [available here](https://github.com/H4ckd4ddy/fix-sentry-safe)

### Story

I just buy a logic analyser, what can I do with it ?

![Logic Analyser](images/analyser.jpg)

Look around... TV... printer... 

... electronic safe !

![Safe](images/safe.jpg)

### Disassembly

Let's try to understand how it work.

Start by disassembly the door to take a look inside.

There is only two circuit boards :

- 1x outside with batteries, keypad, leds & buzzer
- 1x inside with memory & solenoid (who mechanically unlock the bolts)

![Front of boards](images/boards_front.png)
![Back of boards](images/boards_back.png)

### Capture

Between these two board, there is 4 wires (black, yellow, green & red). As colors of wires not always being respected, let's assume that we don't trust them. We can inspect circuit tracks to find wires mapping, but as my new logic analyser can cature 8 channels, let's just plug each wire.

This kind of electronic safe need 5 digits code, so for the first cature, I will just press 5 times the "1" key.

![11111 capture](images/11111.png)

Result : only green wire has changing signal. Black is always low, yellow & red always high (so colors are respected).

We can see on the captured signal 5 repetitions of the same shape in a row, we can assume that match our 5 pressed "1" key.
To decode full signal, we have to search the protocol used.

This safe is not open hardware or software, we don't know how it work.

So take a look at the chips on boards.

Only one microcontroller on each of them.

![Chip](images/chip.jpg)

According the datasheet, main communication protocols used with this microcontroller are SPI and I²C.
But not this time, cause both of them need a clock signal, and there is no move on any of the other wires.

So I guess it can be the most used asynchronous bus protocol : UART

Good news, the open source pulseview software contains a lot of decoders including UART.

But wich frequency use ?

To find out the frequency, I start by measuring time between each shape that we suppose to be pressed key.

![Time](images/time.png)

The whole key take 2.1 ms to be transmitted, and the shortest peak take approximatly 0.2 ms.
UART use basically 1 start bit, 8 data bits & 1 stop bit = 10 bits

2.1 ms / 10 = 0.21 ms

This match with our shortest peak.

10 bits in 2.1 ms
so
(1000 ms / 2.1 ms) * 10 = number of bits that can be transmitted in a second \~= 4762


UART frequency must match 9600 subdivision, 4762 is very close to 4800 wich match 9600/2, a common speed for UART.

So we try to set this frequency in our decoder.

![Decode UART](images/decode.png)

It works ! We can read our 5x "1" keys.

After some tries with different codes, here is some observations about different signal parts :

- signal always begin by a wake up shape with 2.7 ms low state, then 0.25ms high (not part of UART protocol)
- after wake up, signal always send 0x0 byte
- after that, fixed value 0x71 is sent, it seem to be a command byte (0x71 = try an unlock)
- then 5 digits of the pressed code are sent
- finnaly, the last byte change depending code, seem to be a custom checksum

Here is data :

| Command byte | Tried Code | Checksum byte |
|--------------|------------|---------------|
| 0x71         | 11111      | 0x76          |
| 0x71         | 22222      | 0x7B          |
| 0x71         | 12345      | 0x80          |

We can see that checksum correspond to command byte added to each code byte

Now we have all informations to automate this process with a short Arduino function.

```cpp
void send_command(int command, int a, int b, int c, int d, int e) {
  int checksum = (command + a + b + c + d + e);
  
  // Wake-up signal shape
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delayMicroseconds(2750);
  digitalWrite(pin, HIGH);
  delayMicroseconds(200);

  // Data
  Serial.begin(4800);
  Serial.write(0x0);
  Serial.write(command);
  Serial.write(a);
  Serial.write(b);
  Serial.write(c);
  Serial.write(d);
  Serial.write(e);
  Serial.write(checksum);
  Serial.end();

  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
}
```

But even if each code can be tried very fast, the safe contain a bruteforce protection.

So I try to capture signal during code change to see what append.

To change the code :
- Press "P" button
- Enter the factory code
- Enter your new code

![Change code signal](images/change.png)

When we look at the signal :
- Some strange custom shapes are sent during "P" press
- Keypad board send factory code with a 0x74 command byte
- The chip inside respond if factory code is valid or not (factory code is printed on manual, random and cannot be change)
- Keypad send new code with 0x75 command byte

I also capture signal exchange at boot time, so we can now map some functions :

| Command byte | Function                | Code provided |
|--------------|-------------------------|---------------|
| 0x71         | Try to unlock           | Code to try   |
| 0x74         | Try to init code change | Factory code  |
| 0x75         | End code change process | New code      |
| 0x78         | Boot                    | 00000 (null)  |


But what append if you directly send a new code signal, skiping factory code step...

... the code is overwrited !!! So we can reset code without knowing it.


It mind that we can open every Sentry Safe & Master lock electronic safe in a second.

### Poc

As a Proof of Conncept, I will build a pocket tool.

I take a Atmega328, 8bits AVR microcontroller at 8 MHz (without crystal). After flashing program in his flash, I solder two CR2032 batteries, a button and two pins.

Program will just send a reset signal to set a dummy code, then send an unlock signal to open safe.

And I put all of this in a pen... to have a real pen-test tool.

🎉

![Hardware](images/hardware.png)

![Pen-test tool](images/pen.png)

How to use it :

- Remove tiny screw in front of safe
- Touch black and green wires with two pins of our pen
- Press button
- Safe is open !

### Demo

![Demo](images/demo.gif)

### Fix

To fix this vulnerability, [check my other repository](https://github.com/H4ckd4ddy/fix-sentry-safe)

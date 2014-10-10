
    digitalWrite(pin, LOW);
    chThdSleepMilliseconds(DOTLENGTH);
}

void dash(int pin) {
    // Make a dash signal
    digitalWrite(pin, HIGH);
    chThdSleepMilliseconds(3*DOTLENGTH);
    digitalWrite(pin, LOW);
    chThdSleepMilliseconds(DOTLENGTH);
}

void letterBreak() {
    // The break between letters is 3x(dot length).
    // One dot length comes automatically at the end of
    // the previous dot or dash, so 2 more are needed.
    chThdSleepMilliseconds(2*DOTLENGTH);
}

void wordBreak() {
    // The break between words is 4x(dot length).
    // Three come for free since a letter just ended,
    // so 4 more are needed.
    chThdSleepMilliseconds(4*DOTLENGTH);
}

void send(char letter, int pin) {
    // Sends the given letter to the output
    switch (letter) {
        case 'A': 
        case 'a': dot( pin ); dash( pin ); break;
        case 'B':
        case 'b': dash( pin ); dot( pin ); dot( pin ); dot( pin ); break;
        case 'C':
        case 'c': dash( pin ); dot( pin ); dash( pin ); dot( pin ); break;
        case 'D':
        case 'd': dash( pin ); dot( pin ); dot( pin ); break;
        case 'E':
        case 'e': dot( pin ); break;
        case 'F':
        case 'f': dot( pin ); dot( pin ); dash( pin ); dot( pin ); break;
        case 'G':
        case 'g': dash( pin ); dash( pin ); dot( pin ); break;
        case 'H':
        case 'h': dot( pin ); dot( pin ); dot( pin ); dot( pin ); break;
        case 'I':
        case 'i': dot( pin ); dot( pin ); break;
        case 'J':
        case 'j': dot( pin ); dash( pin ); dash( pin ); dash( pin ); break;
        case 'K':
        case 'k': dash( pin ); dot( pin ); dash( pin ); break;
        case 'L':
        case 'l': dot( pin ); dash( pin ); dot( pin ); dot( pin ); break;
        case 'M':
        case 'm': dash( pin ); dash( pin ); break;
        case 'N':
        case 'n': dash( pin ); dot( pin ); break;
        case 'O':
        case 'o': dash( pin ); dash( pin ); dash( pin ); break;
        case 'P':
        case 'p': dot( pin ); dash( pin ); dash( pin ); dot( pin ); break;
        case 'Q':
        case 'q': dash( pin ); dash( pin ); dot( pin ); dash( pin ); break;
        case 'R':
        case 'r': dot( pin ); dash( pin ); dot( pin ); break;
        case 'S':
        case 's': dot( pin ); dot( pin ); dot( pin ); break;
        case 'T':
        case 't': dash( pin ); break;
        case 'U':
        case 'u': dot( pin ); dot( pin ); dash( pin ); break;
        case 'V':
        case 'v': dot( pin ); dot( pin ); dot( pin ); dash( pin ); break;
        case 'W':
        case 'w': dot( pin ); dash( pin ); dash( pin ); break;
        case 'X':
        case 'x': dash( pin ); dot( pin ); dot( pin ); dash( pin ); break;
        case 'Y':
        case 'y': dash( pin ); dot( pin ); dash( pin ); dash( pin ); break;
        case 'Z':
        case 'z': dash( pin ); dash( pin ); dot( pin ); dot( pin ); break;
        case '0': dash( pin ); dash( pin ); dash( pin ); dash( pin ); dash( pin ); break;
        case '1': dot( pin ); dash( pin ); dash( pin ); dash( pin ); dash( pin ); break;
        case '2': dot( pin ); dot( pin ); dash( pin ); dash( pin ); dash( pin ); break;
        case '3': dot( pin ); dot( pin ); dot( pin ); dash( pin ); dash( pin ); break;
        case '4': dot( pin ); dot( pin ); dot( pin ); dot( pin ); dash( pin ); break;
        case '5': dot( pin ); dot( pin ); dot( pin ); dot( pin ); dot( pin ); break;
        case '6': dash( pin ); dot( pin ); dot( pin ); dot( pin ); dot( pin ); break;
        case '7': dash( pin ); dash( pin ); dot( pin ); dot( pin ); dot( pin ); break;
        case '8': dash( pin ); dash( pin ); dash( pin ); dot( pin ); dot( pin ); break;
        case '9': dash( pin ); dash( pin ); dash( pin ); dash( pin ); dot( pin ); break;
        case ',': dash( pin ); dash( pin ); dot( pin ); dot( pin ); dash( pin ); dash( pin ); break;
        case '.': dot( pin ); dash( pin ); dot( pin ); dash( pin ); dot( pin ); dash( pin ); break;
        case '?': dot( pin ); dot( pin ); dash( pin ); dash( pin ); dot( pin ); dot( pin ); break;
        case ';': dash( pin ); dot( pin ); dash( pin ); dot( pin ); dash( pin ); dot( pin ); break;
        case '\'': dash( pin ); dash( pin ); dash( pin ); dot( pin ); dot( pin ); dot( pin ); break;
        case '-': dash( pin ); dot( pin ); dot( pin ); dot( pin ); dot( pin ); dash( pin ); break;
        case '/': dash( pin ); dot( pin ); dot( pin ); dash( pin ); dot( pin ); break;
        case '(': 
        case ')': dash( pin ); dot( pin ); dash( pin ); dash( pin ); dot( pin ); dash( pin ); break;
        case '_': dot( pin ); dot( pin ); dash( pin ); dash( pin ); dot( pin ); dash( pin ); dash( pin ); break;
        case ' ': wordBreak(); break;
    }
    letterBreak();
}

//------------------------------------------------------------------------------
// thread 1 - print idle loop count every second
// 200 byte stack beyond task switch and interrupt needs
static WORKING_AREA(waThread1, 200);
static msg_t Thread1(void *arg) {
  Serial.begin(9600);
  while (TRUE) {
    Serial.println(count);
    chThdSleepMilliseconds(1000);
  }
  return 0;
}

//------------------------------------------------------------------------------
// thread 2 - high priority for blinking LED
// 64 byte stack beyond task switch and interrupt needs
static WORKING_AREA(waThread2, 8);
static msg_t Thread2(void *arg) 
{
  
  int outpin = 2;
  pinMode(outpin, OUTPUT);

  while(1)
  {
    // Make a dot signal
    digitalWrite(outpin, HIGH);
    chThdSleepMilliseconds( 100 );
    digitalWrite(outpin, LOW);
    chThdSleepMilliseconds( 100 );
  }
}


//------------------------------------------------------------------------------
// thread 3 - high priority for blinking LED
// 64 byte stack beyond task switch and interrupt needs
static WORKING_AREA(waThread3, 8);
static msg_t Thread3(void *arg) 
{
  
  int outpin = 3;
  pinMode(outpin, OUTPUT);

  while(1)
  {
    // Make a dot signal
    digitalWrite(outpin, HIGH);
    chThdSleepMilliseconds( 110 );
    digitalWrite(outpin, LOW);
    chThdSleepMilliseconds( 790 );
  }
}

//------------------------------------------------------------------------------
// thread 4 - high priority for blinking LED
// 64 byte stack beyond task switch and interrupt needs
static WORKING_AREA(waThread4, 8);
static msg_t Thread4(void *arg) 
{
  
  int outpin = 4;
  pinMode(outpin, OUTPUT);

  while(1)
  {
    // Make a dot signal
    digitalWrite(outpin, HIGH);
    chThdSleepMilliseconds( 1220 );
    digitalWrite(outpin, LOW);
    chThdSleepMilliseconds( 800 );
  }
}

//------------------------------------------------------------------------------
// thread 5 - high priority for blinking LED
// 64 byte stack beyond task switch and interrupt needs
static WORKING_AREA(waThread5, 8);
static msg_t Thread5(void *arg) 
{
  
  int outpin = 5;
  pinMode(outpin, OUTPUT);

  while(1)
  {
    // Make a dot signal
    digitalWrite(outpin, HIGH);
    chThdSleepMilliseconds( 830 );
    digitalWrite(outpin, LOW);
    chThdSleepMilliseconds( 270 );
  }
}


//------------------------------------------------------------------------------
// thread 6 - high priority for blinking LED
// 64 byte stack beyond task switch and interrupt needs
static WORKING_AREA(waThread6, 8);
static msg_t Thread6(void *arg) 
{
  
  int outpin = 6;
  pinMode(outpin, OUTPUT);

  while(1)
  {
    // Make a dot signal
    digitalWrite(outpin, HIGH);
    chThdSleepMilliseconds( 340 );
    digitalWrite(outpin, LOW);
    chThdSleepMilliseconds( 60 );
  }
}


//------------------------------------------------------------------------------
// thread 7 - high priority for blinking LED
// 64 byte stack beyond task switch and interrupt needs
static WORKING_AREA(waThread7, 8);
static msg_t Thread7(void *arg) 
{
  
  int outpin = 7;
  pinMode(outpin, OUTPUT);
  char message[] = "This is a test, this is only a test";

  while(1)
  {
    chThdSleepMilliseconds( 2000 );    // three second delay before (and between).

    for (byte j=0; j<sizeof(message); j++) 
    {    
        send(message[j], outpin); // Send each character in the message.
    }
  }
}

//------------------------------------------------------------------------------
void setup() {
  // initialize ChibiOS with interrupts disabled
  // ChibiOS will enable interrupts
  cli();
  halInit();
  chSysInit();
  
  // start print thread
  chThdCreateStatic(waThread1, sizeof(waThread1),
    NORMALPRIO + 1, Thread1, NULL);

  // start blink thread
  chThdCreateStatic(waThread2, sizeof(waThread2),
    NORMALPRIO + 1, Thread2, NULL);

  // start blink thread
  chThdCreateStatic(waThread3, sizeof(waThread3),
    NORMALPRIO + 2, Thread3, NULL);
    
  // start blink thread
  chThdCreateStatic(waThread4, sizeof(waThread4),
    NORMALPRIO + 2, Thread4, NULL);
    
  // start blink thread
  chThdCreateStatic(waThread5, sizeof(waThread5),
    NORMALPRIO + 2, Thread5, NULL);
    
  // start blink thread
  chThdCreateStatic(waThread6, sizeof(waThread6),
    NORMALPRIO + 2, Thread6, NULL);
    
  // start blink thread
  chThdCreateStatic(waThread7, sizeof(waThread7),
    NORMALPRIO + 2, Thread7, NULL);
    
----------------------------------

  // start morse code thread
  chThdCreateStatic(waThread13, sizeof(waThread13),
    NORMALPRIO + 2, Thread13, NULL);
    
  // start morse code thread
  chThdCreateStatic(waThread14, sizeof(waThread14),
    NORMALPRIO + 2, Thread14, NULL);
    
  // start morse code thread
  chThdCreateStatic(waThread15, sizeof(waThread15),
    NORMALPRIO + 2, Thread15, NULL);


}
//------------------------------------------------------------------------------
// idle loop runs at NORMALPRIO
void loop() {
  // must insure increment is atomic
  // in case of context switch for print
  ATOMIC_BLOCK(ATOMIC_FORCEON) {
    count++;
  }
}


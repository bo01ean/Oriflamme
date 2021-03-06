/*
IR ErektaWorm Controller
*/

#include <SPI.h>
#include <String.h>
#include <IRremote.h>
int RECV_PIN = 2;

IRrecv irrecv(RECV_PIN);
decode_results results;

boolean isOn = false;
boolean isRunningSequence = false;

int relayBankSize = 8;
int delayMultiple = 100  ;

boolean selenoidStates[8] = {0, 0, 0, 0, 0, 0, 0, 0};
boolean incoming = false;
boolean ON = LOW;
boolean OFF = HIGH;
boolean HOLDING = false;

int holdTimeout = 80;// amount of ms before we care about it being off or not. aka minum burst

String sequence = "$#1#2#3#4#5!3*";
 
String sequences[10];

boolean STATE = 0;
long lastPress = 0;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
 // Serial.print("ARMING ... in 5 seconds ... ");
  while (!Serial) {} // Wait for Serial to wake up and have some Cereal
  initializeRelays();
 // setSequences();
  irrecv.enableIRIn(); // Start the receiver
  Serial.print("ARMING ... in 5 seconds ... ");
  //sequenceListener();
  arm();
}

void arm() {
  digitalWrite(getRelayPos(relayBankSize), LOW);
  Serial.println("ARMED.");
}

void loop() {  
  if (irrecv.decode(&results)) { // THE BURSTS
    parseIRCode();
  } else {
    if(((millis() - lastPress) 
      > holdTimeout)
      ) { // DONE BURSTING
      HOLDING = false;
      closeAllSelenoids();
      sequenceListener();
      
      
   /*  switchAllRelays(1);
     delay(100);
     switchAllRelays(0);
     delay(100);*/
    }
  }
}

void closeAllSelenoids() {
  for (int i = 1; i <= relayBankSize - 1; i++) {
    ////////Serial.print(selenoidStates[i]);
    if (selenoidStates[i] == ON
        && !HOLDING
       ) {
      //////Serial.print("CLOSING SELENOID: ");
      ////Serial.println(i);
      toggleRelay(i, OFF);
    }
  }
}

void parseIRCode() {
  
  lastPress = millis();
//  ////Serial.println(mapHexToSelenoid(results.value));
  ////Serial.print("parse ");
  ////Serial.println(results.value, HEX);

  HOLDING = false;

  if (results.value == 0xFFFFFFFF) {
    ////Serial.print(" HOLD: ");
    HOLDING = true;
  }
  
  if (results.value == 0x9CB47) {
    ////Serial.print(" UP : Selenoid 3");
    toggleRelay(3, ON);
  }

  if (results.value == 0x5CB47) {
    ////Serial.print(" DOWN : Selenoid 5");
    toggleRelay(5, ON);
  }
  if (results.value == 0xDCB47) {
    ////Serial.print(" LEFT : Selenoid 2");
    toggleRelay(2, ON);
  }
  if (results.value == 0x3CB47) {
    ////Serial.print(" RIGHT : Selenoid 4");
    toggleRelay(4, ON);
  }
  if (results.value == 0xBCB47) {
    ////Serial.print(" CENTER : Selenoid 1");
    toggleRelay(1, ON);
  }
  if (results.value == 0xB47) {
    ////Serial.print(" PROG 1");
    sequence = sequences[0];
  }
  if (results.value == 0x80B47) {
    ////Serial.print(" PROG 2");
    sequence = sequences[1];
  }
  if (results.value == 0x40B47) {
    ////Serial.print(" PROG 3");
    sequence = sequences[2];
  }
  if (results.value == 0xC0B47) {
    ////Serial.print(" PROG 4");
    sequence = sequences[3];
  }
  if (results.value == 0x20B47) {
    ////Serial.print(" PROG 5");
    sequence = sequences[4];    
  }
  if (results.value == 0xA0B47) {
    ////Serial.print(" PROG 6");
    sequence = sequences[5];    
  }
  if (results.value == 0x60B47) {
    ////Serial.print(" PROG 7");
    sequence = sequences[6];    
  }
  if (results.value == 0xE0B47) {
    ////Serial.print(" PROG 8");
    sequence = sequences[7];    
  }
  if (results.value == 0x10B47) {
    ////Serial.print(" PROG 9");
    sequence = sequences[8];    
  }  
  if (results.value == 0xF0B47) {
    ////Serial.print(" PROG 10");
    sequence = sequences[9];    
  }  
  if (results.value == 0x90B47) {
    ////Serial.print(" PROG 11");
    sequence = sequences[10];    
  }  
  if (results.value == 0xEAB47) {
    ////Serial.print(" PROG 12");
    sequence = sequences[11];    
  } 

  irrecv.resume(); // Receive the next value
}

void sequenceListener() {
  if (sequence != "")
  {
    parseSequence();
    sequence = "";
  }
}

void parseSequence() {
  int seqCount = getSeqCount(sequence, '*');
  String seqs[seqCount];
  setSequence(sequence, seqs, seqCount, '*');

  for (int i = 0; i <  seqCount; i++) {
    int blastSegCount = getSeqCount(seqs[i], '!');
    String blast[blastSegCount];
    setSequence(seqs[i], blast, blastSegCount, '!');

    int blastDelay = blast[1].toInt() * delayMultiple;
    
     ////Serial.print("HERE IS WHERE THE DELAY IS:");
     ////Serial.println(blastDelay);
 

    int relaySeqCount = getSeqCount(blast[0], '#');
    String relaysToFire[relaySeqCount];
    setSequence(blast[0], relaysToFire, relaySeqCount, '#');

    for (int j = 0; j < relaySeqCount; j++) {
      int relayState = isOff(relaysToFire[j].toInt());
      digitalWrite(getRelayPos(abs(relaysToFire[j].toInt())), relayState);
      //digitalWrite(22, HIGH);
     // delay(blastDelay);
      
    }
    delay(blastDelay);
  }
}


int isOff(int relayToFire)
{
  int isOff = 1;

  if (relayToFire > 0) {
    ////Serial.print(relayToFire);
    ////Serial.println(" return 0");
    isOff = 0;
  }
  ////Serial.print(relayToFire);
  ////Serial.println(" return 1");
  return isOff;
}

void listenForSerialSequence() {
  while (Serial.available() > 0) {
    boolean currentLineIsBlank = true;
    char c = Serial.read();

    if (c == ' ' && currentLineIsBlank == true) {
      incoming = 0;
      break;
    }

    if (c == '$') {
      incoming = 1;
      sequence = "";
      continue;
    }

    //Checks for the URL string $1 or $2
    if (incoming == 1) {
      sequence = sequence + c;
    }

    if (c == '\n') {
      currentLineIsBlank = true;
    } else if (c != '\r') {
      currentLineIsBlank = false;
    }
    // give the web browser time to receive the data
    delay(1);
  }
}

void runCycle() {
  int j = 100;

  for (int i = 22; i < 38; i++) {
    digitalWrite(i, LOW);
    delay(j);
    digitalWrite(i, HIGH);

    j = j - 10;

    if (j < 10)
    {
      j = 500;
    }
  }
}

int getSeqCount(String sequence, char delimiter) {
  int seqCount = 0;
  int sequenceLength = sequence.length();
  int lastSeqIndex = sequence.lastIndexOf(delimiter);

  for (int i = 0; i < sequenceLength; i++) {
    if (sequence.charAt(i) == delimiter) {
      seqCount++;
    }
  }

  if (lastSeqIndex + 1 < sequenceLength) {
    seqCount++;
  }

  return seqCount;
}

void setSequence(String sequence, String seqs[], int seqCount, char delimiter) {
  ////Serial.println("------");
  int nextSeqIndex = -1;
  int startSeqIndex = -1;
  int sequenceLength = sequence.length();

  ////Serial.print("Sequence Length: ");
  ////Serial.println(sequenceLength);

  int lastSeqIndex = sequence.lastIndexOf(delimiter);

  ////Serial.print("lastSeqIndex: ");
  ////Serial.println(lastSeqIndex);

  for (int j = 0; j < seqCount; j++) {
    startSeqIndex = nextSeqIndex + 1;

    if (nextSeqIndex == lastSeqIndex) {
      seqs[j] = sequence.substring(startSeqIndex);
    } else {
      nextSeqIndex = startSeqIndex + sequence.substring(startSeqIndex).indexOf(delimiter);
      seqs[j] = sequence.substring(startSeqIndex, nextSeqIndex);
      ////Serial.print("NextSeqIndex: ");
      ////Serial.println(nextSeqIndex);
    }
  }


  for (int k = 0; k < seqCount; k++) {
    ////Serial.println(seqs[k]);
  }

// return seqs;
  //fireSequence(seqs, seqCount);
}

/*void fireSequence(String seqs[], int seqCount) {
  for (int i = 0; i < seqCount; i++) {

    int relay = getRelayPos(seqs[i].substring(seqs[i].indexOf('*') + 1, seqs[i].indexOf('!')).toInt());
    int blastTime = seqs[i].substring(seqs[i].indexOf('!') + 1).toInt() * 150;

    ////Serial.print("Relay:");
    ////Serial.print(relay);
    ////Serial.print("BlastTime:");
    ////Serial.println(blastTime);

    digitalWrite(relay, HIGH);
    delay(blastTime);
    digitalWrite(relay, LOW);
  }
}*/

int getRelayPos(int pos) {
  switch (pos) {
  case 1:
    return 22;
  case 2:
    return 23;
  case 3:
    return 24;
  case 4:
    return 25;
  case 5:
    return 26;
  case 6:
    return 27;
  case 7:
    return 28;
  case 8:
    return 29;
  case 9:
    return 30;
  case 10:
    return 31;
  case 11:
    return 32;
  case 12:
    return 33;
  case 13:
    return 34;
  case 14:
    return 35;
  case 15:
    return 36;
  case 16:
    return 37;
  default:
    return -1;
  }
}

/*
SONY REMOTE
// Selenoids
Sel 3 ‚Äî UP 9CB47
Sel 5 ‚Äî DOWN 5CB47
Sel 2 ‚Äî LEFT DCB47
Sel 4 ‚Äî RIGHT 3CB47
Sel 1 ‚Äî CENTER BCB47
// SEQUENCES
#1 B47
#2 80B47
#3 40B47
#4 C0B47
#5 20B47
#6 A0B47
*/

void initializeRelays() {
  for (int i = 1; i <= relayBankSize; i++) {
    //////Serial.println(i);
    pinMode(getRelayPos(i), OUTPUT);
    digitalWrite(getRelayPos(i), OFF);// HIGH IS OFF...
  }
}

void toggleRelay(int POSITION, int DIRECTION) {
  ////Serial.print("Toggling Selenoid:");
  ////Serial.print(POSITION);
  ////Serial.print(" -> ");
  ////Serial.println(DIRECTION);
  digitalWrite(getRelayPos(POSITION), DIRECTION);
  selenoidStates[POSITION] = DIRECTION;
}

void switchAllRelays(int DIR) {
  for (int i = 1; i <= relayBankSize; i++) {
    toggleRelay(i, DIR);
  }
}

void triggerSelenoidForPeriod(int POSITION, int DIRECTION, int PERIOD) {
  // we don't want to set the selenoid to the same state twice.
  if (selenoidStates[POSITION] == DIRECTION) {
    return;
  } else {
    toggleRelay(POSITION, DIRECTION);
    delay(PERIOD);
    toggleRelay(POSITION, !DIRECTION);
  }
}

/*void setSequences()
{
  sequences[1] = "$#-1#2#3#4#5!3*";
  
  sequences[2] = "$#1#2#3#4#5!3*";
  
  sequences[3] =  "$#-1#2#3#4#5!3*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!3*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!2*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!2*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#2#3#4#5!1*"
                  "$#1#-2#-3#-4#-5!4*";
  
  sequences[4] =  "$#1#2#3#4#5!2*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#2#3#4#5!2*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#2#3#4#5!2*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#-3#-4#-5!2*"
                  "$#-1#-2#3#-4#-5!2*"
                  "$#-1#-2#-3#4#-5!2*"
                  "$#-1#-2#-3#-4#5!2*"
                  "$#-1#2#-3#-4#-5!2*"
                  "$#-1#-2#3#-4#-5!2*"
                  "$#-1#-2#-3#4#-5!2*"
                  "$#-1#-2#-3#-4#5!2*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#2#3#4#5!2*";
  
  sequences[5] =  "$#-1#2#-3#-4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#3#-4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#-4#5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#-2#-3#-4#-5!5*"
                  "$#-1#-2#-3#-4#5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#3#-4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#-3#-4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#-4#-5!10*"
                  "$#-1#2#-3#-4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#3#-4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#-4#5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#-2#-3#-4#-5!5*"
                  "$#-1#-2#-3#-4#5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#3#-4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#-3#-4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#-4#-5!10*"
                  "$#-1#-2#-3#-4#5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#3#-4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#-3#-4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#-2#-3#-4#-5!5*"
                  "$#-1#2#-3#-4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#3#-4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#-4#5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#-4#-5!10*"
                  "$#-1#-2#-3#-4#5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#3#-4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#-3#-4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#-2#-3#-4#-5!1*"
                  "$#-1#2#-3#-4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#3#-4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#4#-5!4*"
                  "$#-1#-2#-3#-4#-5!4*"
                  "$#-1#-2#-3#-4#5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#-4#-5!10*";
  
  sequences[6] =  "$#-1#2#-3#-4#-5!4*"
                  "$#-1#-2#-3#-4#-5!3*"
                  "$#-1#-2#3#-4#-5!4*"
                  "$#-1#-2#-3#-4#-5!3*"
                  "$#-1#-2#-3#4#-5!4*"
                  "$#-1#-2#-3#-4#-5!3*"
                  "$#-1#-2#-3#-4#5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#-2#-3#-4#-5!4*"
                  "$#-1#2#-3#-4#-5!3*"
                  "$#-1#-2#-3#-4#-5!2*"
                  "$#-1#-2#3#-4#-5!3*"
                  "$#-1#-2#-3#-4#-5!2*"
                  "$#-1#-2#-3#4#-5!3*"
                  "$#-1#-2#-3#-4#-5!2*"
                  "$#-1#-2#-3#-4#5!3*"
                  "$#1#-2#-3#-4#-5!2*"
                  "$#-1#-2#-3#-4#-5!10*"
                  "$#-1#2#-3#-4#-5!2*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#3#-4#-5!2*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#4#-5!2*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#-4#5!2*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#-2#-3#-4#-5!4*"
                  "$#-1#2#-3#-4#-5!1*"
                  "$#-1#-2#-3#-4#-5!2*"
                  "$#-1#-2#3#-4#-5!1*"
                  "$#-1#-2#-3#-4#-5!2*"
                  "$#-1#-2#-3#4#-5!1*"
                  "$#-1#-2#-3#-4#-5!2*"
                  "$#-1#-2#-3#-4#5!1*"
                  "$#1#-2#-3#-4#-5!2*"
                  "$#-1#-2#-3#-4#-5!10*"
                  "$#-1#-2#-3#-4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#4#-5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#3#-4#-5!2*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#-3#-4#-5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#-4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#4#-5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#3#-4#-5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#-3#-4#-5!1*"
                  "$#1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#-4#-5!10*"
                  "$#-1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#-2#-3#-4#-5!4*"
                  "$#-1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!1*"
                  "$#1#-2#-3#-4#-5!4*"
                  "$#-1#-2#-3#-4#-5!10*";
                    
  sequences[7] = "$#-1#2#-3#4#-5!4*"
                  "$#-1#-2#-3#-4#-5!3*"
                  "$#-1#-2#3#-4#5!4*"
                  "$#-1#-2#-3#-4#-5!3*"
                  "$#-1#2#-3#4#-5!4*"
                  "$#-1#-2#-3#-4#-5!3*"
                  "$#-1#-2#3#-4#5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#2#3#4#5!4*"
                  "$#-1#-2#-3#-4#-5!3*"
                  "$#-1#-2#-3#-4#-5!2*"
                  "$#-1#2#3#-4#-5!3*"
                  "$#-1#-2#-3#-4#-5!2*"
                  "$#-1#-2#-3#4#5!3*"
                  "$#-1#-2#-3#-4#-5!2*"
                  "$#-1#2#3#-4#-5!3*"
                  "$#1#-2#-3#-4#-5!4*"
                  "$#-1#-2#-3#-4#-5!5*"
                  "$#-1#2#3#4#5!2*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!2*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#-3#-4#-5!2*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#3#-4#-5!2*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#-2#-3#4#-5!2*"
                  "$#-1#2#-3#-4#5!1*"
                  "$#-1#-2#-3#-4#-5!2*"
                  "$#-1#-2#3#-4#-5!1*"
                  "$#-1#-2#-3#-4#-5!2*"
                  "$#-1#-2#-3#4#-5!1*"
                  "$#-1#-2#-3#-4#-5!2*"
                  "$#-1#-2#-3#-4#5!1*"
                  "$#1#-2#-3#-4#-5!3*"
                  "$#-1#-2#-3#-4#-5!5*"
                  "$#-1#-2#-3#-4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#4#-5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#3#-4#-5!2*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#-3#-4#-5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#-4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#4#-5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#3#-4#-5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#-3#-4#-5!1*"
                  "$#1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#-4#-5!10*"
                  "$#-1#2#3#-4#-5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#3#4#-5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#4#5!1*"
                  "$#-1#-2#3#4#-5!1*"
                  "$#-1#2#3#-4#-5!1*"
                  "$#1#-2#-3#-4#-5!3*"
                  "$#-1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#-2#-3#-4#-5!5*"
                  "$#-1#-2#-3#-4#-5!10*";
  
  sequences[8] =  "$#1#-2#-3#-4#-5!2*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#-2#-3#-4#-5!2*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#-2#-3#-4#-5!2*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#-2#-3#-4#-5!2*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#-2#-3#-4#-5!2*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#-2#-3#-4#-5!2*"
                  "$#-1#-2#-3#-4#-5!5*"
                  "$#-1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!1*"
                  "$#1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!1*"
                  "$#1#-2#-3#-4#-5!3*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!5*"
                  "$#-1#2#-3#-4#-5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#-3#-4#-5!1*"
                  "$#-1#-2#3#-4#-5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#3#-4#-5!1*"
                  "$#-1#-2#-3#4#-5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#4#-5!1*"
                  "$#-1#-2#-3#-4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#-4#5!1*"
                  "$#1#-2#-3#-4#-5!4*"
                  "$#-1#2#-3#4#-5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#-3#4#-5!1*"
                  "$#-1#-2#3#-4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#3#-4#5!1*"
                  "$#-1#2#-3#4#-5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#-3#4#-5!1*"
                  "$#-1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!1*"
                  ""
                  "$#-1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#2#3#4#5!1*"
                  "$#1#-2#-3#-4#-5!1*"
                  "$#1#2#-3#4#-5!1*"
                  "$#1#-2#-3#-4#-5!1*"
                  "$#1#-2#3#-4#5!1*"
                  "$#1#-2#-3#-4#-5!1*"
                  "$#1#-2#3#-4#5!1*"
                  "$#1#-2#-3#-4#-5!4*";
  
  sequences[9] =  "$#-1#-2#-3#-4#5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#3#-4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#-3#-4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#-2#-3#-4#-5!5*"
                  "$#-1#2#-3#-4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#3#-4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#-4#5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#-4#-5!10*"
                  "$#-1#-2#-3#-4#5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#3#-4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#-3#-4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#-2#-3#-4#-5!1*"
                  "$#-1#2#-3#-4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#3#-4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#4#-5!4*"
                  "$#-1#-2#-3#-4#-5!4*"
                  "$#-1#-2#-3#-4#5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#-2#-3#-4#-5!10*"
                  "$#1#2#-3#4#-5!3*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#-2#3#-4#5!3*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!6*"
                  "$#1#-2#-3#-4#-5!6*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#-3#4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#-2#-3#-4#-5!4*"
                  "$#1#2#-3#4#-5!3*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#-2#3#-4#5!3*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!6*"
                  "$#1#-2#-3#-4#-5!6*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#3#4#5!1*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#-1#2#-3#4#-5!4*"
                  "$#-1#-2#-3#-4#-5!1*"
                  "$#1#-2#-3#-4#-5!4*";
}*/





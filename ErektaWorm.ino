/*
IR ErektaWorm Controller
*/

#include <SPI.h>
#include <String.h>
#include <IRremote.h>
int RECV_PIN = 23;

IRrecv irrecv(RECV_PIN);
decode_results results;

boolean isOn = false;
boolean isRunningSequence = false;

int relayBankSize = 8;

boolean selenoidStates[8] = {0,0,0,0,0,0,0,0};
boolean incoming = false;
boolean ON = LOW;
boolean OFF = HIGH;

//String sequence = "
/*
$1#2#3#4#5#-6#-7#-8#-9#-10#-11#-12#-13#-14#-15#-16!1*
1#-2#3#4#-5#-6#-7#-8#-9#-10#-11#-12#-13#-14#-15#-16!1*
1#2#3#-4#-5#-6#-7#-8#-9#-10#-11#-12#-13#-14#-15#-16!1*
1#-2#-3#-4#-5#-6#-7#-8#-9#-10#-11#-12#-13#-14#-15#-16!1*
1#2#-3#-4#-5#-6#-7#-8#-9#-10#-11#-12#-13#-14#-15#-16!1*
1#2#3#4#5#-6#-7#-8#-9#-10#-11#-12#-13#-14#-15#-16!1*
1#-2#3#4#-5#-6#-7#-8#-9#-10#-11#-12#-13#-14#-15#-16!
1*1#2#3#-4#-5#-6#-7#-8#-9#-10#-11#-12#-13#-14#-15#-16!
1*1#-2#-3#-4#-5#-6#-7#-8#-9#-10#-11#-12#-13#-14#-15#-16!
1*1#2#-3#-4#-5#-6#-7#-8#-9#-10#-11#-12#-13#-14#-15#-16!
1*1#2#3#4#5#-6#-7#-8#-9#-10#-11#-12#-13#-14#-15#-16!
1*1#-2#3#4#-5#-6#-7#-8#-9#-10#-11#-12#-13#-14#-15#-16!
1*1#2#3#-4#-5#-6#-7#-8#-9#-10#-11#-12#-13#-14#-15#-16!
1*1#-2#-3#-4#-5#-6#-7#-8#-9#-10#-11#-12#-13#-14#-15#-16!
1*1#2#-3#-4#-5#-6#-7#-8#-9#-10#-11#-12#-13#-14#-15#-16!
1*1#2#3#4#5#-6#-7#-8#-9#-10#-11#-12#-13#-14#-15#-16!
1*1#-2#3#4#-5#-6#-7#-8#-9#-10#-11#-12#-13#-14#-15#-16!
1*1#2#3#-4#-5#-6#-7#-8#-9#-10#-11#-12#-13#-14#-15#-16!
1*1#-2#-3#-4#-5#-6#-7#-8#-9#-10#-11#-12#-13#-14#-15#-16!
1*1#2#-3#-4#-5#-6#-7#-8#-9#-10#-11#-12#-13#-14#-15#-16!1*";
*/

//String sequence = "$1#2#3#4#5#6#7#8#9#10#11#12#13#14#15#16#!50*1#-2#-3#-4#-5#-6#-7#-8#-9#-10#-11#-12#-13#-14#-15#-16#-!1*";
String sequence = "";
//1 //3 //7
//30 //32 //36


void setup() { 
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while(!Serial){}// Wait for Serial to wake up and have some Cereal
  initializeRelays();
  irrecv.enableIRIn(); // Start the receiver
  Serial.print("ARMING ... in 5 seconds ... ");
  delay(5 * 1000);
  arm();
}


void arm() {
  digitalWrite(getRelayPos(relayBankSize), LOW);
  Serial.println("ARMED.");
}


void loop() {
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    parseIRCode();
    irrecv.resume(); // Receive the next value
  } else {
    closeAllSelenoids();            
    if(sequence != "") {
      //close all selenoids
    }
  }
}

void looper()
{
//  listenForRFSequence();
  if(sequence != "")
  {
   int seqCount = getSeqCount(sequence, '*');
   String seqs[seqCount];
   setSequence(sequence, seqs, seqCount, '*');
   
   for(int i=0; i <  seqCount; i++)
   {
     int blastSegCount = getSeqCount(seqs[i], '!');
     String blast[blastSegCount];
       setSequence(seqs[i], blast, blastSegCount, '!');
        
     int blastDelay = blast[1].toInt() * 100;
     
     int relaySeqCount = getSeqCount(blast[0], '#');  
     String relaysToFire[relaySeqCount];
     setSequence(blast[0], relaysToFire, relaySeqCount, '#');

     for(int j=0; j < relaySeqCount; j++)
     {
       int relayState = isOff(relaysToFire[j].toInt());
       //Serial.print("abs relayToFire: ");
       //Serial.println(abs(relaysToFire[j].toInt()));
       digitalWrite(getRelayPos(abs(relaysToFire[j].toInt())), relayState);
       digitalWrite(22, HIGH);
     }
     delay(blastDelay);
    }
    sequence = "";
  }
}

int isOff(int relayToFire)
{
  int isOff = 1;
  
  if(relayToFire > 0)
  {
    Serial.print(relayToFire);
    Serial.println(" return 0");
    isOff = 0;
  }
  Serial.print(relayToFire);  
  Serial.println(" return 1");
  return isOff;
}

void listenForSerialSequence()
{ 
  while(Serial.available() > 0) 
  {
    boolean currentLineIsBlank = true;
    char c = Serial.read();

    if(c == ' ' && currentLineIsBlank == true)
    { 
      incoming = 0;
      break;
    }
    
    if(c == '$')
    { 
      incoming = 1; 
      sequence = "";
      continue;
    }
    
    //Checks for the URL string $1 or $2
    if(incoming == 1)
    {
      sequence = sequence + c;
    }

    if (c == '\n') 
    {
     currentLineIsBlank = true;
    } 
    else if (c != '\r') {
      currentLineIsBlank = false;
    }
    
    // give the web browser time to receive the data
    delay(1);
  }
}

void runCycle()
{
 int j = 100;
 
 for(int i=22; i < 38; i++)
 {
  digitalWrite(i, LOW);
  delay(j);
  digitalWrite(i, HIGH);
  
  j = j-10;
  
  if(j < 10)
  {
    j = 500;
  }
 } 
}

int getSeqCount(String sequence, char delimiter)
{
  int seqCount = 0;
  int sequenceLength = sequence.length();
  int lastSeqIndex = sequence.lastIndexOf(delimiter);
  
  for(int i=0; i < sequenceLength; i++)
  {
    if(sequence.charAt(i) == delimiter)
    {
      seqCount++; 
    }
  }
  
  if(lastSeqIndex + 1 < sequenceLength)
  {
     seqCount++; 
  }
  
  return seqCount;
}

void setSequence(String sequence, String seqs[], int seqCount, char delimiter)
{
  Serial.println("------");
  int nextSeqIndex = -1;
  int startSeqIndex = -1;
  int sequenceLength = sequence.length();
  
  Serial.print("Sequence Length: ");
  Serial.println(sequenceLength);
  
  int lastSeqIndex = sequence.lastIndexOf(delimiter);
  
  Serial.print("lastSeqIndex: ");
  Serial.println(lastSeqIndex);
  
  for(int j=0; j < seqCount; j++)
  {
    startSeqIndex = nextSeqIndex + 1;
   
    if(nextSeqIndex == lastSeqIndex)
    {
      seqs[j] = sequence.substring(startSeqIndex);
    }
    else
    {
      nextSeqIndex = startSeqIndex + sequence.substring(startSeqIndex).indexOf(delimiter);
      seqs[j] = sequence.substring(startSeqIndex,nextSeqIndex);
      Serial.print("NextSeqIndex: ");
      Serial.println(nextSeqIndex);
    }
  }  
  
  
  for(int k =0; k < seqCount; k++)
  {
    Serial.println(seqs[k]);  
  }
  
 // return seqs;
  //fireSequence(seqs, seqCount);
}

void fireSequence(String seqs[], int seqCount)
{
 for(int i=0; i < seqCount; i++)
 {
  
  int relay = getRelayPos(seqs[i].substring(seqs[i].indexOf('*') + 1, seqs[i].indexOf('!')).toInt());
  int blastTime = seqs[i].substring(seqs[i].indexOf('!') + 1).toInt() * 150;
  
  Serial.print("Relay:");
  Serial.print(relay);
  Serial.print("BlastTime:");
  Serial.println(blastTime);
   
  digitalWrite(relay, HIGH);
  delay(blastTime);
  digitalWrite(relay, LOW);
 } 
}

int getRelayPos(int pos)
{
 switch (pos) 
 {
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
Sel 3 — UP 9CB47
Sel 5 — DOWN 5CB47
Sel 2 — LEFT DCB47
Sel 4 — RIGHT 3CB47
Sel 1 — CENTER BCB47
// SEQUENCES
#1 B47
#2 80B47
#3 40B47
#4 C0B47
#5 20B47
#6 A0B47

*/

void fireSelenoidUntilButtonReelase() {
  
}

void parseIRCode() {
   if(results.value==0xFFFFFFFF) {
      Serial.println("HOLDING: ");
   }
  //Serial.println(results.value, HEX);
   if(results.value==0x9CB47){
     Serial.println("UP : Selenoid 3");
              toggleRelay(3, ON);
   }
   if(results.value==0x5CB47){
     Serial.println("DOWN : Selenoid 5");
              toggleRelay(5, ON);
   }
   if(results.value==0xDCB47){
     Serial.println("LEFT : Selenoid 2");
              toggleRelay(2, ON);
   }
   if(results.value==0x3CB47){
     Serial.println("RIGHT : Selenoid 4");
              toggleRelay(4, ON);
   }
   if(results.value==0xBCB47){
     Serial.println("CENTER : Selenoid 1");
              toggleRelay(1, ON);
   }
   if(results.value==0xB47){
     Serial.println("PROG 1");
   }
   if(results.value==0x80B47){
     Serial.println("PROG 2");
   }
   if(results.value==0x40B47){
     Serial.println("PROG 3");
   }
   if(results.value==0xC0B47){
     Serial.println("PROG 4");
   }
   if(results.value==0x20B47){
     Serial.println("PROG 5");
   }
   if(results.value==0xA0B47){
     Serial.println("PROG 6");
   }
   if(results.value==0xff10EF){
     Serial.println("7");
   }
   if(results.value==0xff38C7){
     Serial.println("8");
   }
   if(results.value==0xff5AA5){
     Serial.println("9");
   }
   if(results.value==0xff4AB5){
     Serial.println("0");
   }
   if(results.value==0xff42BD){
     Serial.println("*");
   }
   if(results.value==0xff52AD){
     Serial.println("#");
   }
    irrecv.resume(); // Receive the next value
}

void initializeRelays() {
   for(int i=1; i<=relayBankSize; i++) {
    //Serial.println(i);
     pinMode(getRelayPos(i), OUTPUT);
     digitalWrite(getRelayPos(i), OFF);// HIGH IS OFF...
   }
}

void toggleRelay(int POSITION, int DIRECTION) {
  Serial.print("Toggling Selenoid:");
  Serial.print(POSITION);
  Serial.print(" -> ");
  Serial.println(DIRECTION);
  digitalWrite(getRelayPos(POSITION), DIRECTION);
  selenoidStates[POSITION] = DIRECTION;
}

void closeAllSelenoids() {
  for(int i=1; i<=relayBankSize-1; i++) {
    Serial.print(selenoidStates[i]);
    Serial.println(" ");
    if(selenoidStates[i] == ON) {
      Serial.print("CLOSING SELENOID: ");
      Serial.println(i);
      toggleRelay(i, OFF);
    }
  }    
}

void switchAllRelays(int DIR) {
  for(int i=1; i<=relayBankSize; i++) {
    toggleRelay(i, DIR);
  }  
}

void triggerSelenoid(int POSITION, int DIRECTION, int PERIOD) {
  //we don't want to set the selenoid to the same state twice.
  if (selenoidStates[POSITION] == DIRECTION) {
    return;
  } else {
    toggleRelay(POSITION, DIRECTION);
    delay(PERIOD);
    toggleRelay(POSITION, !DIRECTION);
  }
}


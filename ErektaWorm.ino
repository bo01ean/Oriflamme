/*
  WiFi Web Server

 A simple web server that shows the value of the analog input pins.
 using a WiFi shield.

 This example is written for a network using WPA encryption. For
 WEP or WPA, change the Wifi.begin() call accordingly.

 Circuit:
 * WiFi shield attached
 * Analog inputs attached to pins A0 through A5 (optional)

 created 13 July 2010
 by dlf (Metodo2 srl)
 modified 31 May 2012
 by Tom Igoe

 */

#include <SPI.h>
#include <WiFi.h>
#include <String.h>

boolean isOn = false;

char ssid[] = "NETGEAR86";      // your network SSID (name)
//char ssid[] = "linksys";      // your network SSID (name)
//char pass[] = "EA375176748FBC28862CA50CF6";   // your network password
char pass[] = "rapiddiamond019";   // your network password
int keyIndex = 0;                 // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

boolean incoming = false;

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

String sequence = "$1#2#3#4#5#6#7#8#9#10#11#12#13#14#15#16#!50*1#-2#-3#-4#-5#-6#-7#-8#-9#-10#-11#-12#-13#-14#-15#-16#-!1*";
//1 //3 //7
//30 //32 //36

WiFiServer server(80);

void setup() { 
  //Initialize serial and wait for port to open:
  initializeRelays();
  turnOffAllRelays();
  Serial.begin(9600);
  initializeWIFI();  
}

void initializeWIFI()
{
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if ( fv != "1.1.0" )
    Serial.println("Please upgrade the firmware");

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    printWifiStatus();
    // wait 1 seconds for connection:
    delay(1000);
  }
  server.begin();
  // you're connected now, so print out the status:
  printWifiStatus(); 
}

void loop()
{
  // listen for incoming clients
  listenForWIFISequence();
  //listenForSerialSequence();
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
     
//     Serial.println("Here is the sequence:" + sequence);
//     Serial.print("balst Count: ");
//     Serial.println(blastSegCount);
//     Serial.print("Sequence#: ");
//     Serial.print(i);
//     Serial.print(" Blast#: ");
//     Serial.print(blast[0]);
//     Serial.print(" Delay: ");
//     Serial.println(blast[1]);
//     Serial.print("Relay Count: ");
//     Serial.println(relaySeqCount); 
//     Serial.print("Relay that Fired: ");
     
     for(int j=0; j < relaySeqCount; j++)
     {
       int relayState = isOff(relaysToFire[j].toInt());
       //Serial.print("abs relayToFire: ");
       //Serial.println(abs(relaysToFire[j].toInt()));
       digitalWrite(getRelayPos(abs(relaysToFire[j].toInt())), relayState);
       digitalWrite(22, HIGH);
//       Serial.println(" ");
//       Serial.print(" FIRE--> ");
//       Serial.print(relaysToFire[j].toInt()); 
//       Serial.print(" RelayState: ");
//       Serial.print(relayState);
       
     }

//     Serial.print(" With delay: ");
//     Serial.print(blastDelay);
//     Serial.println(" ");
     
     
     delay(blastDelay);
     
//     for(int k=0; k < relaySeqCount; k++)
//     {
//       digitalWrite(getRelayPos(relaysToFire[k].toInt()), true);
//       Serial.print(" OFF--> ");
//       Serial.print(relaysToFire[k].toInt()); 
//     }
     
    //Serial.println("");
      
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

void listenForWIFISequence()
{
  WiFiClient client = server.available();
  
  while(client) 
  {
    boolean currentLineIsBlank = true;
    
    if(client.connected()) 
    {
      if (client.available()) 
      {
        char c = client.read();
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        //reads URL string from $ to first blank space
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
      }
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
   
  digitalWrite(relay, LOW);
  delay(blastTime);
  digitalWrite(relay, HIGH);
 } 
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
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

void initializeRelays()
{
   for(int i=1; i<=16; i++)
  {
    pinMode(getRelayPos(i), OUTPUT);
  }
}

void turnOffAllRelays()
{
  for(int i=1; i<=16; i++)
  {
    digitalWrite(getRelayPos(i), HIGH);
  }
}


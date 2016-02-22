/* 

This is a example sketch for the CC1101Radio library

The sketch will:

    * send a broadcast message at 2000 ms intervals
    * receive all messages on this freqency, network/sync word and channel
    * print send/receive events on serial
    * publish (if Photon board) at 5000 ms intervals
        How many packages send and received
        - every 4 times, a list of active devices (cc1101 addresses), will be published instead


The sketch is testet (not intensly) using a:
    * Photon board and firmware 4.09
    * Arduino Due board
    * Arduino Mini 3.3v
    
    and CC1101 running at 433 MHZ
    
There are some stability issues when used with 5v boards, could be a need for level-shifting    
    

The CC1101Radio library is:
    * work in progress and will be changed frequently
    * Aiming at implemented a simpel protocol (not used in the sketch and be founddation for a simpel network layer
    * free to use at your own risk and cost


Michael feb. 21 2016

*/


#ifdef ARDUINO
#include "SPI.h"
#endif



// ----------------------
// select functions
// ----------------------

// comment next line if packets should not be send automatically
#define _sendPackets

// comment next line if packets should not be read automatically
#define _readPackets

// comment next line if Photon should not publish automatically
#define _PhotonStuff


#include "CC1101Radio.h"
CC1101Radio cc1101;


// ----------------------
// Stats to publish
// ----------------------

uint16_t packetsSend = 0;
uint16_t packetsReceived = 0;
uint32_t packetsSendBytes = 0;
uint32_t packetsReceivedBytes = 0;

// ----------------------
// Counters and timers
// ----------------------

uint16_t packetNum = 0;
uint32_t lastSendMs = 0;
#define msBetweenSending 2000

uint32_t lastPublishMs = 0;
#define msBetweenPublish 5000

uint8_t publishDevices = 0; // publish a list of devices or a stat of send/receive packets

// ----------------------
// Interrupt procedures
// ----------------------

void cc1101signalsInterrupt(void){
	cc1101.packetAvailable=true;
}

void attachIntr() {
	#if defined(PARTICLE)
	attachInterrupt(cc1101.GDO0pin, cc1101signalsInterrupt, FALLING);
	#else
	attachInterrupt(digitalPinToInterrupt(cc1101.GDO0pin), cc1101signalsInterrupt, FALLING);
	#endif
}

void detachIntr() {
	#if defined(PARTICLE)
	detachInterrupt(cc1101.GDO0pin);
	#else
	detachInterrupt(digitalPinToInterrupt(cc1101.GDO0pin));
	#endif
}

// ----------------------
// Setup() & Loop()
// ----------------------

void setup()
{

	Serial.begin(57600);

	cc1101.init(); // initialize to default setup values

	// change what is needed
	
	// Photon, Arduino Uno, Mini, Mega & Due is given a uniq device Address in CC1101Radio.h
	// If more than one of each board is used, they will get the samme address. I that case change device Address below
	// cc1101.deviceData.deviceAddress = 99;
	#ifdef ARDUINO
		pinMode(9,INPUT);
		if(digitalRead(9)==LOW)
		cc1101.deviceData.deviceAddress++;
		
		Serial.println("\r\nAdded 1 to deviceAddress becourse D9 is connected to ground");
	#endif
	
	
	
	
	// remoteDeviceAddress is set to '0'. This means broadcast to all cc1101 devices
	cc1101.deviceData.remoteDeviceAddress = 0;
	
	// We don't let the CC1101 filter packets for this deviceAddress, we receive everything
	cc1101.deviceData.addressCheck = false;
	
	// Use the same frequency, network and channel for alle CC1101 modules
	cc1101.deviceData.carrierFreq = CFREQ_433;
	cc1101.deviceData.txPower = PA_ShortDistance;
	cc1101.deviceData.channel = 5 ;
	cc1101.deviceData.syncWord[0] = 19;
	cc1101.deviceData.syncWord[1] = 9;

	// write deviceData to cc1101 module
	cc1101.begin();
	
	// start reading the GDO0 pin through the ISR function
	attachIntr();


}


void loop() {
	
	// Check if the CC1101 has a packet
	if(!receivePacket()) {

		// If not packet available, check if we shoud send one
		if(millis() > lastSendMs + msBetweenSending) {
			sendPacket();
			lastSendMs = millis();
		}
	}
	
    #ifdef PARTICLE
    #ifdef _PhotonStuff
	if(millis() > lastPublishMs + msBetweenPublish) {
	    publishStats();
	    lastPublishMs = millis();
	}
    #endif
    #endif
	delay(15);

}

// ----------------------
// Send & Receive
// ----------------------

void sendPacket() {

	#ifdef _sendPackets

	uint8_t dataLength=10;
	
	CC1101Radio::CCPACKET pkt;
	pkt.length = dataLength;
	
	pkt.data[0] = cc1101.deviceData.remoteDeviceAddress;
	pkt.data[1] = cc1101.deviceData.deviceAddress;
	pkt.data[2] = lowByte(packetNum);
	pkt.data[3] = highByte(packetNum);
	
	for(int idx=4;idx<dataLength;idx++) {
		pkt.data[idx] = 65+idx;
	}
	
	// do the actual transmitting
	cc1101.sendData(pkt);

	packetsSend++;
	packetsSendBytes+=pkt.length;

	Serial.print("\r\nSend packet #");
	Serial.print(packetNum);
	Serial.print(" -");
	cc1101.printCCPACKETdata(&pkt);
	packetNum++;

	#endif
}

bool receivePacket() {

	if(!cc1101.packetAvailable) {
		return false;
	}

	// The cc1101 has received a package
	detachIntr();
	cc1101.packetAvailable = false;

	CC1101Radio::CCPACKET pkt;

	// read the packet
	if(cc1101.receiveData(&pkt)) {
		
		
	#ifdef _readPackets

		// do we have valid packet ?
		if((pkt.crc_ok==1) && (pkt.length > 0)) {
			
			packetsReceived++;
			packetsReceivedBytes+=pkt.length;

			Serial.print("\r\nCC1101 have news! - ");
			Serial.print("Package len = ");
			Serial.print(pkt.length);
			Serial.print(" lgi=");
			Serial.print(pkt.lqi);
			Serial.print(" rssi=");
			Serial.print(pkt.rssi);
			Serial.print(" CRC=");
			Serial.print(pkt.crc_ok);
			
			cc1101.printCCPACKETdata(&pkt);
			
			
	#endif
			
		} // crc & len>0
		
	} // cc1101.readData
	
	attachIntr(); // re-attach interrupt so packetAvailable will be set true in ISR

	return true;

}



// ----------------------
//  Photon publish
// ----------------------

#ifdef PARTICLE


void publishStats() {
    char txt[64];
    
    if(((publishDevices % 4) > 0) && !cc1101.newDeviceAdded) {
        sprintf(txt,"Device #%d Sended %lu & received %lu packets (%lu/%lu bytes)",cc1101.deviceData.deviceAddress,packetsSend,packetsReceived,packetsSendBytes,packetsReceivedBytes);
    }
    else {
        sprintf(txt,"I am cc1101 address #%d, the others are ",cc1101.deviceData.deviceAddress);
        for(int idx = 0; idx < maxDevices; idx ++) {
            if(cc1101.deviceList[idx] != -1) sprintf(txt,"%s #%d",txt,cc1101.deviceList[idx]);
        }
    }
    
    cc1101.newDeviceAdded = false;
    Particle.publish("CC1101 Stats",txt);
    publishDevices ++;
    

 }
 
 #endif
//210220161831

#include "CC1101Radio.h"


/*
* Macros
*/

// Select (SPI) CC1101
#define cc1101_Select() digitalWrite(SPI_SS , LOW)

// Deselect (SPI) CC1101
#define cc1101_Deselect()  digitalWrite(SPI_SS , HIGH)



// Wait until SPI MISO line goes low
//#define /*wait_Miso()*/  while(digitalRead(SPI_MISO)==HIGH)

// Get GDO0 pin state
#define getGDO0state()  digitalRead(GDO0pin)

// Wait until GDO0 line goes high
#define wait_GDO0_high() while(!getGDO0state())

// Wait until GDO0 line goes low
#define wait_GDO0_low() while(getGDO0state())

// ----------------------
// Constructor
// ----------------------

CC1101Radio::CC1101Radio(uint8_t _SPI_SS, uint8_t _GDO0pin) {
	SPI_SS = _SPI_SS;
	GDO0pin = _GDO0pin;
}

CC1101Radio::CC1101Radio(void) {
	SPI_SS = _SPI_SS_;
	GDO0pin = _GDO0pin_;
}

// ----------------------
//  Wake up from power down state
// ----------------------

void CC1101Radio::wakeUp(void)
{
	cc1101_Select();                      // Select CC1101
	delay(10);
	cc1101_Deselect();                    // Deselect CC1101
}

// ----------------------
//  Transfer 1 byte to CC1101 regeister
// ----------------------

void CC1101Radio::writeReg(byte regAddr, byte value)
{
	cc1101_Select();                      // Select CC1101
	/*wait_Miso()*/;                          // Wait until MISO goes low
	SPI.transfer(regAddr);
	SPI.transfer(value);
	
	cc1101_Deselect();                    // Deselect CC1101
}


// ----------------------
//  Transfer bytes to CC1101 regeister
// ----------------------

void CC1101Radio::writeBurstReg(byte regAddr, byte* buffer, byte len)
{
	byte addr, i;
	
	addr = regAddr | WRITE_BURST;         // Enable burst transfer
	cc1101_Select();                      // Select CC1101
	/*wait_Miso()*/;                          // Wait until MISO goes low
	
	SPI.transfer(addr);
	for(i=0 ; i<len ; i++)
	SPI.transfer(buffer[i]);              // Send value

	cc1101_Deselect();                    // Deselect CC1101
}


// ----------------------
//  Strobe 1 byte to CC1101 regeister
// ----------------------

void CC1101Radio::cmdStrobe(byte cmd)
{
	cc1101_Select();                      // Select CC1101

	SPI.transfer(cmd);
	
	//  /*wait_Miso()*/;                  // Wait until MISO goes low
	cc1101_Deselect();                    // Deselect CC1101
}


// ----------------------
//  Read 1 byte from  CC1101 registers
// ----------------------

byte CC1101Radio::readReg(byte regAddr, byte regType)
{
	byte addr, val;

	addr = regAddr | regType;
	cc1101_Select();                      // Select CC1101

	SPI.transfer(addr);
	val = SPI.transfer(0x00);
	
	//  /*wait_Miso()*/;                          // Wait until MISO goes low
	
	cc1101_Deselect();                    // Deselect CC1101
	return val;
}


// ----------------------
//  Read bytes from CC1101 registers
// ----------------------

void CC1101Radio::readBurstReg(byte * buffer, byte regAddr, byte len)
{
	byte addr, i;
	
	addr = regAddr | READ_BURST;
	cc1101_Select();                      // Select CC1101
	
	SPI.transfer(addr);
	for(i=0 ; i<len ; i++)
	buffer[i] = SPI.transfer(0x00);         // Read result byte by byte
	
	
	//  /*wait_Miso()*/;                          // Wait until MISO goes low

	cc1101_Deselect();                    // Deselect CC1101
}

// ----------------------
//  Reset cc1101
// ----------------------

void CC1101Radio::reset(void)
{
	cc1101_Deselect();                    // Deselect CC1101
	delayMicroseconds(5);
	cc1101_Select();                      // Select CC1101
	delayMicroseconds(10);
	cc1101_Deselect();                    // Deselect CC1101
	delayMicroseconds(41);
	cc1101_Select();                      // Select CC1101

	/*wait_Miso()*/;                          // Wait until MISO goes low

	SPI.transfer(CC1101_SRES);              // Send reset command strobe

	/*wait_Miso()*/;                          // Wait until MISO goes low

	cc1101_Deselect();                    // Deselect CC1101

	setDefaultRegs();                     // Reconfigure CC1101
	
	flushRxFifo();              // flush receive queue

}

// ----------------------
//  Transfer comm setup to cc1101 registers
// ----------------------

void CC1101Radio::setDefaultRegs(void)
{

	writeReg(CC1101_IOCFG2,  CC1101_DEFVAL_IOCFG2);
	writeReg(CC1101_IOCFG1,  CC1101_DEFVAL_IOCFG1);
	writeReg(CC1101_IOCFG0,  CC1101_DEFVAL_IOCFG0);
	writeReg(CC1101_FIFOTHR,  CC1101_DEFVAL_FIFOTHR);
	writeReg(CC1101_PKTLEN,  CC1101_DEFVAL_PKTLEN);
	writeReg(CC1101_PKTCTRL1,  CC1101_DEFVAL_PKTCTRL1);
	writeReg(CC1101_PKTCTRL0,  CC1101_DEFVAL_PKTCTRL0);

	writeReg(CC1101_FSCTRL1,  CC1101_DEFVAL_FSCTRL1);
	writeReg(CC1101_FSCTRL0,  CC1101_DEFVAL_FSCTRL0);

	// Set default carrier frequency = 433 MHz
	setCarrierFreq(CFREQ_433);

	writeReg(CC1101_MDMCFG4,  CC1101_DEFVAL_MDMCFG4);
	writeReg(CC1101_MDMCFG3,  CC1101_DEFVAL_MDMCFG3);
	writeReg(CC1101_MDMCFG2,  CC1101_DEFVAL_MDMCFG2);
	writeReg(CC1101_MDMCFG1,  CC1101_DEFVAL_MDMCFG1);
	writeReg(CC1101_MDMCFG0,  CC1101_DEFVAL_MDMCFG0);
	writeReg(CC1101_DEVIATN,  CC1101_DEFVAL_DEVIATN);
	writeReg(CC1101_MCSM2,  CC1101_DEFVAL_MCSM2);
	writeReg(CC1101_MCSM1,  CC1101_DEFVAL_MCSM1);
	writeReg(CC1101_MCSM0,  CC1101_DEFVAL_MCSM0);
	writeReg(CC1101_FOCCFG,  CC1101_DEFVAL_FOCCFG);
	writeReg(CC1101_BSCFG,  CC1101_DEFVAL_BSCFG);
	writeReg(CC1101_AGCCTRL2,  CC1101_DEFVAL_AGCCTRL2);
	writeReg(CC1101_AGCCTRL1,  CC1101_DEFVAL_AGCCTRL1);
	writeReg(CC1101_AGCCTRL0,  CC1101_DEFVAL_AGCCTRL0);
	writeReg(CC1101_WOREVT1,  CC1101_DEFVAL_WOREVT1);
	writeReg(CC1101_WOREVT0,  CC1101_DEFVAL_WOREVT0);
	writeReg(CC1101_WORCTRL,  CC1101_DEFVAL_WORCTRL);
	writeReg(CC1101_FREND1,  CC1101_DEFVAL_FREND1);
	writeReg(CC1101_FREND0,  CC1101_DEFVAL_FREND0);
	writeReg(CC1101_FSCAL3,  CC1101_DEFVAL_FSCAL3);
	writeReg(CC1101_FSCAL2,  CC1101_DEFVAL_FSCAL2);
	writeReg(CC1101_FSCAL1,  CC1101_DEFVAL_FSCAL1);
	writeReg(CC1101_FSCAL0,  CC1101_DEFVAL_FSCAL0);
	writeReg(CC1101_RCCTRL1,  CC1101_DEFVAL_RCCTRL1);
	writeReg(CC1101_RCCTRL0,  CC1101_DEFVAL_RCCTRL0);
	writeReg(CC1101_FSTEST,  CC1101_DEFVAL_FSTEST);
	writeReg(CC1101_PTEST,  CC1101_DEFVAL_PTEST);
	writeReg(CC1101_AGCTEST,  CC1101_DEFVAL_AGCTEST);
	writeReg(CC1101_TEST2,  CC1101_DEFVAL_TEST2);
	writeReg(CC1101_TEST1,  CC1101_DEFVAL_TEST1);
	writeReg(CC1101_TEST0,  CC1101_DEFVAL_TEST0);
}


// ----------------------
//  Fill device data with default values
// ----------------------

void CC1101Radio::setDefaultDeviceInfo(uint8_t _deviceAddr, uint8_t _remoteDeviceAddr) {
	deviceData.deviceAddress = _deviceAddr;
	deviceData.remoteDeviceAddress = _remoteDeviceAddr;
	deviceData.syncWord[0] = 19;
	deviceData.syncWord[1] = 9;
	deviceData.channel = 5;
	deviceData.carrierFreq = CFREQ_868;
	deviceData.txPower = PA_LongDistance;
	deviceData.addressCheck = true;
}


// ----------------------
//  Begin SPI and set default device data
// ----------------------

void CC1101Radio::init(void) {
	init(_default_deviceAddress, _default_remoteDeviceAddress);
}

void CC1101Radio::init(uint8_t deviceAddress, uint8_t _remoteDeviceAddr)
{
	SPI.begin();
	
	// Configure SPI pins
	digitalWrite(SPI_SS, HIGH);
	pinMode(SPI_SS, OUTPUT);

	SPI.setClockDivider(spiDivide);
	
	pinMode(GDO0pin,OUTPUT);  // set GDO interrupt pin as output and high so we don't get stray interrupts while starting up
	digitalWrite(GDO0pin,HIGH);

	// set deviceData to defaults
	setDefaultDeviceInfo(deviceAddress, _remoteDeviceAddr);
	
}

// ----------------------
//  Transer device setup data and set GDO0 pin as input
// ----------------------

void CC1101Radio::begin(){
	reset();

	setDeviceData();

	printSetup();

	pinMode(GDO0pin,INPUT); // set GDO interrupt pin as inoput so we get get  interrupts

	writeReg(0x02,0x06); // gdo0 output pin configuration
	// setTxPowerAmp(PA_MinDistance);
	
	setRxState();
}

// ----------------------
//  Serial.print device setup
// ----------------------

void CC1101Radio::printSetup(){
	
	delay(100);
	hwsSerial.print("PARTNUM "); //cc1101=0
	hwsSerial.print(readReg(CC1101_PARTNUM, CC1101_STATUS_REGISTER));
	hwsSerial.print(", VERSION "); //cc1101=4
	hwsSerial.print(readReg(CC1101_VERSION, CC1101_STATUS_REGISTER));
	hwsSerial.print(", MARCSTATE ");
	hwsSerial.println(readReg(CC1101_MARCSTATE, CC1101_STATUS_REGISTER) & 0x1f);
	delay(100);
	char *freqStr[] = {"868","915","433"};
	hwsSerial.println("CC1101 initialized");hwsSerial.print("Network {");
		hwsSerial.print(deviceData.syncWord[0]);hwsSerial.print(", ");hwsSerial.print(deviceData.syncWord[1]);
	hwsSerial.print("}, ");
	hwsSerial.print("Frequency=0x"); hwsSerial.print(freqStr[deviceData.carrierFreq]);
	hwsSerial.print(", tx-power="); hwsSerial.print(deviceData.txPower,HEX);
	hwsSerial.print(", Channel=");
	hwsSerial.print(deviceData.channel);
	hwsSerial.print(", Device #");
	hwsSerial.print(deviceData.deviceAddress);
	hwsSerial.print(", remoteDevice #");
	hwsSerial.print(deviceData.remoteDeviceAddress);
	
	hwsSerial.print(", Address check: ");
	char* yesNo[] = {"no", "yes"};
	hwsSerial.println(yesNo[deviceData.addressCheck]);
	hwsSerial.print("GDO0 pin=");
	hwsSerial.print(GDO0pin);
	hwsSerial.print("SPI: Slave Select pin=");
	hwsSerial.print(SPI_SS);
	hwsSerial.print(", clock divider=");
	hwsSerial.println(spiDivide);
	hwsSerial.println("\r\n");

}


// ----------------------
//  Serial.print data part of CCPACKET
// ----------------------

void CC1101Radio::printCCPACKETdata(CCPACKET *pkt) {
	
	Serial.print(" data[");
	Serial.print(pkt->length);
	Serial.print("]=");
	
	char* sep = "{";
		
		for(int idx=0;idx<pkt->length;idx++) {
			Serial.print(sep);
			Serial.print(pkt->data[idx], DEC);
			sep=", ";
		} // for idx
		
	Serial.println("}");
}


// ----------------------
//  Set device data
// ----------------------

void CC1101Radio::setDeviceData(){
	
	setCarrierFreq(deviceData.carrierFreq);
	writeReg(CC1101_SYNC1,deviceData.syncWord[1]);
	writeReg(CC1101_SYNC0, deviceData.syncWord[0]);
	writeReg(CC1101_CHANNR,  deviceData.channel);
	writeReg(CC1101_ADDR, deviceData.deviceAddress);

	if(deviceData.addressCheck)
	enableAddressCheck();
	else disableAddressCheck();
	
	setTxPowerAmp(deviceData.txPower);
	writeReg(CC1101_PATABLE, paTableByte);
}


void CC1101Radio::setCarrierFreq(byte freq)
{
	switch(freq)
	{
		case CFREQ_915:
		writeReg(CC1101_FREQ2,  CC1101_DEFVAL_FREQ2_915);
		writeReg(CC1101_FREQ1,  CC1101_DEFVAL_FREQ1_915);
		writeReg(CC1101_FREQ0,  CC1101_DEFVAL_FREQ0_915);
		break;
		case CFREQ_433:
		writeReg(CC1101_FREQ2,  CC1101_DEFVAL_FREQ2_433);
		writeReg(CC1101_FREQ1,  CC1101_DEFVAL_FREQ1_433);
		writeReg(CC1101_FREQ0,  CC1101_DEFVAL_FREQ0_433);
		break;
		default:
		writeReg(CC1101_FREQ2,  CC1101_DEFVAL_FREQ2_868);
		writeReg(CC1101_FREQ1,  CC1101_DEFVAL_FREQ1_868);
		writeReg(CC1101_FREQ0,  CC1101_DEFVAL_FREQ0_868);
		break;
	}
	
}



// ----------------------
//  Put cc1101 in power-down state
// ----------------------

void CC1101Radio::setPowerDownState()
{
	// Comming from RX state, we need to enter the IDLE state first
	cmdStrobe(CC1101_SIDLE);
	// Enter Power-down state
	cmdStrobe(CC1101_SPWD);
}

// ----------------------
// Send a CCPACKET
// ----------------------

boolean CC1101Radio::sendData(CCPACKET packet)
{
	byte marcState;
	bool res = false;
	
	// Declare to be in Tx state. This will avoid receiving packets whilst
	// transmitting
	rfState = RFSTATE_TX;

	// Enter RX state
	setRxState();
	tmpMs=millis();
	uint8_t flushes=0;
	// Check that the RX state has been entered
	while (((marcState = readStatusReg(CC1101_MARCSTATE)) & 0x1F) != 0x0D)
	{
		if (marcState == 0x11)        // RX_OVERFLOW
		{
			flushes++;
			flushRxFifo();              // flush receive queue
			
		}
		if(millis()>tmpMs+200) {
			Serial.print("TROUBLE! marcState ");
			Serial.println(marcState);
			Serial.println(readStatusReg(CC1101_MARCSTATE));
			Serial.print(" flushes ");
			Serial.println(0x1F);
			return false;
		}
		
	}

	delayMicroseconds(500);

	// Set data length at the first position of the TX FIFO
	writeReg(CC1101_TXFIFO,  packet.length);
	// Write data into the TX FIFO
	writeBurstReg(CC1101_TXFIFO, packet.data, packet.length);

	// CCA enabled: will enter TX state only if the channel is clear
	setTxState();

	// Check that TX state is being entered (state = RXTX_SETTLING)
	marcState = readStatusReg(CC1101_MARCSTATE) & 0x1F;
	if((marcState != 0x13) && (marcState != 0x14) && (marcState != 0x15))
	{
		setIdleState();       // Enter IDLE state
		flushTxFifo();        // Flush Tx FIFO
		setRxState();         // Back to RX state

		// Declare to be in Rx state
		rfState = RFSTATE_RX;
		
		return false;
	}
	
	// Wait for the sync word to be transmitted
	wait_GDO0_high();

	// Wait until the end of the packet transmission
	wait_GDO0_low();

	// Check that the TX FIFO is empty
	if((readStatusReg(CC1101_TXBYTES) & 0x7F) == 0)
	res = true;

	setIdleState();       // Enter IDLE state
	flushTxFifo();        // Flush Tx FIFO

	// Enter back into RX state
	setRxState();

	// Declare to be in Rx state
	rfState = RFSTATE_RX;

	return res;
}

// ----------------------
//  send data - not used at the moment
// ----------------------


boolean CC1101Radio::sendData(uint8_t *data, uint8_t len) {
	
	byte marcState;
	bool res = false;
	
	// Declare to be in Tx state. This will avoid receiving packets whilst
	// transmitting
	rfState = RFSTATE_TX;

	// Enter RX state
	setRxState();
	tmpMs=millis();

	// Check that the RX state has been entered
	while (((marcState = readStatusReg(CC1101_MARCSTATE)) & 0x1F) != 0x0D)
	{
		if (marcState == 0x11) flushRxFifo(); // RX_OVERFLOW
		
	}

	delayMicroseconds(500);

	// Set data length at the first position of the TX FIFO
	writeReg(CC1101_TXFIFO,  len);
	// Write data into the TX FIFO
	writeBurstReg(CC1101_TXFIFO, data, len);

	// CCA enabled: will enter TX state only if the channel is clear
	setTxState();
	
	// Check that TX state is being entered (state = RXTX_SETTLING)
	marcState = readStatusReg(CC1101_MARCSTATE) & 0x1F;
	if((marcState != 0x13) && (marcState != 0x14) && (marcState != 0x15))
	{
		setIdleState();       // Enter IDLE state
		flushTxFifo();        // Flush Tx FIFO
		setRxState();         // Back to RX state

		// Declare to be in Rx state
		rfState = RFSTATE_RX;
		
		return false;
	}

	// Wait for the sync word to be transmitted
	wait_GDO0_high();

	// Wait until the end of the packet transmission
	wait_GDO0_low();

	// Check that the TX FIFO is empty
	if((readStatusReg(CC1101_TXBYTES) & 0x7F) == 0)
	res = true;
	
	setIdleState();       // Enter IDLE state
	flushTxFifo();        // Flush Tx FIFO

	// Enter back into RX state
	setRxState();

	// Declare to be in Rx state
	rfState = RFSTATE_RX;

	return res;
	
}

// ----------------------
//  Send a messsage
// ----------------------

boolean CC1101Radio::sendMessage(uint8_t toDevice, uint8_t state, uint16_t transNum, uint8_t parm) {
	uint8_t* data;
	return sendMessage(toDevice,state,transNum,parm,data,0);
}

boolean CC1101Radio::sendMessage(messageInfo *msg) {
	return sendMessage(msg->toDevice,msg->messageType,msg->messageId,msg->parm,msg->data,msg->dataLength);
}

boolean CC1101Radio::sendMessage(uint8_t toDevice, uint8_t state, uint16_t transNum, uint8_t parm, uint8_t *data, uint8_t dataLength ) {
	
	CCPACKET pkt;
	pkt.data[0] = toDevice;
	pkt.data[1] = deviceData.deviceAddress;
	pkt.data[2] = state;
	pkt.data[3] = lowByte(transNum);
	pkt.data[4] = highByte(transNum);
	pkt.data[5] = parm;
	pkt.length = messageInfoHeaderLength;

	
	if(dataLength>0) {
		for(int i=0;i<dataLength;i++) {
			pkt.data[pkt.length] = data[i];
			pkt.length++;
		}
	}

	return sendData(pkt);
}

// ----------------------
//  Wait for a confirmation after sending
// ----------------------

bool CC1101Radio::waitResponse(messageInfo *msg, messageInfo *responceMsg, uint8_t waitFlag, uint16_t timeoutMs) {
	uint32_t startMs = millis();
	while(millis()<startMs + timeoutMs) {
		
		if(packetAvailable) {
			CCPACKET pkt;
			
			detectMessageInfo(responceMsg);

			if(responceMsg->messageType == _state_confirmed) {
				if((sendMsg.messageId == responceMsg->messageId) &&
				(sendMsg.toDevice == responceMsg->fromDevice)) {
					return true;
				}
			} // confirmation
		} // packetAvailable
		delay(5); // give receiving device time to answer
		
	} // while
	
	return false;
}

// ----------------------
//  Receive and parse/detect data
// ----------------------

byte CC1101Radio::receiveData(CCPACKET * pkt)
{
	byte val;
	byte rxBytes = readStatusReg(CC1101_RXBYTES);

	// Any byte waiting to be read and no overflow?
	if (rxBytes & 0x7F && !(rxBytes & 0x80))
	{
		
		// Read data length
		pkt->length = readConfigReg(CC1101_RXFIFO);
		// If packet is too long
		if (pkt->length > CC1101_DATA_LEN)
		pkt->length = 0;   // Discard packet
		else
		{
			// Read data packet
			readBurstReg(pkt->data, CC1101_RXFIFO, pkt->length);
			// Read RSSI
			pkt->rssi = readConfigReg(CC1101_RXFIFO);
			// Read LQI and CRC_OK
			val = readConfigReg(CC1101_RXFIFO);
			pkt->lqi = val & 0x7F;
			pkt->crc_ok = bitRead(val, 7);
			updateDeviceList(pkt);
		}
	}
	else
	pkt->length = 0;

	setIdleState();       // Enter IDLE state
	flushRxFifo();        // Flush Rx FIFO
	
	cmdStrobe(CC1101_SCAL);

	// Back to RX state
	setRxState();

	return pkt->length;
}

bool CC1101Radio::detectMessageInfo(messageInfo *info) {
	packetAvailable = false;
	CCPACKET pkt;
	if(receiveData(&pkt) > 0) {
		
		if(pkt.crc_ok && pkt.length > 0) {
			info->toDevice = pkt.data[0];
			info->fromDevice = pkt.data[1];
			info->messageType = pkt.data[2];
			info->messageId = pkt.data[3] + pkt.data[4]*256;
			info->parm = pkt.data[5];

			info->messageType = pkt.data[2];

			if(pkt.length>messageInfoHeaderLength) {
				info->dataLength=pkt.length-messageInfoHeaderLength;
				memcpy(info->data, pkt.data+messageInfoHeaderLength, info->dataLength);
			}
			else {
				info->dataLength=0;
			}
		}
		
		
		if(info->messageType == _state_confirmed) {
			if((sendMsg.messageId == info->messageId) && (sendMsg.toDevice == info->fromDevice)) {
				sendMsg.messageType = _state_confirmed;
				
			}
			else Serial.print("Confirmation Error!");
		}
		
		// send confirmation ?
		if(info->parm & _parm_request_confirmation) {
			// use another sendMessage variant, if you need to add data to the confirmation message
			// instead of '0' some meaningfull info could be send in parm attribute
			sendMessage(info->fromDevice, _state_confirmed, info->messageId, 0); //instead of '0' some meaningfull info could be send
			Serial.print("\t\tSend confirmation to device#");
			Serial.print(info->fromDevice);
			Serial.print(" for msg#");
			Serial.println(info->messageType);
		}

		return true;
	}
	return false;
}


// ----------------------
//  Keep track of devices
// ----------------------

void CC1101Radio::updateDeviceList(CC1101Radio::CCPACKET *pkt) {
	
	int firstEmptyIdx=-1;
	
	for(int idx = 0; idx < maxDevices; idx ++) {
		if(deviceList[idx] == pkt->data[1]) return;
		if((deviceList[idx] == -1) && (firstEmptyIdx==-1)) firstEmptyIdx = idx;
	}
	
	if(firstEmptyIdx >= 0) {
		deviceList[firstEmptyIdx] = pkt->data[1];
		newDeviceAdded = true;
		Serial.print("New remote device #");
		Serial.print(deviceList[firstEmptyIdx]);
		Serial.print(" is added idx ");Serial.println(firstEmptyIdx);

	}

}

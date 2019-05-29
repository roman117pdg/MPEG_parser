#include "pch.h"
#include <iostream>
#include <fstream>

//MPEG

class TS_Header  //ts header
{
public:
	uint8_t SB; //8 sync byte, == G
	bool E; //1 error
	bool S; //1 payload unit start indicator, pokazuje czy lecą dane
	bool T; //1 transport priority
	uint16_t PID; //13 Packet Identier,
	uint8_t TSC; //2 Transport Scrambling Control,
	uint8_t AFC; //2 Adaptation Field Control, 1-dane;2-poleadaptacyjne;3-dane i polea daptacyjne
	uint8_t  CC; //4 Control Counter, pilnuje ze pakiety sa wysylane po koleji
};
class TS_Packet
{
public:
	TS_Header header;
	uint8_t buf[188];

	void parse()
	{
		header.SB = buf[0];
		header.E = buf[1] & 0b10000000;
		header.S = buf[1] & 0b01000000;
		header.T = buf[1] & 0b00100000;
		header.PID = (uint16_t(buf[1] & 0b00011111) << 8) | buf[2];
		header.TSC = buf[3] & 0b11000000 >> 6;
		header.AFC = buf[3] & 0b00110000 >> 4;
		header.CC = buf[3] & 0b00001111;
	};

	void clear()
	{
		header.SB = 0;
		header.E = 0;
		header.S = 0;
		header.T = 0;
		header.PID = 0;
		header.TSC = 0;
		header.AFC = 0;
		header.CC = 0;
	};
	void print()
	{
		if (header.PID == 202)
		{
			std::cout << "SB: " << header.SB << " | ";
			std::cout << "E: " << header.E << " | ";
			std::cout << "S: " << header.S << " | ";
			std::cout << "T: " << header.T << " | ";
			std::cout << "PID: " << header.PID << " | ";
			std::cout << "TSC: " << int(header.TSC) << " | ";
			std::cout << "AFC: " << int(header.AFC) << " | ";
			std::cout << "CC: " << int(header.CC) << " | ";
			std::cout << "\n";
		}
	};
};

class PES_Header
{
public:
	uint8_t C1;
	uint8_t C2;
	uint8_t C3;
	uint8_t ID;
	uint16_t LEN;
};

class PES_Packet
{

public:
	PES_Header header;
	uint8_t packet[4096];

	void parse()
	{
		header.C1 = packet[0];
		header.C2 = packet[1];
		header.C3 = packet[2];
		header.ID = packet[3];
		header.LEN = _byteswap_ushort(*((uint16_t*)&packet[4]));
	};

	void clear()
	{
		header.C1 = 0;
		header.C2 = 0;
		header.C3 = 0;
		header.ID = 0;
		header.LEN = 0;
		for (int i = 0; i < sizeof(packet); i++) {
			packet[i] = 0;
		}
	};
	bool checkControlBytes()
	{
		if (header.C1 == 0 and header.C2 == 0 and header.C3 == 1)
			return true;
		else
			return false;
	};
	void print()
	{
		if (checkControlBytes())
		{
			std::cout << "Stream id:  " << int(header.ID) << ", ";
			std::cout << "PES Packet length: " << int(header.LEN);
			std::cout << "\n";
		}
	};
};

int main()
{
	std::fstream file;

	file.open("example.ts", std::ios::in | std::ios::binary);
	if (file.good())
	{
		while (!file.eof())
		{

			TS_Packet ts;
			PES_Packet pes;
			file.read((char *)ts.buf, 188);
			ts.parse();
			ts.print();

			if (ts.header.PID == 202 and ts.header.S == 1) {

				int j = 0;

				for (int k = 0; k < 19; k++)
				{
					if (ts.header.AFC == 0b01) // czy payload
					{
						for (int i = 4; i < 188; i++, j++)
						{
							pes.packet[j] = ts.buf[i];
						}
					}
					else //czy posiada adaptation field
					{
						for (int i = 5 + ts.buf[4]; i < 188; i++, j++) // buf[4] len of adaptation field
						{
							pes.packet[j] = ts.buf[i];
						}

					}
				}
				pes.parse();
				pes.print();

			}
		}
		file.close();
	}

	system("PAUSE");
	return 0;
}
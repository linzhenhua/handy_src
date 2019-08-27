void byteStrToHex(unsigned char* readbuf, void *writebuf, size_t len) {
	for(size_t i=0; i < len; i++) {
		char *l = (char*) (2*i + ((intptr_t) writebuf));
		sprintf(l, "%02x", readbuf[i]);
	}
}

void hexStrToByte(const char* source, unsigned char* dest, size_t sourceLen) {
	short i;
	unsigned char highByte, lowByte;

	for (i = 0; i < sourceLen; i += 2) {
		highByte = std::toupper(source[i]);
		lowByte = std::toupper(source[i+1]);

		if (highByte > 0x39)
			highByte -= 0x37;
		else
			highByte -= 0x30;

		if (lowByte > 0x39)
			lowByte -= 0x37;
		else
			lowByte -= 0x30;

		dest[i / 2] = (highByte << 4) | lowByte;
	}
}
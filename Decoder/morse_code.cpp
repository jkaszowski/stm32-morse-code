#include "morse_code.h"

extern char human_readable[128];
extern char buffer[256];
extern char *ptr;
extern void CDC_Transmit_FS(char *ptr, int);
const char *morseCode[] = { ".-",   // A
		"-...", // B
		"-.-.", // C
		"-..",  // D
		".",    // E
		"..-.", // F
		"--.",  // G
		"....", // H
		"..",   // I
		".---", // J
		"-.-",  // K
		".-..", // L
		"--",   // M
		"-.",   // N
		"---",  // O
		".--.", // P
		"--.-", // Q
		".-.",  // R
		"...",  // S
		"-",    // T
		"..-",  // U
		"...-", // V
		".--",  // W
		"-..-", // X
		"-.--", // Y
		"--.."  // Z
		};

// turn Morse Coded Letter to ASCII character
char getAscii(char *str) {
	for (int i = 0; i < 26; i++) {
		if (strcmp(str, morseCode[i]) == 0) {
			return i + 'a';
		}
	}
}

void convertWord(char *word) {
	char korektor[] = " ";
	char *schowek;
	schowek = strtok(word, korektor);
	*(ptr++) = getAscii(schowek);
	while ((schowek = strtok(NULL, korektor)) != NULL) {
		*(ptr++) = getAscii(schowek);
	}
	*(ptr++) = ' ';
}

void convertBuffer(char *stack) {
	char korektor[] = " ";
	char *schowek;
	ptr = human_readable;
	schowek = strtok(stack, korektor);
	*(ptr++) = getAscii(schowek);
	while ((schowek = strtok(NULL, korektor)) != NULL) {
		*(ptr++) = getAscii(schowek);
	}
	*(ptr++) = 0;
}

void printAscii(char *stack) {
	char delimiter[] = " ";
	char *tmp;
	tmp = strtok(stack, delimiter);
	char result = getAscii(tmp);
	CDC_Transmit_FS(&result, 1);
	while ((tmp = strtok(NULL, delimiter)) != NULL) {
		result = getAscii(tmp);
		CDC_Transmit_FS(&result, 1);
	}
}

//void run(char *stack) {
//  char korektor[] = " ";
//  char *schowek;
//  schowek = strtok(stack, korektor);
//  char result = getAscii(schowek);
//  CDC_Transmit_FS(&result, 1);
//  while ((schowek = strtok(NULL, korektor)) != NULL) {
//    result = getAscii(schowek);
//    CDC_Transmit_FS(&result, 1);
//  }
//}

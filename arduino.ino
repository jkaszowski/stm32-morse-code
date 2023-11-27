// Define the mapping between the alphabet and Morse code symbols
const char* morseCode[] = {
  ".-",   // A
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

const int ledPin = 7; // Pin number for the LED
const int buttonPin = 8; // Pin number for the button
const int dotTime = 200;      // Time for a dot (in milliseconds)
const int dashTime = dotTime * 3; // Time for a dash (in milliseconds)
const int interElementSpace = dotTime; // Time between dots and dashes within a letter
const int interLetterSpace = dotTime * 3; // Time between letters
const int interWordSpace = dotTime * 7; // Time between words

void setup() {

   // Set up the serial communication
  Serial.begin(9600);
   pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT); // Set the button pin as input

  // Calibration phase
  Serial.println("Calibration in progress...");
  delay(1000); // Give some time to open the serial monitor

  unsigned long lastButtonPressTime = millis(); // Record the time of the last button press
  if (digitalRead(buttonPin) == HIGH) {
      // If the button is pressed, send a high signal
      digitalWrite(ledPin, HIGH);
      lastButtonPressTime = millis(); // Update the time of the last button press
    } else {
      // If the button is not pressed, send a low signal
      digitalWrite(ledPin, LOW);
    }

    // If the button has not been pressed for 20 seconds, end the calibration
    if (millis() - lastButtonPressTime > 20000) {
      Serial.println("Calibration complete");
      delay(2000); // Give some time for the user to see the calibration results
      break; // Exit the loop
    }
    Serial.flush();
    while(Serial.read()>=0){}
 
}

void loop() {
  // Capture the text input from the user
  String input;
  Serial.println("Enter text to convert to Morse code:");
  while (!Serial.available()) {
    // Wait for user input
  }
  input = Serial.readString();
  input.trim(); // Remove leading and trailing whitespaces

  // Convert the input to Morse code
  String morseCodeOutput = "";
  for (int i = 0; i < input.length(); i++) {
    char c = input.charAt(i);
    if (c == ' ') {
      morseCodeOutput += "   "; // Use three spaces to separate words in Morse code
    } else if (c >= 'A' && c <= 'Z') {
      morseCodeOutput += String(morseCode[c - 'A']) + " ";
    } else if (c >= 'a' && c <= 'z') {
      morseCodeOutput += String(morseCode[c - 'a']) + " ";
    }
  }

  // Output the Morse code and original text to the serial monitor
  Serial.println("Original text: " + input);
  Serial.println("Morse code: " + morseCodeOutput);

  // Output the Morse code using the LED
  for (int i = 0; i < input.length(); i++) {
    char c = input.charAt(i);
    if (c == ' ') {
      delay(interWordSpace);
    } else if (c >= 'A' && c <= 'Z') {
      outputMorseCode(morseCode[c - 'A']);
      delay(interLetterSpace);
    } else if (c >= 'a' && c <= 'z') {
      outputMorseCode(morseCode[c - 'a']);
      delay(interLetterSpace);
    }
  }
}

void outputMorseCode(const char* code) {
  for (int i = 0; i < strlen(code); i++) {
    if (code[i] == '.') {
      digitalWrite(ledPin, HIGH);
      delay(dotTime);
      digitalWrite(ledPin, LOW);
      delay(interElementSpace);
    } else if (code[i] == '-') {
      digitalWrite(ledPin, HIGH);
      delay(dashTime);
      digitalWrite(ledPin, LOW);
      delay(interElementSpace);
    }
  }
}

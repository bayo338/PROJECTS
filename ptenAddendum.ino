#include <WiFi.h>
#include <DMD32.h>
#include "fonts/SystemFont5x7.h"
#include "fonts/Arial_black_16.h"
#include <Ticker.h>

//#include <RtcDS1302.h>


// Replace with your network credentials
const char* ssid = "printf";
const char* password = "12345678";


// Set web server port number to 80
WiFiServer server(80);
Ticker marqueeTick;
// Variable to store the HTTP request
String header;

boolean displayHappyEmoji = false;

boolean ret = false;


//Set up the DMD library as dmd
#define DISPLAYS_ACROSS 1
#define DISPLAYS_DOWN 1
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);


void marquee() {
  if (displayHappyEmoji) {
    // Clear the screen
    dmd.clearScreen(true);
    // Draw the happy emoji
    dmd.drawFilledBox(10, 5, 12, 7, GRAPHICS_NORMAL);
    dmd.drawFilledBox(20, 5, 22, 7, GRAPHICS_NORMAL);
    dmd.drawFilledBox(8, 9, 24, 11, GRAPHICS_NORMAL);
    dmd.drawFilledBox(9, 8, 23, 12, GRAPHICS_NORMAL);
    dmd.drawFilledBox(10, 7, 22, 13, GRAPHICS_NORMAL);
  } else {
    // Continue with the regular marquee scrolling
    dmd.stepMarquee(-1, 0);
  }
}
//Timer setup
//create a hardware timer  of ESP32
hw_timer_t* timer = NULL;

/*--------------------------------------------------------------------------------------
  Interrupt handler for Timer1 (TimerOne) driven DMD refresh scanning, this gets
  called at the period set in Timer1.initialize();
--------------------------------------------------------------------------------------*/
void IRAM_ATTR triggerScan() {
  dmd.scanDisplayBySPI();
  //   Serial.println("Here!!!");
}


char scrollingMessage[1000];  

//////////////////////////////////////////////////////////////////////

char* urlDecode(const char* encoded) {
  int len = strlen(encoded);
  char* decoded = (char*)malloc((len + 1) * sizeof(char));
  int j = 0;

  for (int i = 0; i < len; i++) {
    if (encoded[i] == '%') {
      if (i + 2 < len) {
        char hex[3] = { encoded[i + 1], encoded[i + 2], '\0' };
        int value = strtol(hex, NULL, 16);
        decoded[j++] = (char)value;
        i += 2;
      }
    } else if (encoded[i] == '+') {
      decoded[j++] = ' ';
    } else if (encoded[i] == '&') {
      decoded[j++] = '&';
    } else if (encoded[i] == '=') {
      decoded[j++] = '=';
    } else if (encoded[i] == '?') {
      decoded[j++] = '?';
    } else {
      decoded[j++] = encoded[i];
    }
  }

  decoded[j] = '\0';
  return decoded;
}


///////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------------------
  setup
  Called by the Arduino architecture before the main loop begins
--------------------------------------------------------------------------------------*/
void setup(void) {

  // Connect to Wi-Fi network with SSID and password
  Serial.begin(115200);
  Serial.println("P10 Display");

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid,password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  // return the clock speed of the CPU
  uint8_t cpuClock = ESP.getCpuFreqMHz();
  // Use 1st timer of 4
  // devide cpu clock speed on its speed value by MHz to get 1us for each signal  of the timer
  timer = timerBegin(0, cpuClock, true);
  // Attach triggerScan function to our timer
  timerAttachInterrupt(timer, &triggerScan, true);
  // Set alarm to call triggerScan function
  // Repeat the alarm (third parameter)
  timerAlarmWrite(timer, 300, true);

  // Start an alarm
  timerAlarmEnable(timer);

  //clear/init the DMD pixels held in RAM
  dmd.clearScreen(true);  //true is normal (all pixels off), false is negative (all pixels on)

  // brightness control
  analogWrite(PIN_DMD_nOE, 100);

  // byte b;
  // 10 x 14 font clock, including demo of OR and NOR modes for pixels so that the flashing colon can be overlayed
  dmd.clearScreen(true);
  // dmd.selectFont(Arial_Black_16);
  // dmd.drawChar(0, 3, '1', GRAPHICS_NORMAL);
  // dmd.drawChar(7, 3, '4', GRAPHICS_NORMAL);
  // dmd.drawChar(17, 3, '0', GRAPHICS_NORMAL);
  // dmd.drawChar(25, 3, '0', GRAPHICS_NORMAL);
  // dmd.drawChar(15, 3, ':', GRAPHICS_OR);  // clock colon overlay on
  // delay(1000);
  // dmd.drawChar(15, 3, ':', GRAPHICS_NOR);  // clock colon overlay off
  // delay(1000);
  // dmd.drawChar(15, 3, ':', GRAPHICS_OR);  // clock colon overlay on
  // delay(1000);
  // dmd.drawChar(15, 3, ':', GRAPHICS_NOR);  // clock colon overlay off
  // delay(1000);
  // dmd.drawChar(15, 3, ':', GRAPHICS_OR);  // clock colon overlay on
  // delay(1000);
  // // half the pixels on
  // dmd.drawTestPattern(PATTERN_ALT_0);
  // delay(1000);
  // // the other half on
  // dmd.drawTestPattern(PATTERN_ALT_1);
  // delay(1000);

  // // display some text
  // dmd.clearScreen(true);
  // dmd.selectFont(System5x7);
  // for (byte x = 0; x < DISPLAYS_ACROSS; x++) {
  //   for (byte y = 0; y < DISPLAYS_DOWN; y++) {
  //     dmd.drawString(2 + (32 * x), 1 + (16 * y), "Great", 5, GRAPHICS_NORMAL);
  //     dmd.drawString(2 + (32 * x), 9 + (16 * y), "Icons", 5, GRAPHICS_NORMAL);
  //   }
  // }
  // delay(2000);
  // // draw a border rectangle around the outside of the display
  // dmd.clearScreen(true);
  // dmd.drawBox(0, 0, (32 * DISPLAYS_ACROSS) - 1, (16 * DISPLAYS_DOWN) - 1, GRAPHICS_NORMAL);
  // delay(1000);

  // for (byte y = 0; y < DISPLAYS_DOWN; y++) {
  //   for (byte x = 0; x < DISPLAYS_ACROSS; x++) {
  //     // draw an X
  //     int ix = 32 * x;
  //     int iy = 16 * y;
  //     dmd.drawLine(0 + ix, 0 + iy, 11 + ix, 15 + iy, GRAPHICS_NORMAL);
  //     dmd.drawLine(0 + ix, 15 + iy, 11 + ix, 0 + iy, GRAPHICS_NORMAL);
  //     delay(1000);
  //     // draw a circle
  //     dmd.drawCircle(16 + ix, 8 + iy, 5, GRAPHICS_NORMAL);
  //     delay(1000);
  //     // draw a filled box
  //     dmd.drawFilledBox(24 + ix, 3 + iy, 29 + ix, 13 + iy, GRAPHICS_NORMAL);
  //     delay(1000);
  //   }
  // }

  // // stripe chaser
  // for (b = 0; b < 20; b++) {
  //   dmd.drawTestPattern((b & 1) + PATTERN_STRIPE_0);
  //   delay(200);
  // }
  // delay(200);


  // dmd.clearScreen(true);
  // int x, y;
  // for (y = 0; y < 16; y++) {
  //   for (x = 0; x < 32; x++) {
  //     // draw smiley face
  //     if ((x >= 10 && x <= 12 && y >= 5 && y <= 7) ||   // left eye
  //         (x >= 20 && x <= 22 && y >= 5 && y <= 7) ||   // right eye
  //         (x >= 8 && x <= 24 && y >= 9 && y <= 11) ||   // mouth
  //         (x >= 9 && x <= 23 && y >= 8 && y <= 12) ||   // mouth
  //         (x >= 10 && x <= 22 && y >= 7 && y <= 13)) {  // mouth
  //       dmd.writePixel(x, y, GRAPHICS_NORMAL, 1);
  //     }
  //   }
  // }
  // delay(5000);

  memset(scrollingMessage, 0, sizeof(scrollingMessage));
  strcpy(scrollingMessage, "Welcome to the Embedded Systems Lab.");
  ret = false;
  dmd.selectFont(Arial_Black_16);
  dmd.drawMarquee(scrollingMessage, strlen(scrollingMessage), (32 * DISPLAYS_ACROSS) - 1, 0);
  marqueeTick.attach_ms(20, marquee);
}


WiFiClient client;
/*--------------------------------------------------------------------------------------
  loop
  Arduino architecture main loop 
--------------------------------------------------------------------------------------*/
void loop(void) {

  client = server.available();  // Listen for incoming clients

  if (client) {
    // If a new client connects,
    Serial.println("New Client.");  // print a message out in the serial port
    String currentLine = "";        // make a String to hold incoming data from the client
    while (client.connected()) {    // loop while the client's connected
      if (client.available()) {     // if there's bytes to read from the client,
        char c = client.read();     // read a byte, then
        Serial.write(c);            // print it out the serial monitor
        header += c;
        if (c == '\n') {
          // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            int start = header.indexOf("GET /?displaydata");
            if (start >= 0) {
              start = start + 18;
              int end = header.indexOf("\r\n", start);  // Find end of header value
              String displayString = header.substring(start, end - 8);
              memset(scrollingMessage, 0, sizeof(scrollingMessage));
              strcpy(scrollingMessage, displayString.c_str());
               if (strcmp(displayString.c_str(), ":)") == 0) {
                 displayHappyEmoji = true;
             } else {
              strcpy(scrollingMessage, urlDecode(displayString.c_str()));
              ret = false;
              dmd.selectFont(Arial_Black_16);
              dmd.drawMarquee(scrollingMessage, strlen(scrollingMessage), (32 * DISPLAYS_ACROSS) - 1, 0);
              // Serial.printf(scrollingMessage);

             }
            }
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>ESP32 Scrolling LED Display Server</h1>");
            client.println("<h2>Mood Smiley</h2>");
            client.println("<form action=\"\" method=\"get\">");
            client.println("<label for=\"text-input\">Enter your text:</label>");
            client.println("<input type=\"text\" id=\"text-input\" name=\"displaydata\" required>");
            client.println("<input type=\"submit\" value=\"Submit\">");
            client.println("</form>");
            client.println("</body></html>");
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else {  // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
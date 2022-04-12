#include <usbdrv.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

int servoPin = 5;
Servo myservo;


LiquidCrystal_I2C lcd(0x27, 16, 2);
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);

#define RQ_READ_UID 0
#define RQ_SET_SERVO 1
#define RQ_PRINT 2

static char uid[8]="";

String get_uid(){
  String content= "";
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return content;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return content;
  }
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  return content;
}

void print_welcome(){
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Welcome");
}

void print_incard(){
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("PLEASE");
  lcd.setCursor(3, 1);
  lcd.print("INSERT CARD");
}

void print_deny(){
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Access Denied");
  lcd.setCursor(0, 1);
  lcd.print("Please Try Again");
}

void set_servo(int state){
  if (state == 0){
    myservo.write(90);
    print_incard();
  }
  else if (state == 1){
    myservo.write(180);
    print_welcome();
  }
}

usbMsgLen_t usbFunctionSetup(uint8_t data[8])
{
    usbRequest_t *rq = (usbRequest_t *)data;
    if (rq->bRequest == RQ_READ_UID)
    {
      get_uid().toCharArray(uid, 8);
      usbMsgPtr = (char *) &uid;  
      return sizeof(uid);
    }
    else if (rq->bRequest == RQ_SET_SERVO)
    {
      set_servo(rq->wValue.bytes[0]);
      return 0;
    }
    else if (rq->bRequest == RQ_PRINT)
    {
      if (rq->wValue.bytes[0] == 1){
        print_deny();
      }
      else {
        print_incard();
      }
    }
}

void setup() 
{
  SPI.begin();
  mfrc522.PCD_Init();   // Initiate MFRC522
  lcd.init();
  lcd.backlight();
  myservo.attach(servoPin);
  set_servo(90);
  print_incard();
  usbInit();
    /* enforce re-enumeration of USB devices */
  usbDeviceDisconnect();
  delay(300);
  usbDeviceConnect();
}
void loop() 
{
  usbPoll();
} 

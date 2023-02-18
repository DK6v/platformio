#define si3  OCR1A=4050;
#define lad3 OCR1A=4290;
#define la3  OCR1A=4545;
#define sod3 OCR1A=4816;
#define so3  OCR1A=5102;
#define fad3 OCR1A=5405;
#define fa3  OCR1A=5727;
#define mi3  OCR1A=6067;
#define red3 OCR1A=6428;
#define re3  OCR1A=6810;
#define dod3 OCR1A=7215;
#define do3  OCR1A=7645;
#define si2  OCR1A=8099;
#define lad2 OCR1A=8581;
#define la2  OCR1A=9091;
#define sod2 OCR1A=9631;
#define so2  OCR1A=10204;
#define fad2 OCR1A=10811;
#define fa2  OCR1A=11454;
#define mi2  OCR1A=12135;
#define red2 OCR1A=12856;
#define re2  OCR1A=13621;
#define dod2 OCR1A=14431;
#define do2  OCR1A=15289;
#define si1  OCR1A=16198;
#define lad1 OCR1A=17161;
#define la1  OCR1A=18182;
#define sod1 OCR1A=19263;
#define so1  OCR1A=20408;
#define fad1 OCR1A=21622;
#define fa1  OCR1A=22908;
#define mi1  OCR1A=24270;
#define red1 OCR1A=25713;
#define re1  OCR1A=27242;
#define dod1 OCR1A=28862;
#define do1  OCR1A=30578;
#define si0  OCR1A=32396;
#define lad0 OCR1A=34323;
#define la0  OCR1A=36364;
#define sod0 OCR1A=38526;
#define so0  OCR1A=40817;
#define fad0 OCR1A=43244;
#define fa0  OCR1A=45815;
#define mi0  OCR1A=48540;
#define red0 OCR1A=51426;
#define re0  OCR1A=54484;
#define dod0 OCR1A=57724;
#define do0  OCR1A=61156;

#define d2 delay(1000);
#define d4 delay(500);
#define d8 delay(250);
#define d16 delay(125);

#define p4 TCCR1B = 0b00001000;delay(500);TCCR1B = 0b00001001;
#define p8 TCCR1B = 0b00001000;delay(250);TCCR1B = 0b00001001;
#define p64 TCCR1B = 0b00001000;delay(7);TCCR1B = 0b00001001;

void setup() {
  pinMode (9, OUTPUT);
  pinMode (10, OUTPUT);
  TCCR1A = 0b01010000;
  //TCCR1B = 0b00001010;  // Делитель на 8, на выходе 1.000 MGz
  TCCR1B = 0b00001001;    // Делитель на 0, на выходе 8.000 MGz
  DDRD = 0b11111111;

  so1 d8
  kuplet();
  do2 d4 si1 d8 p64 si1 d16 la1 d16 so1 d4 p8 so1 d8
  kuplet();
  so1 d2 la1 d4 si1 d4
  do2 d2 p64 do2 d4
  
  PORTD = 0b00000000;
  TCCR1B = 0b00001000; // Осанов
  digitalWrite (9, LOW);
  digitalWrite (10, LOW);
  pinMode (9, INPUT);
  pinMode (10, INPUT);
}

void loop() {
}

void kuplet(){
  do2 d4 so1 d8 p64 so1 d16 la1 d16 si1 d4 mi1 d8 p64 mi1 d8
  la1 d4 so1 d8 p64 so1 d16 fa1 d16 so1 d4 do1 d8 p64 do1 d8
  re1 d4 p64 re1 d8 mi1 d8 fa1 d4 p64 fa1 d8 so1 d8
  la1 d4 si1 d8 do2 d8 re2 d4 p64 re2 d8 so1 d8
  mi2 d4 re2 d8 p64 re2 d16 do2 d16 re2 d4 si1 d8 so1 d8
  do2 d4 si1 d8 p64 si1 d16 la1 d16 si1 d4 mi1 d8 p64 mi1 d8
  la1 d4 so1 d8 fa1 d8 so1 d4 do1 d8 p64 do1 d8
  do2 d4 si1 d8 p64 si1 d16 la1 d16 so1 d2
  mi2 d2 re2 d8 do2 d8 si1 d8 do2 d8
  re2 d4 p64 re2 d8 so1 d8 p64 so1 d2
  do2 d2 si1 d8 la1 d8 so1 d8 la1 d8
  si1 d4 p64 si1 d8 mi1 d8 p64 mi1 d4 p4
  do2 d4 la1 d8 p64 la1 d16 si1 d16 do2 d4 la1 d8 p64 la1 d16 si1 d16
  do2 d4 la1 d8 do2 d8 fa2 d2 p64
  fa2 d2 mi2 d8 re2 d8 do2 d8 re2 d8
  mi2 d4 p64 mi2 d8 do2 d8 p64 do2 d2
  re2 d2 do2 d8 si1 d8 la1 d8 si1 d8
  do2 d4 p64 do2 d8 la1 d8 p64 la1 d2
  do2 d4 si1 d8 la1 d8 so1 d4 do1 d8 do2 d8 p64
}
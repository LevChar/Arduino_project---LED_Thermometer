//Matrix pins
int CLK=2;
int DIN=3;
int CS=4;

//Thermistor pin
int ThermistorPin = A0;

//Thermistor resistor resistance
float R1 = 10000; 

//Steinhart-Hart coeficients for thermistor
float c1 = 0.001129148;
float c2 = 0.000234125; 
float c3 = 0.0000000876741; 

//This matrix is used to address the appropriate column of the LED display,
// the index of each row in the matrix is addressed to the appropriate column
//index on the LED display - which is represented using last 4 bits.
bool cols[][8] = 
{
  {0, 0, 0, 0, 0, 0, 0, 1}, // 1
  {0, 0, 0, 0, 0, 0, 1, 0}, // 2
  {0, 0, 0, 0, 0, 0, 1, 1}, // 3
  {0, 0, 0, 0, 0, 1, 0, 0}, // 4
  {0, 0, 0, 0, 0, 1, 0, 1}, // 5
  {0, 0, 0, 0, 0, 1, 1, 0}, // 6
  {0, 0, 0, 0, 0, 1, 1, 1}, // 7
  {0, 0, 0, 0, 1, 0, 0, 0}, // 8
};

//This is the basic agreement, which describes what lights are required
//to turn on the LED matrix. This scheme is updated after each temperature reading. 
//Each row in the schema represents a column in the leather matrix.
bool scheme[][8] = 
{
  {0, 0, 0, 1, 1, 1, 1, 1}, // Left-most column
  {0, 0, 0, 1, 0, 0, 0, 1},
  {0, 0, 0, 1, 1, 1, 1, 1},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 1, 1, 1, 1, 1},
  {0, 0, 0, 1, 0, 0, 0, 1},
  {0, 0, 0, 1, 1, 1, 1, 1} // Right-most column
};

// Patterns of digits to show on the matrix
// Each row in the matrix represents a decimal digit, 
// 3 columns wide and 5 lines long (when lit on the LED matrix)
bool digits[][3][5] = 
{
  {{1, 1, 1, 1, 1}, {1, 0, 0, 0, 1}, {1, 1, 1, 1, 1}}, // 0
  {{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {1, 1, 1, 1, 1}}, // 1
  {{1, 1, 1, 0, 1}, {1, 0, 1, 0, 1}, {1, 0, 1, 1, 1}}, // 2
  {{1, 0, 1, 0, 1}, {1, 0, 1, 0, 1}, {1, 1, 1, 1, 1}}, // 3
  {{0, 0, 1, 1, 1}, {0, 0, 1, 0, 0}, {1, 1, 1, 1, 1}}, // 4
  {{1, 0, 1, 1, 1}, {1, 0, 1, 0, 1}, {1, 1, 1, 0, 1}}, // 5
  {{1, 1, 1, 1, 1}, {1, 0, 1, 0, 1}, {1, 1, 1, 0, 1}}, // 6
  {{0, 0, 0, 0, 1}, {0, 0, 0, 0, 1}, {1, 1, 1, 1, 1}}, // 7
  {{1, 1, 1, 1, 1}, {1, 0, 1, 0, 1}, {1, 1, 1, 1, 1}}, // 8
  {{1, 0, 1, 1, 1}, {1, 0, 1, 0, 1}, {1, 1, 1, 1, 1}}, // 9
};

void setup() 
{
  pinMode(DIN,OUTPUT);
  pinMode(CS,OUTPUT);
  pinMode(CLK,OUTPUT);
  Serial.begin(115200);

  initMatrix();
  showScheme();
}

void loop() 
{
  float temp = temperature_sensor();
  
  //Because there are 2 digits on the matrix
  if (temp >= 0 && temp < 100) 
  { 
    int int_temp = temp;  					 // The integer part of the temperature
    double temp_fraction = temp - int_temp;  // The fractional part of the temperature
    
	// Set left & right digits on the matrix
	void set_digits((int_temp / 10),(int_temp % 10));
	
    //Set the size of the line at the bottom of the matrix
	etQuarters(temp_fraction*4 + 1);
 
	// Show the new temperature on the matrix
    showScheme(); 
  }

  delay(500);
}

//function which reads the temperature from the sensor
float temperature_sensor()
{
  int Vo = analogRead(ThermistorPin);
  float R2 = R1 * (1023.0 / (float)Vo - 1.0); 						  // Calculate resistance on thermistor
  float logR2 = log(R2);
  float temperature = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2)); // Temperature in Kelvin
  temperature = temperature - 273.15; 						   		  // convert Kelvin to Celcius
  Serial.println(temperature); 										  // Print the temperature (for debug purposes)	
  
  return temperature;
}

//Write the scheme to the matrix
void showScheme() 
{
  for (int i = 0; i < 8; i++) 		// For each column of the matrix
  { 
    for (int j = 0; j < 8; j++)		// Write the number of the column (the first 8 bits)
	{
      writeBit(cols[i][j]);
    }

	//Write which LEDs to turn on in each column (the last 8 bits)
    for (int k = 0; k < 8; k++) 
	{ 
      writeBit(scheme[i][k]);
    }

    latchBuf();
  }
}

//This function Copies both digits pattern from the digits array to the scheme
void set_digits(int l_digit, int r_digit)
{
	for (int i = 0; i < 5; i++) 
	{
		scheme[0][i + 3] = digits[l_digit][0][i];
		scheme[1][i + 3] = digits[l_digit][1][i];
		scheme[2][i + 3] = digits[l_digit][2][i];
		
		scheme[5][i + 3] = digits[r_digit][0][i];
		scheme[6][i + 3] = digits[r_digit][1][i];
		scheme[7][i + 3] = digits[r_digit][2][i];
  }
}

//The function controls the line, which represents quarters of degrees Celsius, 
//at the bottom of the matrix.
//Each quarter of Celsius is described by 2 LEDs on the line.
void setQuarters(int quarters) 
{
  for (int i = 0; i < quarters; i++) 
  {
    scheme[i * 2][0] = 1;
    scheme[i * 2 + 1][0] = 1;
  }

  for (int i = 0; i < 4 - quarters; i++) 
  {
    scheme[7 - i * 2][0] = 0;
    scheme[6 - i * 2][0] = 0;
  }
}

//This function Initializes matrix configurations
//Set registers: decode mode, scan limit, and shutdown (0x900, 0xB07, 0xC01)
void initMatrix() 
{  
  for (int i=0; i<4; i++) writeBit(LOW);
  writeBit(HIGH);
  for (int i=0; i<2; i++) writeBit(LOW);
  writeBit(HIGH);
  for (int i=0; i<8; i++) writeBit(LOW);
  latchBuf();  
  for (int i=0; i<4; i++) writeBit(LOW);
  writeBit(HIGH);
  writeBit(LOW);
  writeBit(HIGH);
  writeBit(HIGH);
  for (int i=0; i<5; i++) writeBit(LOW);
  for (int i=0; i<3; i++) writeBit(HIGH);
  latchBuf();  
  for (int i=0; i<4; i++) writeBit(LOW);
  for (int i=0; i<2; i++) writeBit(HIGH);
  for (int i=0; i<2; i++) writeBit(LOW);
  for (int i=0; i<7; i++) writeBit(LOW);
  writeBit(HIGH);
  latchBuf();
}

//Write 1 bit to the buffer 
void writeBit(bool b) 
{
  digitalWrite(DIN,b);
  digitalWrite(CLK,LOW);
  digitalWrite(CLK,HIGH);
}

//Latch the entire buffer
void latchBuf() 
{
  digitalWrite(CS,LOW);
  digitalWrite(CS,HIGH);
}
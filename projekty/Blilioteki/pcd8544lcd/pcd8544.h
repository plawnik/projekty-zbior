
/*******************************************************************************************************

 _____________________
| 	    PCD8544		  |		
|	 84x48 pixels	  |		
| 38,5x35,5mm(30x22mm)|		
| 6x14char (6x8pixel) |		
|					  |		
|					  |		
|					  |		
| ____________________|	
	  |TOP VIEW|


 _____________________
|  |1|2|3|4|5|6|7|8|  |		1 VDD - supply 2,7...3,3V
|					  |		2 SCLK(CLK) - serial clock
|					  |		3 SDIN(DIN) - data
|					  |		4 D/C(DC) - data/command select (set - data, clear - command)
|					  |		5 SCE(CE) - enable/disable (set - disable, clear - enable)
|					  |		6 GND
|					  |		7 VOUT - Ouptut voltage. Add external 1-10 uF electrolytic capacitor from VOUT to GND
|_____________________|		8 RESET(RST) - external reset (clear - active)
	 |BOTTOM VIEW|


*******************************************************************************************************/



#ifndef PCD8544_H_
#define PCD8544_H_




//----------------------------------------------------------------------------------------
//****************************************************************************************
//*																						 *
//*		U S T A W I E N I A   P O L A C Z E N											 *
//*			WYSWIETLACZA Z PROCESOREM													 *
//*																						 *
//****************************************************************************************
//----------------------------------------------------------------------------------------
#define LCD_CLKPORT  A		// clock
#define LCD_CLK 0

#define LCD_DINPORT  A		// data
#define LCD_DIN 1

#define LCD_DCPORT  A		// data/command select
#define LCD_DC 2

#define LCD_CEPORT  A		// enable(low)
#define LCD_CE 3

#define LCD_RSTPORT  A		// reset(low)
#define LCD_RST 4

//------------------------------------------------  koniec ustawień polaczen ---------------









//----------------------------------------------------------------------------------------
//****************************************************************************************
//*																						 *
//*		U S T A W I E N I A   KOMPILACJI												 *
//*																						 *
//*		1 - oznacza WŁĄCZENIE do kompilacji												 *
//*		0 - oznacza wyłączenie z kompilacji 											 *
//*																						 *
//****************************************************************************************
//----------------------------------------------------------------------------------------


#define USE_PCD_CHAR			1			// wysyła pojedynczy znak jako argument funkcji

#define USE_PCD_STR 			1			// wysyła string umieszczony w pamięci RAM
#define USE_PCD_STR_P 			1			// wysyła string umieszczony w pamięci FLASH
#define USE_PCD_STR_E 			1			// wysyła string umieszczony w pamięci EEPROM

#define USE_PCD_INT 			1			// wyświetla liczbę dziesietną na LCD, automatycznie dołącza pcd_str
#define USE_PCD_HEX 			1			// wyświetla liczbę szesnastkową na LCD, automatycznie dołącza pcd_str
#define USE_PCD_BIN				1			// wyświetla liczbę binarną na LCD, automatycznie dołącza pcd_str
	

#define USE_PCD_CONTRAST 		1			// ustawia kontrast - wartosci 0 - 127
#define USE_PCD_INVERT			1			// 
#define USE_PCD_HOME 			1			// ustawia kursor w pozycji 0 , 0
#define USE_PCD_LOCATE 			1			// ustawia kursor w zadanej pozycji (wiersze 6(0 do 5),kolumny 14(0 do 13))
#define USE_PCD_LOCATE_EX 		1			// ustawia kursow w zadanej pozycji (wiersze 6(0 do 5),pixele 84(0 do 83))

//------------------------------------------------  koniec ustawień kompilacji ---------------








//******************************************************************************************************
//										DEFINICJE POMOCNICZE
//******************************************************************************************************
// *** PORT
#define PORT(x) SPORT(x)
#define SPORT(x) (PORT##x)

// *** DDR
#define DDR(x) SDDR(x)
#define SDDR(x) (DDR##x)



#define LCD_CLK_SET PORT(LCD_CLKPORT)|=(1<<LCD_CLK)			// CLK = 1
#define LCD_CLK_CLR PORT(LCD_CLKPORT)&=~(1<<LCD_CLK)		// CLK = 0

#define LCD_DC_SET PORT(LCD_DCPORT)|=(1<<LCD_DC)			// DC = 1
#define LCD_DC_CLR PORT(LCD_DCPORT)&=~(1<<LCD_DC)			// DC = 0

#define LCD_DIN_SET PORT(LCD_DINPORT)|=(1<<LCD_DIN)			// DIN = 1
#define LCD_DIN_CLR PORT(LCD_DINPORT)&=~(1<<LCD_DIN)		// DIN = 0

#define LCD_CE_SET PORT(LCD_CEPORT)|=(1<<LCD_CE)			// CE = 1
#define LCD_CE_CLR PORT(LCD_CEPORT)&=~(1<<LCD_CE)			// CE = 0

#define LCD_RST_SET PORT(LCD_RSTPORT)|=(1<<LCD_RST)			// RST = 1
#define LCD_RST_CLR PORT(LCD_RSTPORT)&=~(1<<LCD_RST)		// RST = 0



#define PCD_DATA 0
#define PCD_COMMAND 1

#define TRUE 1
#define FALSE 0
//------------------------------------------------  koniec definicji pomocniczych ---------------




//******************************************************************************************************
//										DEKLARACJE FUNKCJI
//******************************************************************************************************
void pcd_write_byte(uint8_t dc , uint8_t data);		// na stałe włączona do kompilacji
void pcd_init(void);								// na stałe włączona do kompilacji
void pcd_cls(void); 								// na stałe włączona do kompilacji



void pcd_str(char * str);
void pcd_str_P(const char * str);
void pcd_str_E(char * str);

void pcd_int(int a);
void pcd_hex(int a);
void pcd_bin(int a);

void pcd_char(char symbol);

void pcd_contrast(uint8_t contrast); //0xC6
void pcd_home(void);
void pcd_locate(uint8_t y , uint8_t x);
void pcd_locate_ex(uint8_t y , uint8_t x);
void pcd_invert(uint8_t onof);



//------------------------------------------------  koniec deklaracji funkcji ------------------------



#endif /* LCD_H_ */

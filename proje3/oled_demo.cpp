

#include <fstream>
#include <iostream>
#include <wiringPi.h>
#include "ArduiPi_OLED_lib.h"
#include "Adafruit_GFX.h"
#include "ArduiPi_OLED.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <sys/wait.h> /* for wait */
#include <curl/curl.h>

#define PRG_NAME        "oled_demo"
#define PRG_VERSION     "1.1"

#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#define irInput1 28
#define irInput2 29

using namespace std;
int  counter = 0;
int  timer = 0 ;
int  inp1_control= 0;
int  inp2_control =0;

// Instantiate the display
ArduiPi_OLED display;


// Config Option
struct s_opts
{
	int oled;
	int verbose;
} ;

int sleep_divisor = 1 ;
	
// default options values
s_opts opts = {
	OLED_ADAFRUIT_SPI_128x32,	// Default oled
  false										// Not verbose
};

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
int sock,sockfd;
char type=' ';

#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 
static unsigned char logo16_glcd_bmp[] =
{ 0b00000000, 0b11000000,
  0b00000001, 0b11000000,
  0b00000001, 0b11000000,
  0b00000011, 0b11100000,
  0b11110011, 0b11100000,
  0b11111110, 0b11111000,
  0b01111110, 0b11111111,
  0b00110011, 0b10011111,
  0b00011111, 0b11111100,
  0b00001101, 0b01110000,
  0b00011011, 0b10100000,
  0b00111111, 0b11100000,
  0b00111111, 0b11110000,
  0b01111100, 0b11110000,
  0b01110000, 0b01110000,
  0b00000000, 0b00110000 };


void testdrawbitmap(const uint8_t *bitmap, uint8_t w, uint8_t h) {
  uint8_t icons[NUMFLAKES][3];
  srandom(666);     // whatever seed
 
  // initialize
  for (uint8_t f=0; f< NUMFLAKES; f++) {
    icons[f][XPOS] = random() % display.width();
    icons[f][YPOS] = 0;
    icons[f][DELTAY] = random() % 5 + 1;
    
    printf("x: %d", icons[f][XPOS]);
    printf("y: %d", icons[f][YPOS]);
    printf("dy: %d\n", icons[f][DELTAY]);
  }

  while (1) {
    // draw each icon
    for (uint8_t f=0; f< NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS], logo16_glcd_bmp, w, h, WHITE);
    }
    display.display();
    usleep(100000/sleep_divisor);
    
    // then erase it + move it
    for (uint8_t f=0; f< NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS],  logo16_glcd_bmp, w, h, BLACK);
      // move it
      icons[f][YPOS] += icons[f][DELTAY];
      // if its gone, reinit
      if (icons[f][YPOS] > display.height()) {
	icons[f][XPOS] = random() % display.width();
	icons[f][YPOS] = 0;
	icons[f][DELTAY] = random() % 5 + 1;
      }
    }
   }
}


void testdrawchar(void) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  for (uint8_t i=0; i < 168; i++) {
    if (i == '\n') continue;
    display.write(i);
    if ((i > 0) && (i % 21 == 0))
      display.print("\n");
  }    
  display.display();
}

void testdrawcircle(void) {
  for (int16_t i=0; i<display.height(); i+=2) {
    display.drawCircle(display.width()/2, display.height()/2, i, WHITE);
    display.display();
  }
}

void testfillrect(void) {
  uint8_t color = 1;
  for (int16_t i=0; i<display.height()/2; i+=3) {
    // alternate colors
    display.fillRect(i, i, display.width()-i*2, display.height()-i*2, color%2);
    display.display();
    color++;
  }
}

void testdrawtriangle(void) {
  for (int16_t i=0; i<min(display.width(),display.height())/2; i+=5) {
    display.drawTriangle(display.width()/2, display.height()/2-i,
                     display.width()/2-i, display.height()/2+i,
                     display.width()/2+i, display.height()/2+i, WHITE);
    display.display();
  }
}

void testfilltriangle(void) {
  uint8_t color = WHITE;
  for (int16_t i=min(display.width(),display.height())/2; i>0; i-=5) {
    display.fillTriangle(display.width()/2, display.height()/2-i,
                     display.width()/2-i, display.height()/2+i,
                     display.width()/2+i, display.height()/2+i, WHITE);
    if (color == WHITE) color = BLACK;
    else color = WHITE;
    display.display();
  }
}

void testdrawroundrect(void) {
  for (int16_t i=0; i<display.height()/2-2; i+=2) {
    display.drawRoundRect(i, i, display.width()-2*i, display.height()-2*i, display.height()/4, WHITE);
    display.display();
  }
}

void testfillroundrect(void) {
  uint8_t color = WHITE;
  for (int16_t i=0; i<display.height()/2-2; i+=2) {
    display.fillRoundRect(i, i, display.width()-2*i, display.height()-2*i, display.height()/4, color);
    if (color == WHITE) color = BLACK;
    else color = WHITE;
    display.display();
  }
}
   
void testdrawrect(void) {
  for (int16_t i=0; i<display.height()/2; i+=2) {
    display.drawRect(i, i, display.width()-2*i, display.height()-2*i, WHITE);
    display.display();
  }
}

void testdrawline() {  
  for (int16_t i=0; i<display.width(); i+=4) {
    display.drawLine(0, 0, i, display.height()-1, WHITE);
    display.display();
  }
  for (int16_t i=0; i<display.height(); i+=4) {
    display.drawLine(0, 0, display.width()-1, i, WHITE);
    display.display();
  }
  usleep(250000/sleep_divisor);
  
  display.clearDisplay();
  for (int16_t i=0; i<display.width(); i+=4) {
    display.drawLine(0, display.height()-1, i, 0, WHITE);
    display.display();
  }
  for (int16_t i=display.height()-1; i>=0; i-=4) {
    display.drawLine(0, display.height()-1, display.width()-1, i, WHITE);
    display.display();
  }
  usleep(250000/sleep_divisor);
  
  display.clearDisplay();
  for (int16_t i=display.width()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, i, 0, WHITE);
    display.display();
  }
  for (int16_t i=display.height()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, 0, i, WHITE);
    display.display();
  }
  usleep(250000/sleep_divisor);

  display.clearDisplay();
  for (int16_t i=0; i<display.height(); i+=4) {
    display.drawLine(display.width()-1, 0, 0, i, WHITE);
    display.display();
  }
  for (int16_t i=0; i<display.width(); i+=4) {
    display.drawLine(display.width()-1, 0, i, display.height()-1, WHITE); 
    display.display();
  }
  usleep(250000/sleep_divisor);
}

void testscrolltext(void) {
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10,0);
  display.clearDisplay();
  
  if (opts.oled == OLED_SH1106_I2C_128x64)
    display.print("No scroll\non SH1106");
  else
    display.print("scroll");
  display.display();
 
  display.startscrollright(0x00, 0x0F);
  sleep(2);
  display.stopscroll();
  sleep(1);
  display.startscrollleft(0x00, 0x0F);
  sleep(2);
  display.stopscroll();
  sleep(1);    
  display.startscrolldiagright(0x00, 0x07);
  sleep(2);
  display.startscrolldiagleft(0x00, 0x07);
  sleep(2);
  display.stopscroll();
}






/* ======================================================================
Function: main
Purpose : Main entry Point
Input 	: -
Output	: -
Comments: 
====================================================================== */
char Auth; // karsi tarafa ses gonderilir




/*

	Parser
*/
void parsingFunc(char ch[],char ID[], char Name[], char Surname[], char kalp[], char kan[], char seker[],char onlineDoc[],char onlineHem[],char patientNum []){

    int temp = -1;
    int temp2 = 0;
    for(int k=0; ch[k]!='\0'; k++){
        if(temp == -1){
            if(ch[k] != ','){
                ID[k] = ch[k];

	            }
	            else{
	                temp++;
           	 }
        
        
        }
        else if(temp == 0){
            if(ch[k] != ','){
                Name[temp2] = ch[k];
		temp2++;
            }
            else{
                temp++;
                temp2 = 0;
            }
        }
        else if(temp == 1){
            if(ch[k] != ','){
                Surname[temp2] = ch[k];
                temp2++;

            }
            else{
                temp++;
                temp2 = 0;
            }
        }


        else if(temp == 2){
            if(ch[k] != ','){
                kalp[temp2] = ch[k];
                temp2++;

             }
            else{
                 temp++;
                temp2 = 0;
            }
         }
        else if(temp == 3){
            if(ch[k] != ','){
                kan[temp2] = ch[k];
                temp2++;
            }
            else{
                temp++;
                temp2=0;
            }
        }
        else if(temp == 4){
            if(ch[k] != ','){
                seker[temp2++] = ch[k];

            }
            else{
                temp++;
                temp2 = 0;
            }
        }
        else if(temp == 5){
            if(ch[k] != ','){
                onlineDoc[temp2++] = ch[k];

            }
            else{
                temp++;
                temp2 = 0;
            }
        }  
  	else if(temp == 6){
            if(ch[k] != ','){
                onlineHem[temp2++] = ch[k];

            }
            else{
                temp++;
                temp2 = 0;
            }
        }
    
    else if(temp == 7){
            if(ch[k] != ','){
                patientNum[temp2++] = ch[k];

            }
            else{
                temp++;
                temp2 = 0;
            }
        }
    }

    }
void parsingHemsire(char ch[],char ID[],char OdaNo[],char name[],char surname[],char pansuman[],char tansiyon[],char onlineDoc[],char onlineHem[],char patientNum[]){
   int temp = -1;
    int temp2 = 0;
    for(int k=0; ch[k]!='\0'; k++){
        if(temp == -1){
            if(ch[k] != ','){
                ID[k] = ch[k];

	            }
	            else{
	                temp++;
           	 }
        
        
        }
        else if(temp == 0){

		if(ch[k] != ','){
			OdaNo[temp2] = ch[k];
			temp2++;
		}else{
			printf("%s\n",ID);
			temp++;
			temp2 = 0;
		
		}
        	
        
        }
        else if(temp == 1){
            if(ch[k] != ','){
                name[temp2] = ch[k];
		temp2++;
            }
            else{
		printf("%s\n",name);
                temp++;
                temp2 = 0;
            }
        }
        else if(temp == 2){
            if(ch[k] != ','){
                surname[temp2] = ch[k];
                temp2++;

            }
            else{
		printf("%s\n",surname);
                temp++;
                temp2 = 0;
            }
        }
        else if(temp == 3){
        	if(ch[k] != ','){
                pansuman[temp2] = ch[k];
                temp2++;

            }
            else{
			printf("%s\n",pansuman);
                temp++;
                temp2 = 0;
            }
        
        }
 	else if(temp == 4){
        	if(ch[k] != ','){
                tansiyon[temp2] = ch[k];
                temp2++;

            }
            else{
				printf("%s\n",tansiyon);
                temp++;
                temp2 = 0;
            }
        
        }
	else if(temp == 5){
        	if(ch[k] != ','){
                onlineDoc[temp2] = ch[k];
                temp2++;

            }
            else{
			printf("%s\n",onlineDoc);
                temp++;
                temp2 = 0;
            }
        
        }
	else if(temp == 6){
        	if(ch[k] != ','){
                onlineHem[temp2] = ch[k];
                temp2++;

            }
            else{
		printf("%s\n",onlineHem);
                temp++;
                temp2 = 0;
            }
        
        }
        else if(temp == 7){
        	if(ch[k] != ','){
                patientNum[temp2] = ch[k];
                temp2++;

            }
            else{
			printf("%s\n",patientNum);
                temp++;
                temp2 = 0;
            }
        
        }

	}
	printf("%s\n",patientNum);

}
void printInfoDisplay(char info[]){




	if(type == 'D'){	
	
	char ID[100] = "";
	char name[100]="";
	char surname[100]="";
	char kalp[100]="";
	char kan[100]="";
	char seker[100]="";
	char onlineDoc[100] = "";
	char onlineHem[100] = "";
	char patientNum[100] ="";
	parsingFunc(info,ID,name,surname,kalp,kan,seker,onlineDoc,onlineHem,patientNum);	

	display.clearDisplay(); 
	display.setCursor(0,0);
 	display.printf("D:%s/5 H:%s/5 N:%s 10:30",onlineDoc,onlineHem,patientNum);
  
  	display.drawLine(0, 10, display.width()-1, 10, WHITE);
	
	display.setCursor(0,15);
	display.printf("%s%s","Name: ",name);
	display.setCursor(0,25);
	display.printf("%s%s","Surname: ",surname);
	display.setCursor(0,35);
	display.printf("%s%s","Heart:  ",kalp);
	display.setCursor(0,45);
	display.printf("%s%s","Blood:  ",kan);
	display.setCursor(0,55);
	display.printf("%s%s","Glikoz: ",seker);
	display.display();
	
	
	
	}else if(type == 'H'){	
		printf("hemsire olabilir\n");
		char ID[100] = "";
		char OdaNo[100]="";
		char name[100]="";
		char surname[100]="";		
		char pansuman[100]="";	//pasusumana kalan dakika 
		char tansiyon[100]="";	// tansiyona kalan dakika
		char onlineDoc[100] = "";
		char onlineHem[100] = "";
		char patientNum[100] ="";	//toplam hasta sayisi
		parsingHemsire(info,ID,OdaNo,name,surname,pansuman,tansiyon,onlineDoc,onlineHem,patientNum);
		display.clearDisplay(); 
		display.setCursor(0,0);
	 	display.printf("D:%s/5 H:%s/5 N:%s 10:30",onlineDoc,onlineHem,patientNum);
	  
	  	display.drawLine(0, 10, display.width()-1, 10, WHITE);
		
		display.setCursor(25,15);
		display.printf("%s %s",name,surname);
		display.setCursor(0,25);
		display.printf("%s%s","Oda: ",OdaNo);
		display.setCursor(0,35);
		display.printf("%s%s dk","Pansuman: ",pansuman);
		display.setCursor(0,45);
		display.printf("%s%s dk","Tansiyon: ",tansiyon);
		display.display();
	
	}





}
void sendAndGet(int *take)
{
	char buff[1024]="";
	char iste[1024] ="";
	//int take = ; once take gonderilecek
	sprintf(iste,"%d",(*take));
	send(sock,iste,sizeof(iste),0);
	
	recv(sock,buff,sizeof(buff),0);
	printf("buffer %s\n",buff);
	printInfoDisplay(buff);
}
std::string buffer;
size_t curl_write( void *ptr, size_t size, size_t nmemb, void *stream)
{
    buffer.append((char*)ptr, size*nmemb);
    return size*nmemb;
}


int main(int argc, char **argv)
{

	
	
	

	int i;
	
	opts.oled = 6;//SH1106 128X64
	
		// I2C change parameters to fit to your LCD
		if ( !display.init(OLED_I2C_RESET,opts.oled) )
			exit(EXIT_FAILURE);
	

	display.begin();
	

  // init done
  	display.clearDisplay();   // clears the screen  buffer
 	
 	/* Hemsire parser deneme
 	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(0,0);
	char tempHem[] = "15,2,Ridvan,Demirci,25,5,4,3,2";
	
	
	type = 'H';
	printInfoDisplay(tempHem);
sleep(10);*/
	
	struct sockaddr_in server;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	server.sin_family = AF_INET;
	server.sin_port = htons(8081);
	server.sin_addr.s_addr = inet_addr("192.168.43.48");
	

	wiringPiSetup () ;
	pinMode (irInput1, INPUT) ;
	pinMode (irInput2, INPUT) ;
	struct timeval startTime, currentTime;
	gettimeofday(&startTime,NULL);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
	for ( i =0 ; i<=100 ; i+=10)
	{
		display.clearDisplay();
		display.setCursor(0,0);
		display.print("Yukleniyor!\n");
		display.printf("  %03d %%", i);
		display.drawHorizontalBargraph(0,16, (int16_t) display.width(),16,1, i);
		display.display();
	}

  int countTemp =0;
  
	
while(countTemp<15){
  	if(connect(sock,(struct sockaddr *)&server,sizeof(server))<0){
  		display.clearDisplay();
		display.setCursor(0,10);
		display.printf("BAGLANTI BEKLENIYOR");
		display.setCursor(0,20);
		//display.display();
		for(int i = 0 ; i<countTemp ; i++){

  		display.printf(".");
		}
		
		
  		display.display();
  		countTemp++;
  	}
  	else{
  		break;
  	
  	}
	
	sleep(1);	
}
if(countTemp>=15){

	display.clearDisplay();
	display.setCursor(5,10);
	display.printf("BAGLANTI yapilamiyor");
	display.display();
	exit(1);

}
		 display.clearDisplay();
		 display.setCursor(10,10);
		 display.printf("BAGLANTI BASARILI");
		 display.display();
		sleep(1);

	// text display tests
  display.clearDisplay();
  
  char buff[1024]="";
  recv(sock,buff,sizeof(buff),0);//online kisi sayisi beklenir
  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.printf("D:%c/5 H:%c/5 N: 10:30",buff[0],buff[2]);
  
  display.drawLine(0, 10, display.width()-1, 10, WHITE);
  		
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,30);
  
  display.printf("\n");
  display.printf("Sesli giris icin mic kullan");  
  display.display();
  
  

  //Authentication sagla
  sleep(1);

  
  while(1){	
 
  pid_t pid=fork();

    if (pid==0){       /* child process */        
        char *args[] = {"arecord", "-f", "S16_LE", "-D", "plughw:1,0", "-r", "16000", "-d", "5","test.wav",0 };
        display.clearDisplay();
	display.setCursor(0,30);  
  	display.printf("Konusabilirsiniz:!\n");	
  	display.display();

        if (execvp(args[0], args) == -1)
            perror("hata");

        exit(127); /* only if execv fails */
    }
    else{   /* pid!=0; parent process */
        waitpid(pid,0,0); /* wait for child to exit */
   	
    }
    display.clearDisplay();
	display.setCursor(0,30);  
  	display.printf("Ses Donusturuluyor!\n");	
  	display.display();
    CURL *hnd = curl_easy_init();
   
    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(hnd, CURLOPT_URL, "https://speech.googleapis.com/v1/speech:recognize?key=AIzaSyBGGfbdYGdva4AYWdA0vkyCVX-z7-Fq5IY");

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Postman-Token: 0a837de6-5412-45d2-adea-04b9bdc5b2d1");
    headers = curl_slist_append(headers, "Cache-Control: no-cache");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, headers);

    pid = fork();
    if(pid == 0){

        char *argv3[]={"convert","-e","test.wav","out.txt",NULL}; // classic
        execve("convert",argv3,NULL);
        exit(1);
    }
    else if(pid < 0){
    	printf("error fork\n");
    
    }
    wait(NULL);
    printf("file olusturuldu\n");


    string content;
    ifstream base64("out.txt");
    string temp,temp1;

    while(getline(base64,temp1)){
        temp+=temp1;

    }
    content ="{\n  \"config\": {\n    \"encoding\":\"ENCODING_UNSPECIFIED\",\n    \"languageCode\":\"tr-TR\"\n  },\n  \"audio\": {\n   \"content\": \""+ temp +"\"},\n}";
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, content.c_str());

    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, curl_write);
    CURLcode ret = curl_easy_perform(hnd);
    curl_easy_cleanup(hnd);
    fwrite( buffer.c_str(), buffer.length(), sizeof(char), stdout);

     string message = "";
     if(buffer.length() >10){

     unsigned long last = buffer.find('"',82);
    // cout << last<<endl;
     message = buffer.substr(82,last-82);
     printf("%s\n",message.c_str());
      display.clearDisplay();
	display.setCursor(0,30);  
  	display.printf("%s",message.c_str());	
  	display.display();                                                                           
     }
  
  
  
  sleep(1);
  Auth = '1'; // ses dinlemesi yapacak oncesinde 1 gonder hemsire yarın 
  printf("%c\n",Auth);
  send(sock,&Auth,sizeof(Auth),0);// karsi tarafin ses dinlemesi yapilir
  send(sock,message.c_str(),strlen(message.c_str()),0);//Authentication dogru ise type beklenit(H,D)
  printf("gonderildi\n");
  
  recv(sock,&type,sizeof(type),0);//Authentication dogru ise type beklenit(H,D)
  fprintf(stderr,"%c\n",type);
  if(type != '0')
  	break;
  else{
  	display.clearDisplay();
	display.setCursor(0,30);  
  	display.printf("Giris basarisiz\nTekrar giris yapiniz!");	
  	display.display();
  	buffer = "";
  	sleep(1);
  }
  
  }
  int take = 0;
  sendAndGet(&take);// dosya alınıp verilir
  
  
  
  
  
  
   //int take=0;
   char notBlock[1024]="";
	while(1){
		int ret = recv(sock,notBlock,sizeof(notBlock),MSG_DONTWAIT);
		if(ret > 0){
		 	printf("%s\n",notBlock);
			display.clearDisplay();
			display.setCursor(0,0);
			display.printf("D:%s/5 H:%s/5 N: 10:30","5","4");			  
			display.drawLine(0, 10, display.width()-1, 10, WHITE);			  		
			display.setTextSize(1);
			display.setTextColor(WHITE);
  			display.setCursor(0,15);
	  		display.printf("%s",notBlock);	
	  		display.display();
	  		
		
		}
		//printf("blocklanmadi\n");
		    
		
		
		timer +=1 ;
		gettimeofday(&startTime,NULL);	
		if(digitalRead(irInput1) == 0 ){
			counter +=1 ;
			inp1_control = timer ;
			//printf("Input 1\n");
		}
		if(digitalRead(irInput2) == 0){
			inp2_control = timer ; 
			//printf("Input 2\n");
		}
		
		if(     inp1_control < inp2_control &&  inp1_control != 0 &&  inp2_control != 0     ){
			printf("                    Sağ yönelim                               -\n ");
			sleep(1);
			counter =0;
			timer =0;
			inp1_control = 0;
			inp2_control = 0;
			if(type == 'D')
				Auth = '2';//hasta istemcisi
			else if(type == 'H')
				Auth = '8'; // hemsire hasta ister
			send(sock,&Auth,sizeof(Auth),0);
			take++;	// hangi hasta istendigi
			sendAndGet(&take);
		}
		if ( inp1_control > inp2_control &&  inp1_control != 0 &&  inp2_control != 0 ){
			printf(" Sol yönelim                                                  --\n");
			sleep(1);
			counter = 0 ;
			timer  = 0;
			inp1_control = 0;
			inp2_control = 0;
			if(type == 'D')
				Auth = '2';//hasta istemcisi
			else if(type == 'H')
				Auth = '8'; // hemsire hasta ister
			send(sock,&Auth,sizeof(Auth),0);
			take--;
			sendAndGet(&take);
			
			
						   
		}
		
		
		if (digitalRead(irInput2) == 0 ){
		
			while(digitalRead(irInput2) == 0) {
				gettimeofday(&currentTime,NULL);
				//printf("bekliyorum %d\n",(currentTime.tv_sec - startTime.tv_sec));
				if((currentTime.tv_sec - startTime.tv_sec) >= 2){
					printf("-----       ses Kayıt     -----\n");
					
					pid_t pid=fork();

    if (pid==0){       /* child process */        
        char *args[] = {"arecord", "-f", "S16_LE", "-D", "plughw:1,0", "-r", "16000", "-d", "5","test.wav",0 };
        display.clearDisplay();
	display.setCursor(0,30);  
  	display.printf("Konusabilirsiniz!");	
  	display.display();

        if (execvp(args[0], args) == -1)
            perror("hata");

        exit(127); /* only if execv fails */
    }
    else{   /* pid!=0; parent process */
        waitpid(pid,0,0); /* wait for child to exit */
   	
    }
	display.clearDisplay();
	display.setCursor(0,30);  
  	display.printf("Ses Donusturuluyor");	
  	display.display();
    CURL *hnd = curl_easy_init();
   
    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(hnd, CURLOPT_URL, "https://speech.googleapis.com/v1/speech:recognize?key=AIzaSyBGGfbdYGdva4AYWdA0vkyCVX-z7-Fq5IY");

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Postman-Token: 0a837de6-5412-45d2-adea-04b9bdc5b2d1");
    headers = curl_slist_append(headers, "Cache-Control: no-cache");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, headers);

    pid = fork();
    if(pid == 0){

        char *argv3[]={"convert","-e","test.wav","out.txt",NULL}; // classic
        execve("convert",argv3,NULL);
        exit(1);
    }
    else if(pid < 0){
    	printf("error fork\n");
    
    }
    wait(NULL);
    printf("file olusturuldu\n");


    string content;
    ifstream base64("out.txt");
    string temp,temp1;

    while(getline(base64,temp1)){
        temp+=temp1;

    }
    buffer = "";
    content ="{\n  \"config\": {\n    \"encoding\":\"ENCODING_UNSPECIFIED\",\n    \"languageCode\":\"tr-TR\"\n  },\n  \"audio\": {\n   \"content\": \""+ temp +"\"},\n}";
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, content.c_str());

    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, curl_write);
    CURLcode ret = curl_easy_perform(hnd);
    curl_easy_cleanup(hnd);
    fwrite( buffer.c_str(), buffer.length(), sizeof(char), stdout);


     	

      string message = "";
     if(buffer.length() >10){

     unsigned long last = buffer.find('"',82);
    // cout << last<<endl;
     message = buffer.substr(82,last-82);
     printf("%s\n",message.c_str());
                                                                                 
     }
  
  
  
  sleep(1);
  if(type == 'D')
	Auth = '5'; // doktor icin ses dinlemesi yapilir
  else if(type == 'H')
	Auth = '6'; //hemsire icin ses dinlemesi yapilir
  send(sock,&Auth,sizeof(Auth),0);
  send(sock,message.c_str(),strlen(message.c_str()),0);//Authentication dogru ise type beklenit(H,D)
  printf("gonderildi\n");  
					
					sleep(3);
					break;
				}

			}
		}




	}
    

	
	
	
		



  

}



#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <time.h>
#include <my_global.h>
#include <mysql.h>
#include <string.h> //łączenie napisów



#define ON  1
#define OFF 0
#define PIN_RELAY 17 
#define TEMPERATURE_REQUESTED 25
#define DELAY 60*1000-19

char * pathToSensorFile = "/sys/bus/w1/devices/28-000008d6b7f8/w1_slave" ;
char * pathToLogFile = "/media/pi/KINGSTON/log.txt";


float readData() {
	FILE *fp;
	
	if((fp = fopen(pathToSensorFile, "r")) == NULL){
		printf("ERROR in readData function, file error, check connection to sensor \n");
		exit(1);
	}
	int i;
	for( i=0; i<69;i++){	//scan and reject early data
		char a;
		fscanf(fp, "%*c", &a);
	}

	char numberString[5];	//scan necceseary data
	for(i=0; i<5 ; i++) {
		fscanf(fp, "%c", &numberString[i] );
	}
	fclose(fp);
	return atof(numberString) /1000 ;
}


int onOffRelay(int PinRelay, int onOff){

	if(wiringPiSetupGpio() == -1){
		puts("fail to start wiringPiSetup()");
		exit(1);
	}

	pinMode (PinRelay, OUTPUT );	//sets pin to out
	if (onOff == ON)
		digitalWrite (PinRelay, HIGH );		//sets pin high
	else if (onOff == OFF)
		digitalWrite (PinRelay, LOW);		//sets pin low
	else{
		puts ("ERROR in onOfffRelay, onOff must be ON or OFF equals 1 or 0");
		exit(1);
	}	

	return 0;
}

void WriteDataToFile(float data){
	FILE *fpointer;
	if((fpointer = fopen(pathToLogFile, "a")) == NULL){
		printf("ERROR in writeData function, file error, check if Pendrive connected \n");
	}
	time_t nowTime;
	time( & nowTime);
	fprintf(fpointer, "%.1f  [st. C], date:  %s", data, ctime( & nowTime));
	fclose(fpointer);
}
	

int main(){

while(1){




	float actualTemp;
	
	
	actualTemp = readData();
	printf("%f\n", actualTemp);

	MYSQL mysql; 

	if(mysql_init(&mysql)==NULL) { 
	       printf("\nInitialization error\n"); 
	        return 0; 
		    } 

	    if( ( mysql_real_connect(&mysql,"localhost","root","T3kturowy","Kira",0,NULL,0))==NULL){
			puts("error in mysql real connect");
			exit(1);
		}

	    printf("Client version: %s",mysql_get_client_info()); 
	    printf("\nServer version: %s\n",mysql_get_server_info(&mysql));
	
		char str1[200]="UPDATE `Kira`.`data` SET `temp` = '";
		char str2[10];
		sprintf(str2, "%.2f", actualTemp );
		char str3[]="' WHERE `data`.`id` = 1";
		strcat(str1, str2);
		strcat(str1, str3);
		puts("send sql querry is: ");
		puts(str1);



		if (mysql_query(&mysql, str1/*"UPDATE `Kira`.`data` SET `temp` = '200' WHERE `data`.`id` = 1"*/)) {
		      puts("error querry");
				exit(1);
			    }

	    mysql_close(&mysql); 



		

	

delay(DELAY);

}
}

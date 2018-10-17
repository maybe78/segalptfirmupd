//**********************************************************
//* Flash Memory Programming v.1.0 
//* LPT data access        
//**********************************************************
#include "iostream.h"
#include "conio.h"
#include "stdlib.h"
#include "stdio.h"
#include "h.h"
#include <windows.h>
#include <fstream.h>

short cp_data=0xFF;
short cp_data1=0xFF;


HANDLE handle;
DWORD readed=7878;
char word[2097152] = "BBB";


//************* Reset FlipFlop *************//
void reset_ff()
{
Out32(890,0x0C); //00001100 to Control (set R)
Out32(890,0x04); //00000100 to Control (release R)
}

//************* Write to FlipFlop *************//
void write_ff(short x)
{
Out32(892,x);    // write data to FF
Out32(890,0x04); // 00000100   release C 
Out32(890,0x00); // 00000000 set C 
}

//************* Reset IO Ports *************//
void reset_io()
{
	write_ff(4); // 00000100 set RESET
	write_ff(0); // 00000000 release RESET
}

//**************** Select port***************//
void select_pa(int cs)
{
if (cs==0) write_ff(0); //00000000 A0=0 A1=0 CS=0
if (cs==1) write_ff(8); //00001000 A0=0 A1=0 CS=1			
};

void select_pb(int cs)
{
if (cs==0)	write_ff(1); //00000001 A0=1 A1=0 CS=0
if (cs==1)  write_ff(9); //00001001 A0=1 A1=0 CS=1
};

void select_pc(int cs)
{
if (cs==0)	write_ff(2);  //00000010 A0=0 A1=1 CS=0
if (cs==1)  write_ff(10); //00001010 A0=0 A1=1 CS=1
};

  
void select_cw(int cs)
{
if (cs==0) write_ff(3); // 00000011 A0=1 A1=1 CS=0
if (cs==1) write_ff(11); //00001011 A0=1 A1=1 CS=1
};
//************* end of selection *************//

//************* Set IO Ports to READ mode  *************//
void control_port_read(int cs)
{
short data;
select_cw(cs);
Out32(890,1); // Set RD=1
data=Inp32(892); //read cw
Out32(890,0); //Set RD=0
if (cs==0) data=0x80; // 10000000 A,B,C ->
if (cs==1) data=0x9A; // 10011010 A,B, C4-7 <- | C0-3 ->
Out32(892,data);
Out32(890,2); //set wr=1
Out32(890,0); //set wr=0
}

//************* Set IO Ports to WRITE mode  *************//
void control_port_write(int cs)
{
short data;
select_cw(cs);

Out32(890,1); // Set RD=1
data=Inp32(892); //read cw
Out32(890,0); //Set RD=0
if (cs==0) data=0x80; //10000000 A,B,C -> 
if (cs==1) data=0x88; //10001000 a,b, c03-> | c47 <-
Out32(892,data);
Out32(890,2); //set wr=1
Out32(890,0); //set wr=0
}

//************* Write DATA to port    *************//
void data_write(int cs, char port, short data)
{

if (port=='a') select_pa(cs);
if (port=='b') select_pb(cs);
if (port=='c') select_pc(cs);
if (port=='c' && cs==0) cp_data=data;
if (port=='c' && cs==1) cp_data1=data;
Out32(892,data);
Out32(890,2); //set wr=1
Out32(890,0); //set wr=0

}

//************* Read DATA from port   *************//
short data_read(int cs, char port)
{
short data;

if (port=='a') select_pa(cs);
if (port=='b') select_pb(cs);
if (port=='c') select_pc(cs);

Out32(890,32); // Set port to read mode
Out32(890,33); // Set RD=1

data=Inp32(892);
if (port=='c' && cs==0) data=cp_data;
if (port=='c' && cs==1) data=cp_data1;
Out32(890,32); //Set RD=0
Out32(890,0); //Set RD=0

return data;
}


//*********************** SET /CE SIGNAL ************************
void set_ce_0()
{
short data;
data=data_read(0,'c');
data&=0xDF;
data_write(0,'c',data);
}

void set_ce_1()
{
short data;
data=data_read(0,'c');
data|=0x20;
data_write(0,'c',data);
}

//*********************** SET /OE SIGNAL ************************
void set_oe_0()
{
short data;
data=data_read(0,'c');
data&=0xBF;
data_write(0,'c',data);
}

void set_oe_1()
{
short data;
data=data_read(0,'c');
data|=0x40;
data_write(0,'c',data);
}
//*********************** SET /WE SIGNAL ************************
void set_we_0()
{
short data;
data=data_read(0,'c');
data&=0x7F;
data_write(0,'c',data);
}

void set_we_1()
{
short data;
data=data_read(0,'c');
data|=0x80;
data_write(0,'c',data);
}

//*********************** SET D_DIR SIGNAL ************************
void set_dir_0()
{
short data;
data=data_read(1,'c');
data&=0xFB;
data_write(1,'c',data);
}

void set_dir_1()
{
short data;
data=data_read(1,'c');
data|=0x04;
data_write(1,'c',data);
}

//*********************** SET CTR_RESET SIGNAL ************************
void set_ctr_res_0()
{
short data;
data=data_read(1,'c');
data&=0xFD;
data_write(1,'c',data);
}

void set_ctr_res()
{
short data;
data=data_read(1,'c');
data|=0x02;
data_write(1,'c',data);
}


void set_cwo()
{
set_ce_1();
set_oe_1();
set_we_1();
}




//*********************** SET ADRESS ************************
//*      процедура для выставления адреса на flash          *
//***********************************************************
void set_adress(int adress)
{
	short a,b,c;
	short datai;
	int adrt;

adrt=adress&0xFF;   // выставляем а
a=adrt;			

adrt=adress&0xFF00; // выставляем б
adrt=adrt>>8;
b=adrt;			

adrt=adress&0xFF0000; // выставляем с
adrt=adrt>>16;
c=adrt;			

datai=data_read(0,'c');

datai&=0xE0; // обнуляем 5 младших разрядов числа полученного из регистра "С",
			 // получаем CE,WE,OE в 3 старших разрядах
c&=0x1F;

c=datai+c;  // складываем два числа, так что 
			// получается число с нужными нам CE,WE,OE


data_write(0,'a',a);
data_write(0,'b',b);
data_write(0,'c',c);
}

//************************ SET DATA *************************
//*         процедура для вывода данных на flash            *
//***********************************************************
void set_data(int adress, int data)
{
	int a,b;
	int datat;
	int idata;

set_adress(adress);
set_dir_1();
set_ce_0();

datat=data&0xFF; 
a=datat;

datat=data&0xFF00;
datat=datat>>8;
b=datat;

	data_write(1,'a',a);
	data_write(1,'b',b);

set_we_0();
set_we_1();

set_ce_1();
}

//************************ READ DATA ************************
//*         процедура для чтения  данных с flash            *
//***********************************************************
int get_data(int adress)
{
	int data_a, data_b, data;

set_adress(adress);
set_dir_0();
set_ce_0();
set_oe_0();

data_a=data_read(1,'a');
data_b=data_read(1,'b');

data_b=data_b<<8;


data=data_a+data_b;
set_oe_1();
set_ce_1();

return data;
}

//****************** FLASH CHIP ERASE ******************
void chip_erase()
{
int idata;
idata=0x00;	

	set_data(0x555,0xAA);
	set_data(0xAAA,0x55);
	set_data(0x555,0x80);
	set_data(0x555,0xAA);
	set_data(0xAAA,0x55);
	set_data(0x555,0x10);

while (idata==0x00) {
idata=data_read(1,'c');
idata&=0xEF;
};

}
//****************** FLASH SECTOR ERASE ******************
void sector_erase(int adress)
{
	set_data(0x555,0xAA);
	set_data(0xAAA,0x55);
	set_data(0x555,0x80);
	set_data(0x555,0xAA);
	set_data(0xAAA,0x55);
	set_data(adress,0x30);
}

//****************** FLASH WRITE ******************
void flash_write(int adress, int data)
{
int idata;
int tdata=0x00;
idata=0x00;

while (idata==0x00) {
idata=data_read(1,'c');
idata&=0xEF;
};

    set_data(0x555,0xAA);
	set_data(0xAAA,0x55);
	set_data(0x555,0xA0);
	set_data(adress,data);
//	Sleep(2);
//tdata=get_data(adress);
//if (data!=tdata) printf("error while writing\n");
}
//************************ FILE OPEN *********************************

void file_open()  {

char number[25];
char filename[20];
int i=0;
int g=0;
int a,b, sum =0;


cout<<"\nInput filename:";
cin>>filename;
handle=CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
if ( handle == INVALID_HANDLE_VALUE) 
{MessageBoxA(0,"","Ошибочка вышла.",MB_OK); exit(123);};


if ( ReadFile(handle, word, 2097152, &readed, NULL) == 0) 
{MessageBoxA(0,"","Ошибочка вышла.",MB_OK); exit(123);};


//printf ("%i: %s",readed,word);

while (i<readed)
{
	a=word[i];
	b=word[i+1];
	a=a<<8;
	sum=a+b;
	flash_write(g,sum);
printf ("Written %X of %X\r",i+1,readed);


//printf ("\n_______________________\n");
//itoa(a,number,2);
//printf ("<a: %s [%X]\n",number,a);
//itoa(b,number,2);
//printf ("<b: %s [%X]\n",number,b);	
//itoa(sum,number,2);
//printf ("<s: %s [%X]\n",number,sum);		
i++; i++; g++;

}

printf ("\nComplete.");


CloseHandle(handle);

} 

void file_check()  {

char number[25];
char filename[20];
int i=0;
int g=0;
int a,b, sum =0;
int data_t=0;

cout<<"\nInput filename:";
cin>>filename;
handle=CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
if ( handle == INVALID_HANDLE_VALUE) 
{MessageBoxA(0,"","Ошибочка вышла.",MB_OK); exit(123);};


if ( ReadFile(handle, word, 2097152, &readed, NULL) == 0) 
{MessageBoxA(0,"","Ошибочка вышла.",MB_OK); exit(123);};


//printf ("%i: %s",readed,word);

while (i<readed)
{
	a=word[i];
	b=word[i+1];
	a=a<<8;
	sum=a+b;
	sum&=0x0000FFFF;


	data_t=get_data(g);
if (sum != data_t) printf("Error at %X: %X instead of %X!\n", g, data_t, sum);
i++; i++; g++;

}

CloseHandle(handle);

} 


//******************** MAIN ******************//
int main()
{
int i=0; 
int data;
char number[25];
char answer;
reset_ff();
reset_io();
control_port_write(0);
control_port_read(1);
set_cwo();

i=0x0000;

/*
while (i<10000){
data=get_data(i);
itoa(data,number,2);
printf ("<s: %s [%X]\n",number,data);	
i++;}*/

//file_check();

control_port_write(1);
printf("Do you need to erase flash memory? [Y/N]");
cin>>answer;
if (answer='Y') 
{ 
	chip_erase(); 
	printf("Chip Erasing...");
	//Sleep(33000);
	printf("Complete!");
};
set_cwo();
file_open();
printf("Check file? [Y/N]");
cin>>answer;
if (answer='Y') 
{ 
	control_port_read(1);
	file_check();
	printf("Complete checking!");
};
while(1)
	{  
	
	
	}






return true;
}


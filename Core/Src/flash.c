/*
 * flash.c
 *
 *  Created on: Feb 15, 2024
 *      Author: trand
 */

#include <flash.h>

uint8_t lengthPage;

void deleteBuffer(char * data){
	uint8_t len = strlen(data);
	for(uint8_t i=0; i<len; i++){
		data[i] = 0;
	}
}

void Flash_Lock(){
	HAL_FLASH_Lock(); // Prevents writing data to writable partitions
}

void Flash_Unlock(){
	HAL_FLASH_Unlock();
}

void Flash_Erase(uint32_t addr){
	while(FLASH->SR & FLASH_SR_BSY); // Check Status Register BUSY when conducting delete, write or process

	FLASH->CR |= FLASH_CR_PER; //Set bit Page Erase Set and OR bitwise
	FLASH->AR = addr;  //Page Address to Address Register
	FLASH->CR |= FLASH_CR_STRT; //Set bit Start Page Erase and OR bitwise

	while (FLASH->SR & FLASH_SR_BSY);
		FLASH->CR &= ~FLASH_SR_BSY; // AND + NOR bitwise
	FLASH->CR &= ~FLASH_CR_PER; //Page Erase Clear and AND + NOR bitwise
}


void Flash_Write_Int(uint32_t addr, int data){
	Flash_Unlock(); //Before writing to Flash, it need to unlock the flash
	FLASH->CR |= FLASH_CR_PG; //Set bit PG (Program) allows writing data to Flash

	while(FLASH->SR & FLASH_SR_BSY);
	*(__IO uint16_t*)addr = data;
	while(FLASH->SR & FLASH_SR_BSY);
	FLASH->CR &= ~FLASH_CR_PG; //Clear PG bit
	Flash_Lock();
}

uint16_t Flash_Read_Int(uint32_t addr)
{
	uint16_t* val = (uint16_t *)addr;
	return *val;
}

void Flash_Write_Char(uint32_t addr, char* data){
	Flash_Unlock();
	int i;
	FLASH->CR |= FLASH_CR_PG;
	int var = 0;
	lengthPage = strlen(data);
	for(i=0; i<lengthPage; i+=1){
		while(FLASH->SR & FLASH_SR_BSY);
			var = (int)data[i];
		*(__IO uint16_t*)(addr + i*2) = var;
	}

	while((FLASH->SR&FLASH_SR_BSY)){};
	FLASH->CR &= ~FLASH_CR_PG;
	FLASH->CR |= FLASH_CR_LOCK;
}

void Flash_Read_Char(char* dataOut, uint32_t addr1, uint32_t addr2){
	int check =0;
	deleteBuffer(dataOut);
	if((unsigned char) Flash_Read_Int(addr2 + (uint32_t)2) == 225){
		 check = (unsigned char)Flash_Read_Int(addr2)-48;
	}else{
		 check = ((unsigned char)Flash_Read_Int(addr2)-48)*10 + (unsigned char)Flash_Read_Int(addr2+2)-48;
	}
	for(int i=0;i<check;i++){
		dataOut[i] = Flash_Read_Int(addr1 + (uint32_t)(i*2));

	}
}


void Flash_Program_Page(char* dataIn, uint32_t addr1, uint32_t addr2){
	    //FLASH_Unlock
		Flash_Unlock();
		//Flash_Erase Page
		Flash_Erase(addr1);
		//FLASH_Program HalfWord
		Flash_Write_Char(addr1,dataIn);
		HAL_Delay(100);
		char tempbuf[5] = {0};
		sprintf(tempbuf,"%d",lengthPage);
		//FLASH_Unlock
		Flash_Unlock();
		//Flash_Erase Page
		Flash_Erase(addr2);
		//FLASH_Program HalfWord
		Flash_Write_Char(addr2,tempbuf);
}

/*
Arduino Buffered Serial
A library that helps establish buffered serial communication with a 
host application.
Copyright (C) 2010 Sigurður Örn Aðalgeirsson (siggi@media.mit.edu)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
 
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "ByteBuffer.h"


ByteBuffer::ByteBuffer(){

}

void ByteBuffer::init(unsigned  int buf_length){
	data = (uint8_t*)malloc(sizeof(uint8_t)*buf_length);
	capacity = buf_length;
	position = 0;
	length = 0;
}

void ByteBuffer::deAllocate(){
	free(data);
}

void ByteBuffer::clear(){
	position = 0;
	length = 0;
}

int ByteBuffer::getSize(){
	return length;
}

int ByteBuffer::getCapacity(){
	return capacity;
}

uint8_t ByteBuffer::peek(unsigned int index){
	return data[(position+index)%capacity];
}

int ByteBuffer::put(uint8_t in){
	if(length < capacity){
		// save data at end of buffer
		data[(position+length) % capacity] = in;
		// increment the length
		length++;
		return 1;
	}
	// return failure
	return 0;
}



int ByteBuffer::autoput(uint8_t in){
  if(length >= capacity){
    length--;
    // return failure
    //return 0;
  }
  
  // save data  at end of buffer
  if( position == 0 )
    position = capacity-1;
  else
    position = (position-1)%capacity;
  data[position] = in;
  // increment the length
  length++;
  return 1;

}


int ByteBuffer::putInFront(uint8_t in){
	if(length < capacity){
		// save data byte at end of buffer
		if( position == 0 )
			position = capacity-1;
		else
			position = (position-1)%capacity;
		data[position] = in;
		// increment the length
		length++;
		return 1;
	}
	// return failure
	return 0;
}

uint8_t ByteBuffer::get(){
	uint8_t b = 0;


	if(length > 0){
		b = data[position];
		// move index down and decrement length
		position = (position+1)%capacity;
		length--;
	}

	return b;
}

uint8_t ByteBuffer::getFromBack(){
	uint8_t b = 0;
	if(length > 0){
		b = data[(position+length-1)%capacity];
		length--;
	}

	return b;
}


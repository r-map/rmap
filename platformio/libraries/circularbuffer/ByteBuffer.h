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
 
#ifndef ByteBuffer_h
#define ByteBuffer_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

class ByteBuffer
{
public:
	ByteBuffer();

	// This method initializes the datastore of the buffer to a certain sizem the buffer should NOT be used before this call is made
	void init(unsigned int buf_size);

	// This method resets the buffer into an original state (with no data)	
	void clear();

	// This releases resources for this buffer, after this has been called the buffer should NOT be used
	void deAllocate();

	// Returns how much space is left in the buffer for more data
	int getSize();
	
	// Returns the maximum capacity of the buffer
	int getCapacity();

	// This method returns the int that is located at index in the buffer but doesn't modify the buffer like the get methods (doesn't remove the retured int from the buffer)
	uint8_t peek(unsigned int index);

	//
	// Put methods, either a regular put in back or put in front
	// 
	int autoput(uint8_t in);     // put in front and remove if needed from back
	int putInFront(uint8_t in);
	int put(uint8_t in);

	//
	// Get methods, either a regular get from front or from back
	// 
	uint8_t get();
	uint8_t getFromBack();

private:
	uint8_t* data;

	unsigned int capacity;
	unsigned int position;
	unsigned int length;
};

#endif


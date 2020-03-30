/*
  Log library for FreeRTOS
  version 1.0.0

Copyright (C) 2020  Paolo Patruno <p.patruno@iperbole.bologna.it>
authors:
Paolo Patruno <p.patruno@iperbole.bologna.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "frtosLog.h"

void frtosLogging::begin(int level, Print* logOutput, MutexStandard &semaphore, bool showLevel){
  _semaphore = semaphore;
  _logging.begin(level, logOutput, showLevel);
}

void frtosLogging::setPrefix(printfunction f){
  _logging.setPrefix(f);
}

void frtosLogging::setSuffix(printfunction f){
  _logging.setSuffix(f);
}


frtosLogging frtosLog = frtosLogging();


/**@file sim800Client.h */

/*********************************************************************
Copyright (C) 2017  Marco Baldinetti <m.baldinetti@digiteco.it>
authors:
Paolo Patruno <p.patruno@iperbole.bologna.it>
Marco Baldinetti <m.baldinetti@digiteco.it>

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
**********************************************************************/

#ifndef Client_h
#define Client_h

#include "sim800.h"

/*!
\class sim800Client
\brief sim800Client class.
*/
class sim800Client : public SIM800 {

 public:
  sim800Client();

  /*!
  \fn int connect(IPAddress ip, int port)
  \brief Connect to server at specified port.
  \param[in] ip ip address of server.
  \param[in] port port to server.
  \return sim800 status of connection sequence on each call.
  */
  int connect(IPAddress ip, int port);

  /*!
  \fn int connect(const char *host, int port)
  \brief Connect to server at specified port.
  \param[in] *host hostname of server.
  \param[in] port port to server.
  \return sim800 status of connection sequence on each call.
  */
  int connect(const char *host, int port);

  /*!
  \fn uint8_t connected()
  \brief Check if connection was performed.
  \return true if it is connected, false otherwise.
  */
  uint8_t connected();

  /*!
  \fn int available()
  \brief Return the number of bytes available in the stream.
  \return the number of bytes available.
  */
  int available();

  /*!
  \fn int read()
  \brief Reads one characters from an incoming stream.
  \return The readed characters or -1 if there isn't characters.
  */
  int read();

  /*!
  \fn int readBytes(char *buffer, size_t size)
  \brief Read characters from a stream into a buffer. Terminates if the determined length has been read, or it times out.
  \param [out] *buffer pointer to readed characters buffer.
  \param [in] size maximum number of characters to be read.
  \return the number of characters placed in the buffer. A 0 means no valid data was found.
  */
  int readBytes(char *buffer, size_t size);

  /*!
  \fn int readBytes(uint8_t *buffer, size_t size)
  \brief Read bytes from a stream into a buffer. Terminates if the determined length has been read, or it times out.
  \param [out] *buffer pointer to readed bytes buffer.
  \param [in] size maximum number of bytes to be read.
  \return the number of bytes placed in the buffer. A 0 means no valid data was found.
  */
  int readBytes(uint8_t *buffer, size_t size);

  /*!
  \fn void setTimeout(uint32_t timeout)
  \brief Sets the maximum milliseconds to wait for stream data.
  \param[in] timeout milliseconds for timeout.
  \return void.
  */
  void setTimeout(uint32_t timeout);

  /*!
  \fn size_t write(uint8_t buffer)
  \brief Write bytes from a buffer into a stream.
  \param [in] *buffer pointer to write bytes buffer.
  \return the number of bytes written in the buffer.
  */
  size_t write(uint8_t buffer);

  /*!
  \fn size_t write(const uint8_t *buffer, size_t size)
  \brief Write bytes from a buffer into a stream. Terminates if the determined length has been written.
  \param [in] *buffer pointer to write bytes buffer.
  \param [in] size maximum number of bytes to write into stream.
  \return the number of bytes written in the buffer.
  */
  size_t write(const uint8_t *buffer, size_t size);

  /*!
  \fn void flush()
  \brief Clears the buffer once all outgoing characters have been sent.
  \return void.
  */
  void flush();

  /*!
  \fn void stop()
  \brief Stop.
  \return void.
  */
  void stop();
};

#endif

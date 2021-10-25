/*
 UIPClient.cpp - Arduino implementation of a uIP wrapper class.
 Copyright (c) 2013 Norbert Truchsess <norbert.truchsess@t-online.de>
 All rights reserved.

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

extern "C"
{
#include "utility/uip-conf.h"
#include "utility/uip.h"
#include "utility/uip_arp.h"
}
#include "Ethernet.h"
#include "EthernetClient.h"
#include "Dns.h"

#define UIP_TCP_PHYH_LEN UIP_LLH_LEN+UIP_IPTCPH_LEN

#define UIPClient EthernetClient // to not pollute source code history with the rename

uip_userdata_t UIPClient::all_data[UIP_CONNS];

UIPClient::UIPClient() :
    data(NULL)
{
}

UIPClient::UIPClient(uip_userdata_t* conn_data) :
    data(conn_data)
{
}

int
UIPClient::connect(IPAddress ip, uint16_t port)
{
  stop();
  uip_ipaddr_t ipaddr;
  uip_ip_addr(ipaddr, ip);
  struct uip_conn* conn = uip_connect(&ipaddr, htons(port));
  if (conn)
    {
#if UIP_CONNECT_TIMEOUT > 0
      int32_t timeout = millis() + 1000 * UIP_CONNECT_TIMEOUT;
#endif
      while((conn->tcpstateflags & UIP_TS_MASK) != UIP_CLOSED)
        {
          UIPEthernetClass::tick();
          if ((conn->tcpstateflags & UIP_TS_MASK) == UIP_ESTABLISHED)
            {
              data = (uip_userdata_t*) conn->appstate;
#ifdef UIPETHERNET_DEBUG_CLIENT
              Serial.print(F("connected, state: "));
              Serial.print(data->state);
              Serial.print(F(", first packet in: "));
              Serial.println(data->packets_in[0]);
#endif
              return 1;
            }
#if UIP_CONNECT_TIMEOUT > 0
          if (((int32_t)(millis() - timeout)) > 0)
            {
              conn->tcpstateflags = UIP_CLOSED;
              break;
            }
#endif
        }
    }
  return 0;
}

int
UIPClient::connect(const char *host, uint16_t port)
{
  // Look up the host first
  int ret = 0;
#if UIP_UDP
  DNSClient dns;
  IPAddress remote_addr;

  dns.begin(UIPEthernetClass::_dnsServerAddress);
  ret = dns.getHostByName(host, remote_addr);
  if (ret == 1) {
    return connect(remote_addr, port);
  }
#endif
  return ret;
}

void
UIPClient::stop()
{
  if (data && data->state)
    {
#ifdef UIPETHERNET_DEBUG_CLIENT
      Serial.println(F("before stop(), with data"));
      _dumpAllData();
#endif
      _flushBlocks(data->packets_in);
      if (data->state & UIP_CLIENT_REMOTECLOSED)
        {
          data->state = 0;
        }
      else
        {
          flush();
          data->state |= UIP_CLIENT_CLOSE;
        }
#ifdef UIPETHERNET_DEBUG_CLIENT
      Serial.println(F("after stop()"));
      _dumpAllData();
#endif
    }
#ifdef UIPETHERNET_DEBUG_CLIENT
  else
    {
      Serial.println(F("stop(), data: NULL"));
    }
#endif
  data = NULL;
  UIPEthernetClass::tick();
}

uint8_t
UIPClient::connected()
{
  return (data && (data->packets_in[0] != NOBLOCK || (data->state & UIP_CLIENT_CONNECTED))) ? 1 : 0;
}

bool
UIPClient::operator==(const UIPClient& rhs)
{
  return data && rhs.data && (data == rhs.data);
}

UIPClient::operator bool()
{
  UIPEthernetClass::tick();
  return data && (!(data->state & UIP_CLIENT_REMOTECLOSED) || data->packets_in[0] != NOBLOCK);
}

size_t
UIPClient::write(uint8_t c)
{
  return write(&c, 1);
}

size_t
UIPClient::write(const uint8_t *buf, size_t size)
{
  uip_userdata_t* u = data;
  int remain = size;
  uint16_t written;
#if UIP_WRITE_TIMEOUT > 0
  uint32_t timeout_start = millis();
#endif
  repeat:
  UIPEthernetClass::tick();
  if (u && u->state && !(u->state & (UIP_CLIENT_CLOSE | UIP_CLIENT_REMOTECLOSED)))
    {
      uint8_t p = _currentBlock(u->packets_out);
      if (u->packets_out[p] == NOBLOCK)
        {
newpacket:
          u->packets_out[p] = Enc28J60Network::allocBlock(UIP_SOCKET_DATALEN);
          if (u->packets_out[p] == NOBLOCK)
            {
#if UIP_WRITE_TIMEOUT > 0
              if (millis() - timeout_start > UIP_WRITE_TIMEOUT)
                {
                  setWriteError();
                  goto ready;
                }
#endif
              Ethernet.call_yield();
              goto repeat;
            }
          u->out_pos = 0;
        }
#ifdef UIPETHERNET_DEBUG_CLIENT
      Serial.print(F("UIPClient.write: writePacket("));
      Serial.print(u->packets_out[p]);
      Serial.print(F(") pos: "));
      Serial.print(u->out_pos);
      Serial.print(F(", buf["));
      Serial.print(size-remain);
      Serial.print(F("-"));
      Serial.print(remain);
      Serial.print(F("]: '"));
      Serial.write((uint8_t*)buf+size-remain,remain);
      Serial.println(F("'"));
#endif
      written = Enc28J60Network::writePacket(u->packets_out[p],u->out_pos,(uint8_t*)buf+size-remain,remain);
      remain -= written;
      u->out_pos+=written;
      if (remain > 0)
        {
          if (p == 0) // block 0 just filled, start sending immediately
            flush();
          if (p == UIP_SOCKET_NUMPACKETS-1)
            {
#if UIP_WRITE_TIMEOUT > 0
              if (millis() - timeout_start > UIP_WRITE_TIMEOUT)
                {
                  setWriteError();
                  goto ready;
                }
#endif
              Ethernet.call_yield();
              goto repeat;
            }
          p++;
          goto newpacket;
        }
ready:
      return size-remain;
    }
  return 0;
}

int
UIPClient::availableForWrite()
{
  const int MAX_AVAILABLE = UIP_SOCKET_DATALEN * UIP_SOCKET_NUMPACKETS;
  UIPEthernetClass::tick();
  if (data->packets_out[0] == NOBLOCK)
    return MAX_AVAILABLE;
  uint8_t p = _currentBlock(data->packets_out);
  int used = UIP_SOCKET_DATALEN * p + data->out_pos;
  return MAX_AVAILABLE - used;
}

void
UIPClient::flush()
{
  UIPEthernetClass::tick();

  if (data && data->packets_out[0] != NOBLOCK)
    {
      struct uip_conn* conn = &uip_conns[data->conn_index];
      if (!uip_outstanding(conn))
        {
          uip_poll_conn(conn);
          if (uip_len > 0)
            {
              uip_arp_out();
              UIPEthernetClass::network_send();
            }
        }
    }
}

int
UIPClient::available()
{
  if (!(*this))
    return 0;

  int len = 0;
  for (uint8_t i = 0; i < UIP_SOCKET_NUMPACKETS; i++)
    {
      len += Enc28J60Network::blockSize(data->packets_in[i]);
    }

  // if sketch checks for incoming data and there are unsent data, flush the transmit buffer
  if (!len)
    flush();
  return len;
}

int
UIPClient::read(uint8_t *buf, size_t size)
{
  if (*this)
    {
      uint16_t remain = size;
      if (data->packets_in[0] == NOBLOCK)
        {
        flush();  // if sketch checks for incoming data and there are unsent data, flush the transmit buffer
        return 0;
        }
      uint16_t read;
      do
        {
          read = Enc28J60Network::readPacket(data->packets_in[0],0,buf+size-remain,remain);
          if (read == Enc28J60Network::blockSize(data->packets_in[0]))
            {
              remain -= read;
              _eatBlock(data->packets_in);
              if (uip_stopped(&uip_conns[data->conn_index]) && !(data->state & (UIP_CLIENT_CLOSE | UIP_CLIENT_REMOTECLOSED)))
                data->state |= UIP_CLIENT_RESTART;
              if (data->packets_in[0] == NOBLOCK)
                {
                  if (data->state & UIP_CLIENT_REMOTECLOSED)
                    {
                      data->state = 0;
                      data = NULL;
                    }
                  return size-remain;
                }
            }
          else
            {
              Enc28J60Network::resizeBlock(data->packets_in[0],read);
              break;
            }
        }
      while(remain > 0);
      return size;
    }
  return -1;
}

int
UIPClient::read()
{
  uint8_t c;
  if (read(&c,1) <= 0)
    return -1;
  return c;
}

int
UIPClient::peek()
{
  if (*this)
    {
      if (data->packets_in[0] != NOBLOCK)
        {
          uint8_t c;
          Enc28J60Network::readPacket(data->packets_in[0],0,&c,1);
          return c;
        }
    }
  return -1;
}

void
UIPClient::discardReceived()
{
  if (*this)
    {
      _flushBlocks(data->packets_in);
    }
}

IPAddress
UIPClient::remoteIP(void)
{
  return data ? ip_addr_uip(uip_conns[data->conn_index].ripaddr) : IPAddress();
}

uint16_t
UIPClient::remotePort(void)
{
  return data ? ntohs(uip_conns[data->conn_index].rport) : 0;
}

void
uipclient_appcall(void)
{
  uint16_t send_len = 0;
  uip_userdata_t *u = (uip_userdata_t*)uip_conn->appstate;
  if (!u && uip_connected())
    {
#ifdef UIPETHERNET_DEBUG_CLIENT
      Serial.println(F("UIPClient uip_connected"));
      UIPClient::_dumpAllData();
#endif
      u = (uip_userdata_t*) UIPClient::_allocateData();
      if (u)
        {
          uip_conn->appstate = u;
#ifdef UIPETHERNET_DEBUG_CLIENT
          Serial.print(F("UIPClient allocated state: "));
          Serial.println(u->state,BIN);
#endif
        }
#ifdef UIPETHERNET_DEBUG_CLIENT
      else
        Serial.println(F("UIPClient allocation failed"));
#endif
    }
  if (u)
    {
      if (uip_newdata())
        {
#ifdef UIPETHERNET_DEBUG_CLIENT
          Serial.print(F("UIPClient uip_newdata, uip_len:"));
          Serial.println(uip_len);
#endif
          if (uip_len && !(u->state & (UIP_CLIENT_CLOSE | UIP_CLIENT_REMOTECLOSED)))
            {
              for (uint8_t i=0; i < UIP_SOCKET_NUMPACKETS; i++)
                {
                  if (u->packets_in[i] == NOBLOCK)
                    {
                      u->packets_in[i] = Enc28J60Network::allocBlock(uip_len);
                      if (u->packets_in[i] != NOBLOCK)
                        {
                          Enc28J60Network::copyPacket(u->packets_in[i],0,UIPEthernetClass::in_packet,((uint8_t*)uip_appdata)-uip_buf,uip_len);
                          if (i == UIP_SOCKET_NUMPACKETS-1)
                            uip_stop();
                          goto finish_newdata;
                        }
                    }
                }
              UIPEthernetClass::packetstate &= ~UIPETHERNET_FREEPACKET;
              uip_stop();
            }
        }
finish_newdata:
      if (u->state & UIP_CLIENT_RESTART)
        {
          u->state &= ~UIP_CLIENT_RESTART;
          uip_restart();
        }
      // If the connection has been closed, save received but unread data.
      if (uip_closed() || uip_timedout() || uip_aborted())
        {
#ifdef UIPETHERNET_DEBUG_CLIENT
          Serial.println(F("UIPClient uip_closed"));
          UIPClient::_dumpAllData();
#endif
          // drop outgoing packets not sent yet:
          UIPClient::_flushBlocks(u->packets_out);
          if (u->packets_in[0] != NOBLOCK)
            {
              ((uip_userdata_closed_t *)u)->lport = uip_conn->lport;
              u->state |= UIP_CLIENT_REMOTECLOSED;
            }
          else
            u->state = 0;
          // disassociate appdata.
#ifdef UIPETHERNET_DEBUG_CLIENT
          Serial.println(F("after UIPClient uip_closed"));
          UIPClient::_dumpAllData();
#endif
          uip_conn->appstate = NULL;
          goto finish;
        }
      if (uip_acked())
        {
#ifdef UIPETHERNET_DEBUG_CLIENT
          Serial.println(F("UIPClient uip_acked"));
#endif
          UIPClient::_eatBlock(u->packets_out);
          goto send;
        }
      if (uip_poll() || uip_rexmit())
        {
#ifdef UIPETHERNET_DEBUG_CLIENT
          //Serial.println(F("UIPClient uip_poll"));
#endif
send:
          if (u->packets_out[0] != NOBLOCK)
            {
              if (u->packets_out[1] == NOBLOCK)
                {
                  send_len = u->out_pos;
                  if (send_len > 0)
                    {
                      Enc28J60Network::resizeBlock(u->packets_out[0],0,send_len);
                    }
                 }
              else
                send_len = Enc28J60Network::blockSize(u->packets_out[0]);
              if (send_len > 0)
                {
                  UIPEthernetClass::uip_hdrlen = ((uint8_t*)uip_appdata)-uip_buf;
                  UIPEthernetClass::uip_packet = Enc28J60Network::allocBlock(UIPEthernetClass::uip_hdrlen+send_len + UIP_SENDBUFFER_OFFSET + UIP_SENDBUFFER_PADDING);
                  if (UIPEthernetClass::uip_packet != NOBLOCK)
                    {
                      Enc28J60Network::copyPacket(UIPEthernetClass::uip_packet,UIPEthernetClass::uip_hdrlen + UIP_SENDBUFFER_OFFSET,u->packets_out[0],0,send_len);
                      UIPEthernetClass::packetstate |= UIPETHERNET_SENDPACKET;
                    }
                }
              goto finish;
            }
        }
      // don't close connection unless all outgoing packets are sent
      if (u->state & UIP_CLIENT_CLOSE)
        {
#ifdef UIPETHERNET_DEBUG_CLIENT
          Serial.println(F("UIPClient state UIP_CLIENT_CLOSE"));
          UIPClient::_dumpAllData();
#endif
          if (u->packets_out[0] == NOBLOCK)
            {
              u->state = 0;
              uip_conn->appstate = NULL;
              uip_close();
#ifdef UIPETHERNET_DEBUG_CLIENT
              Serial.println(F("no blocks out -> free userdata"));
              UIPClient::_dumpAllData();
#endif
            }
          else
            {
              uip_stop();
#ifdef UIPETHERNET_DEBUG_CLIENT
              Serial.println(F("blocks outstanding transfer -> uip_stop()"));
#endif
            }
        }
    }
  finish:
  uip_send(uip_appdata,send_len);
  uip_len = send_len;
}

uip_userdata_t *
UIPClient::_allocateData()
{
  for ( uint8_t sock = 0; sock < UIP_CONNS; sock++ )
    {
      uip_userdata_t* data = &UIPClient::all_data[sock];
      if (!data->state)
        {
          data->conn_index = uip_conn - uip_conns; // pointer arithmetics
          data->state = UIP_CLIENT_CONNECTED;
          memset(&data->packets_in[0],0,sizeof(uip_userdata_t)-sizeof(data->state));
          return data;
        }
    }
  return NULL;
}

uint8_t
UIPClient::_currentBlock(memhandle* block)
{
  for (uint8_t i = 1; i < UIP_SOCKET_NUMPACKETS; i++)
    {
      if (block[i] == NOBLOCK)
        return i-1;
    }
  return UIP_SOCKET_NUMPACKETS-1;
}

void
UIPClient::_eatBlock(memhandle* block)
{
#ifdef UIPETHERNET_DEBUG_CLIENT
  memhandle* start = block;
  Serial.print(F("eatblock("));
  Serial.print(*block);
  Serial.print(F("): "));
  for (uint8_t i = 0; i < UIP_SOCKET_NUMPACKETS; i++)
    {
      Serial.print(start[i]);
      Serial.print(F(" "));
    }
  Serial.print(F("-> "));
#endif
  Enc28J60Network::freeBlock(block[0]);
  for (uint8_t i = 0; i < UIP_SOCKET_NUMPACKETS-1; i++)
    {
      block[i] = block[i+1];
    }
  block[UIP_SOCKET_NUMPACKETS-1] = NOBLOCK;
#ifdef UIPETHERNET_DEBUG_CLIENT
  for (uint8_t i = 0; i < UIP_SOCKET_NUMPACKETS; i++)
    {
      Serial.print(start[i]);
      Serial.print(F(" "));
    }
  Serial.println();
#endif
}

void
UIPClient::_flushBlocks(memhandle* block)
{
  for (uint8_t i = 0; i < UIP_SOCKET_NUMPACKETS; i++)
    {
      Enc28J60Network::freeBlock(block[i]);
      block[i] = NOBLOCK;
    }
}

#ifdef UIPETHERNET_DEBUG_CLIENT
void
UIPClient::_dumpAllData() {
  for (uint8_t i=0; i < UIP_CONNS; i++)
    {
      Serial.print(F("UIPClient::all_data["));
      Serial.print(i);
      Serial.print(F("], state:"));
      Serial.println(all_data[i].state, BIN);
      Serial.print(F("packets_in: "));
      for (uint8_t j=0; j < UIP_SOCKET_NUMPACKETS; j++)
        {
          Serial.print(all_data[i].packets_in[j]);
          Serial.print(F(" "));
        }
      Serial.println();
      if (all_data[i].state & UIP_CLIENT_REMOTECLOSED)
        {
          Serial.print(F("state remote closed, local port: "));
          Serial.println(htons(((uip_userdata_closed_t *)(&all_data[i]))->lport));
        }
      else
        {
          Serial.print(F("packets_out: "));
          for (uint8_t j=0; j < UIP_SOCKET_NUMPACKETS; j++)
            {
              Serial.print(all_data[i].packets_out[j]);
              Serial.print(F(" "));
            }
          Serial.println();
          Serial.print(F("out_pos: "));
          Serial.println(all_data[i].out_pos);
        }
    }
}
#endif

#include "inetGSM.h"
#define _GSM_CONNECTION_TOUT_ 5
#define _TCP_CONNECTION_TOUT_ 20
#define _GSM_DATA_TOUT_ 10


// You must free the result if result is non-NULL.
char *str_replace( const char *orig, const char *rep, const char *with) {
    char *result; // the return string
    const char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep
    int len_with; // length of with
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    if (!orig)
        return NULL;
    if (!rep)
        rep = "";
    len_rep = strlen(rep);
    if (!with)
        with = "";
    len_with = strlen(with);

    ins = orig;
    for (count = 0; (tmp = strstr(ins, rep)); ++count) {
        ins = tmp + len_rep;
    }

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    tmp = result = (char*)malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}



int InetGSM::httpGET(const char* server, int port, const char* path, char* result, int resultlength)
{

  char Buffer[200];

  char* newpath;

  newpath = str_replace(path,"\"", "%22");

  char end_c[2];
  end_c[0]=0x1a;
  end_c[1]='\0';

  gsm.RxInit(5000,1000);
  gsm.Echo(0);
  
  if (AT_RESP_OK == gsm.SendATCmdWaitResp("AT+SAPBR=3,1,\"Contype\",\"GPRS\"",    5000, 1000, str_ok, 1)) Serial.println(str_ok);
  gsm.RxInit(5000,1000);
  if (AT_RESP_OK == gsm.SendATCmdWaitResp("AT+SAPBR=3,1,\"APN\",\"ibox.tim.it\"", 5000, 1000, str_ok, 1)) Serial.println(str_ok);
  gsm.RxInit(5000,1000);
  if (AT_RESP_OK == gsm.SendATCmdWaitResp("AT+SAPBR=1,1",                         5000, 1000, str_ok, 1)) Serial.println(str_ok);
  gsm.RxInit(5000,1000);
  if (AT_RESP_OK == gsm.SendATCmdWaitResp("AT+SAPBR=2,1",                         5000, 1000, str_ok, 1)) Serial.println(str_ok);
  gsm.RxInit(5000,1000);
  if (AT_RESP_OK == gsm.SendATCmdWaitResp("AT+HTTPINIT",                          5000, 1000, str_ok, 1)) Serial.println(str_ok);
  gsm.RxInit(5000,1000);
  if (AT_RESP_OK == gsm.SendATCmdWaitResp("AT+HTTPPARA=\"CID\",1",                5000, 1000, str_ok, 1)) Serial.println(str_ok);
  gsm.RxInit(5000,1000);
  sprintf(Buffer, "AT+HTTPPARA=\"URL\",\"%s%s\"",server,newpath );
  free(newpath);
  if (AT_RESP_OK == gsm.SendATCmdWaitResp(Buffer,                                 5000, 1000, str_ok, 1)) Serial.println(str_ok);
  //if (AT_RESP_OK == gsm.SendATCmdWaitResp("AT+HTTPPARA=\"URL\",\"rmap.cc\"",      5000, 1000, str_ok, 1)) Serial.println(str_ok);
  gsm.RxInit(5000,1000);

  if (AT_RESP_OK == gsm.SendATCmdWaitResp("AT+HTTPACTION=0",                      5000, 1000, str_ok, 1)) Serial.println(str_ok);
  gsm.WaitResp(10000, 50, "+HTTPACTION:");
  gsm.RxInit(5000,1000);
  //if (AT_RESP_OK == gsm.SendATCmdWaitResp("AT+HTTPREAD",                    5000, 1000, str_ok, 1)) Serial.println(str_ok);
  gsm.SimpleWrite("AT+HTTPREAD\r\n");
  gsm.SimpleWrite(end_c);
  //gsm.WaitResp(5000, 50, "+HTTPREAD:");
  int res = gsm.read(result, resultlength);
  gsm.RxInit(5000,1000);
  if (AT_RESP_OK == gsm.SendATCmdWaitResp("AT+HTTPTERM",                          5000, 1000, str_ok, 1)) Serial.println(str_ok);
  if (AT_RESP_OK == gsm.SendATCmdWaitResp("AT+SAPBR=0,1",                         5000, 1000, str_ok, 1)) Serial.println(str_ok);

  return res;
}


int InetGSM::httpGETold(const char* server, int port, const char* path, char* result, int resultlength)
{
     boolean connected=false;
     int n_of_at=0;
     //int length_write;
     char end_c[2];
     end_c[0]=0x1a;
     end_c[1]='\0';

     /*
     Status = ATTACHED.
     if(gsm.getStatus()!=GSM::ATTACHED)
       return 0;
     */

     //disconnectTCP();

     while(n_of_at<3) {
          if(!connectTCP(server, port)) {
#ifdef DEBUG_ON
               Serial.println("DB:NOT CONN");
#endif
               n_of_at++;
          } else {
               connected=true;
               n_of_at=3;
          }
     }

     if(!connected) return 0;

     gsm.SimpleWrite("GET ");
     gsm.SimpleWrite(path);
     gsm.SimpleWrite(" HTTP/1.0\r\nHost: ");
     gsm.SimpleWrite(server);
     gsm.SimpleWrite("\r\n");
     gsm.SimpleWrite("User-Agent: Arduino");
     gsm.SimpleWrite("\r\n\r\n");
     gsm.SimpleWrite(end_c);

     switch(gsm.WaitResp(5000, 10, "SEND OK")) {
     case RX_TMOUT_ERR:
          return 0;
          break;
     case RX_FINISHED_STR_NOT_RECV:
          return 0;
          break;
     }


     delay(50);
#ifdef DEBUG_ON
     Serial.println("DB:SENT");
#endif
     int res = gsm.read(result, resultlength);

     disconnectTCP();

     //int res=1;
     return res;
}

int InetGSM::httpPOST(const char* server, int port, const char* path, const char* parameters, char* result, int resultlength)
{
     boolean connected=false;
     int n_of_at=0;
     char itoaBuffer[8];
     //int num_char;
     char end_c[2];
     end_c[0]=0x1a;
     end_c[1]='\0';

     disconnectTCP();

     while(n_of_at<3) {
          if(!connectTCP(server, port)) {
#ifdef DEBUG_ON
               Serial.println("DB:NOT CONN");
#endif
               n_of_at++;
          } else {
               connected=true;
               n_of_at=3;
          }
     }

     if(!connected) return 0;

     gsm.SimpleWrite("POST ");
     gsm.SimpleWrite(path);
     gsm.SimpleWrite(" HTTP/1.1\r\nHost: ");
     gsm.SimpleWrite(server);
     gsm.SimpleWrite("\r\n");
     gsm.SimpleWrite("User-Agent: Arduino\r\n");
     gsm.SimpleWrite("Content-Type: application/x-www-form-urlencoded\r\n");
     gsm.SimpleWrite("Content-Length: ");
     itoa(strlen(parameters),itoaBuffer,10);
     gsm.SimpleWrite(itoaBuffer);
     gsm.SimpleWrite("\r\n\r\n");
     gsm.SimpleWrite(parameters);
     gsm.SimpleWrite("\r\n\r\n");
     gsm.SimpleWrite(end_c);

     switch(gsm.WaitResp(10000, 10, "SEND OK")) {
     case RX_TMOUT_ERR:
          return 0;
          break;
     case RX_FINISHED_STR_NOT_RECV:
          return 0;
          break;
     }

     delay(50);
#ifdef DEBUG_ON
     Serial.println("DB:SENT");
#endif

     int res= gsm.read(result, resultlength);
     disconnectTCP();
     return res;
}

int InetGSM::openmail(char* server, char* loginbase64, char* passbase64, char* from, char* to, char* subj)
{
     boolean connected=false;
     int n_of_at=0;
     char end_c[2];
     end_c[0]=0x1a;
     end_c[1]='\0';

     while(n_of_at<3) {
          if(!connectTCP(server, 25)) {
#ifdef DEBUG_ON
               Serial.println("DB:NOT CONN");
#endif
               n_of_at++;
          } else {
               connected=true;
               n_of_at=3;
          }
     }

     if(!connected) return 0;

     delay(100);
     gsm.SimpleWrite("HELO ");
     gsm.SimpleWrite(server);
     gsm.SimpleWrite("\n");
     gsm.SimpleWrite(end_c);
     gsm.WaitResp(5000, 100, "OK");
     if(!gsm.IsStringReceived("SEND OK"))
          return 0;
     delay(500);
     gsm.WaitResp(5000, 100);

     delay(100);
     gsm.SimpleWriteln("AT+CIPSEND");
     switch(gsm.WaitResp(5000, 200, ">")) {
     case RX_TMOUT_ERR:
          return 0;
          break;
     case RX_FINISHED_STR_NOT_RECV:
          return 0;
          break;
     }
     gsm.SimpleWrite("AUTH LOGIN\n");
     gsm.SimpleWrite(end_c);
     gsm.WaitResp(5000, 100, "OK");
     if(!gsm.IsStringReceived("OK"))
          return 0;
     delay(500);
     gsm.WaitResp(5000, 100);

     delay(100);
     gsm.SimpleWriteln("AT+CIPSEND");
     switch(gsm.WaitResp(5000, 200, ">")) {
     case RX_TMOUT_ERR:
          return 0;
          break;
     case RX_FINISHED_STR_NOT_RECV:
          return 0;
          break;
     }
     gsm.SimpleWrite(loginbase64);
     gsm.SimpleWrite("\n");
     gsm.SimpleWrite(end_c);
     gsm.WaitResp(5000, 100, "OK");
     if(!gsm.IsStringReceived("OK"))
          return 0;
     delay(500);
     gsm.WaitResp(5000, 100);

     delay(100);
     gsm.SimpleWriteln("AT+CIPSEND");
     switch(gsm.WaitResp(5000, 200, ">")) {
     case RX_TMOUT_ERR:
          return 0;
          break;
     case RX_FINISHED_STR_NOT_RECV:
          return 0;
          break;
     }
     gsm.SimpleWrite(passbase64);
     gsm.SimpleWrite("\n");
     gsm.SimpleWrite(end_c);
     gsm.WaitResp(5000, 100, "OK");
     if(!gsm.IsStringReceived("OK"))
          return 0;
     delay(500);
     gsm.WaitResp(5000, 100);


     delay(100);
     gsm.SimpleWriteln("AT+CIPSEND");
     switch(gsm.WaitResp(5000, 200, ">")) {
     case RX_TMOUT_ERR:
          return 0;
          break;
     case RX_FINISHED_STR_NOT_RECV:
          return 0;
          break;
     }
     gsm.SimpleWrite("MAIL From: <");
     gsm.SimpleWrite(from);
     gsm.SimpleWrite(">\n");
     gsm.SimpleWrite(end_c);
     gsm.WaitResp(5000, 100, "OK");
     if(!gsm.IsStringReceived("OK"))
          return 0;
     delay(500);
     gsm.WaitResp(5000, 100, "");

     delay(100);
     gsm.SimpleWriteln("AT+CIPSEND");
     switch(gsm.WaitResp(5000, 200, ">")) {
     case RX_TMOUT_ERR:
          return 0;
          break;
     case RX_FINISHED_STR_NOT_RECV:
          return 0;
          break;
     }
     gsm.SimpleWrite("RCPT TO: <");
     gsm.SimpleWrite(to);
     gsm.SimpleWrite(">\n");
     gsm.SimpleWrite(end_c);
     gsm.WaitResp(5000, 100, "OK");
     if(!gsm.IsStringReceived("OK"))
          return 0;
     delay(500);
     gsm.WaitResp(5000, 100, "");

     delay(100);
     gsm.SimpleWriteln("AT+CIPSEND");
     switch(gsm.WaitResp(5000, 200, ">")) {
     case RX_TMOUT_ERR:
          return 0;
          break;
     case RX_FINISHED_STR_NOT_RECV:
          return 0;
          break;
     }
     gsm.SimpleWrite("Data\n");
     gsm.SimpleWrite(end_c);
     gsm.WaitResp(5000, 100, "OK");
     if(!gsm.IsStringReceived("OK"))
          return 0;
     delay(500);
     gsm.WaitResp(5000, 100, "");

     delay(100);
     gsm.SimpleWriteln("AT+CIPSEND");
     switch(gsm.WaitResp(5000, 200, ">")) {
     case RX_TMOUT_ERR:
          return 0;
          break;
     case RX_FINISHED_STR_NOT_RECV:
          return 0;
          break;
     }
     gsm.SimpleWrite("Subject: ");
     gsm.SimpleWrite(subj);
     gsm.SimpleWrite("\n\n");

     return 1;
}
int InetGSM::closemail()
{
     char end_c[2];
     end_c[0]=0x1a;
     end_c[1]='\0';

     gsm.SimpleWrite("\n.\n");
     gsm.SimpleWrite(end_c);
     disconnectTCP();
     return 1;
}


int InetGSM::attachGPRS(char* domain, char* dom1, char* dom2)
{
     //int i=0;
     //delay(5000);

     //gsm._tf.setTimeout(_GSM_DATA_TOUT_);	//Timeout for expecting modem responses.
     gsm.WaitResp(50, 50);
     gsm.SimpleWriteln("AT+CIFSR");
     if(gsm.WaitResp(5000, 50, "ERROR")!=RX_FINISHED_STR_RECV) {
#ifdef DEBUG_ON
          Serial.println("DB:ALREADY HAVE AN IP");
#endif
          gsm.SimpleWriteln("AT+CIPCLOSE");
          gsm.WaitResp(5000, 50, "ERROR");
          delay(2000);
          gsm.SimpleWriteln("AT+CIPSERVER=0");
          gsm.WaitResp(5000, 50, "ERROR");
          return 1;
     } else {

#ifdef DEBUG_ON
          Serial.println("DB:STARTING NEW CONNECTION");
#endif

          gsm.SimpleWriteln("AT+CIPSHUT");

          switch(gsm.WaitResp(500, 50, "SHUT OK")) {

          case RX_TMOUT_ERR:
               return 0;
               break;
          case RX_FINISHED_STR_NOT_RECV:
               return 0;
               break;
          }
#ifdef DEBUG_ON
          Serial.println("DB:SHUTTED OK");
#endif
          delay(1000);

          gsm.SimpleWrite("AT+CSTT=\"");
          gsm.SimpleWrite(domain);
          gsm.SimpleWrite("\",\"");
          gsm.SimpleWrite(dom1);
          gsm.SimpleWrite("\",\"");
          gsm.SimpleWrite(dom2);
          gsm.SimpleWrite("\"\r");


          switch(gsm.WaitResp(500, 50, "OK")) {

          case RX_TMOUT_ERR:
               return 0;
               break;
          case RX_FINISHED_STR_NOT_RECV:
               return 0;
               break;
          }
#ifdef DEBUG_ON
          Serial.println("DB:APN OK");
#endif
          delay(5000);

          gsm.SimpleWriteln("AT+CIICR");

          switch(gsm.WaitResp(10000, 50, "OK")) {
          case RX_TMOUT_ERR:
               return 0;
               break;
          case RX_FINISHED_STR_NOT_RECV:
               return 0;
               break;
          }
#ifdef DEBUG_ON
          Serial.println("DB:CONNECTION OK");
#endif

          delay(1000);


          gsm.SimpleWriteln("AT+CIFSR");
          if(gsm.WaitResp(5000, 50, "ERROR")!=RX_FINISHED_STR_RECV) {
#ifdef DEBUG_ON
               Serial.println("DB:ASSIGNED AN IP");
#endif
               gsm.setStatus(gsm.ATTACHED);
               return 1;
          }
#ifdef DEBUG_ON
          Serial.println("DB:NO IP AFTER CONNECTION");
#endif
          return 0;
     }
}

int InetGSM::dettachGPRS()
{
     if (gsm.getStatus()==gsm.IDLE) return 0;

     //gsm._tf.setTimeout(_GSM_CONNECTION_TOUT_);

     //_cell.flush();

     //GPRS dettachment.
     gsm.SimpleWriteln("AT+CGATT=0");
     if(gsm.WaitResp(5000, 50, "OK")!=RX_FINISHED_STR_NOT_RECV) {
          gsm.setStatus(gsm.ERROR);
          return 0;
     }
     delay(500);

     // Commented in initial trial code!!
     //Stop IP stack.
     //_cell << "AT+WIPCFG=0" <<  _DEC(cr) << endl;
     //	if(!gsm._tf.find("OK")) return 0;
     //Close GPRS bearer.
     //_cell << "AT+WIPBR=0,6" <<  _DEC(cr) << endl;

     gsm.setStatus(gsm.READY);
     return 1;
}

int InetGSM::connectTCP(const char* server, int port)
{
     //gsm._tf.setTimeout(_TCP_CONNECTION_TOUT_);

     //Status = ATTACHED.
     //if (getStatus()!=ATTACHED)
     //return 0;

     //_cell.flush();

     //Visit the remote TCP server.
     gsm.SimpleWrite("AT+CIPSTART=\"TCP\",\"");
     gsm.SimpleWrite(server);
     gsm.SimpleWrite("\",");
     gsm.SimpleWriteln(port);

     switch(gsm.WaitResp(1000, 200, "OK")) {
     case RX_TMOUT_ERR:
          return 0;
          break;
     case RX_FINISHED_STR_NOT_RECV:
          return 0;
          break;
     }
#ifdef DEBUG_ON
     Serial.println("DB:RECVD CMD");
#endif
     if (!gsm.IsStringReceived("CONNECT OK")) {
          switch(gsm.WaitResp(15000, 200, "OK")) {
          case RX_TMOUT_ERR:
               return 0;
               break;
          case RX_FINISHED_STR_NOT_RECV:
               return 0;
               break;
          }
     }

#ifdef DEBUG_ON
     Serial.println("DB:OK TCP");
#endif

     delay(3000);
     gsm.SimpleWriteln("AT+CIPSEND");
     switch(gsm.WaitResp(5000, 200, ">")) {
     case RX_TMOUT_ERR:
          return 0;
          break;
     case RX_FINISHED_STR_NOT_RECV:
          return 0;
          break;
     }

#ifdef DEBUG_ON
     Serial.println("DB:>");
#endif
     delay(4000);
     return 1;
}

int InetGSM::disconnectTCP()
{
     //Status = TCPCONNECTEDCLIENT or TCPCONNECTEDSERVER.
     /*
     if ((getStatus()!=TCPCONNECTEDCLIENT)&&(getStatus()!=TCPCONNECTEDSERVER))
        return 0;
     */
     //gsm._tf.setTimeout(_GSM_CONNECTION_TOUT_);


     //_cell.flush();

     //Switch to AT mode.
     //_cell << "+++" << endl;

     //delay(200);

     //Close TCP client and deact.
     gsm.SimpleWriteln("AT+CIPCLOSE");

     //If remote server close connection AT+QICLOSE generate ERROR
     /*if(gsm._tf.find("OK"))
     {
       if(getStatus()==TCPCONNECTEDCLIENT)
         gsm.setStatus(ATTACHED);
       else
         gsm.setStatus(TCPSERVERWAIT);
       return 1;
     }
     gsm.setStatus(ERROR);

     return 0;    */
     if(gsm.getStatus()==gsm.TCPCONNECTEDCLIENT)
          gsm.setStatus(gsm.ATTACHED);
     else
          gsm.setStatus(gsm.TCPSERVERWAIT);
     return 1;
}

int InetGSM::connectTCPServer(int port)
{
     /*
       if (getStatus()!=ATTACHED)
          return 0;
     */
     //gsm._tf.setTimeout(_GSM_CONNECTION_TOUT_);

     //_cell.flush();

     // Set port

     gsm.SimpleWrite("AT+CIPSERVER=1,");
     gsm.SimpleWriteln(port);
     /*
       switch(gsm.WaitResp(5000, 50, "OK")){
     	case RX_TMOUT_ERR:
     		return 0;
     	break;
     	case RX_FINISHED_STR_NOT_RECV:
     		return 0;
     	break;
       }

       switch(gsm.WaitResp(5000, 50, "SERVER")){ //Try SERVER OK
     	case RX_TMOUT_ERR:
     		return 0;
     	break;
     	case RX_FINISHED_STR_NOT_RECV:
     		return 0;
     	break;
       }
     */
     //delay(200);

     return 1;

}

boolean InetGSM::connectedClient()
{
     /*
     if (getStatus()!=TCPSERVERWAIT)
        return 0;
     */

     gsm.SimpleWriteln("AT+CIPSTATUS");
     // Alternative: AT+QISTAT, although it may be necessary to call an AT
     // command every second,which is not wise
     /*
     switch(gsm.WaitResp(1000, 200, "OK")){
     case RX_TMOUT_ERR:
     	return 0;
     break;
     case RX_FINISHED_STR_NOT_RECV:
     	return 0;
     break;
     }*/
     //gsm._tf.setTimeout(1);
     if(gsm.WaitResp(5000, 50, "CONNECT OK")!=RX_FINISHED_STR_RECV) {
          gsm.setStatus(gsm.TCPCONNECTEDSERVER);
          return true;
     } else
          return false;
}


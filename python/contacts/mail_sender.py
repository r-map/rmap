import smtplib
from email.MIMEMultipart import MIMEMultipart
from email.MIMEBase import MIMEBase
from email.MIMEText import MIMEText
from email.Utils import COMMASPACE, formatdate
from email import Encoders

import traceback  #print traceback.format_exc()

import json
import sys
import os
import re
import socket
import argparse

import rmap.settings as settings

#::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
#                     MAIL NOTIFICATION
#::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
def send_mail(subject, message, receivers, sender = settings.DEFAULT_FROM_EMAIL, files = [] ):
    #sender     <type 'email address'>
    #subject    <type 'str'>
    #message    <type 'str'>
    #receivers  <type 'array of email address'>
    
    try:
        #add signature
        message = message + "\n\n" + settings.RAINBO_MESSAGE_SIGNATURE
        
        #MAIL message generation
        msg            = MIMEMultipart()
        msg['From']    = sender
        msg['To']      = ", ".join(receivers)
        msg['Subject'] = subject

        msg.attach( MIMEText ( message ) )

        for f in files:
            part = MIMEBase('application', "octet-stream")
            part.set_payload( open(f,"rb").read() )
            Encoders.encode_base64(part)
            part.add_header('Content-Disposition', 'attachment; filename="%s"' %
            os.path.basename(f))
            msg.attach(part)
            

           
        #Send Mail
        server = smtplib.SMTP( settings.EMAIL_HOST )
        server.set_debuglevel(1)
        server.sendmail(sender, receivers, msg.as_string())
        server.quit()
        return True
    
    except:
        print traceback.format_exc()
        return traceback.format_exc()
    
    
#::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
#                       Command Line Execution
#::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
if __name__ == "__main__":
    
    parser = argparse.ArgumentParser(description='RAINBO email notification system')
    
    #parse arguments
    parser.add_argument('-s', '--sender',
                        required = False,
                        default  = socket.getfqdn() + "@noreplay.it",        
                        help     = "Sender.")
    
    parser.add_argument('-r', '--receivers',
                        required = True,
                        nargs    = '*', 
                        help     = "Array of receiver's address")

    parser.add_argument('-S', '--subject',
                        required = True,
                        help    = 'Mail subject')
    
    parser.add_argument('-m', '--message',
                        required = True,
                        help    = 'Mail message')
    
    parser.add_argument('-a', '--attachments',
                        required = False,
                        nargs    = '*', 
                        default  = [],          
                        help     = "Attachment file list with absolute path")
    
    args = parser.parse_args()

    
    send_mail( args.subject, args.message, args.receivers, args.sender, args.attachments )

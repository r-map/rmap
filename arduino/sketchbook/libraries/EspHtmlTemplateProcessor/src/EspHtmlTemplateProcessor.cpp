#include "EspHtmlTemplateProcessor.h"

EspHtmlTemplateProcessor::EspHtmlTemplateProcessor(WebServer* server) : mServer(server) {}
EspHtmlTemplateProcessor::~EspHtmlTemplateProcessor() {}


bool EspHtmlTemplateProcessor::processAndSend(const String& filePath, GetKeyValueCallback getKeyValueCallback)
{
  // Opening the file
  FileReader reader;
  if (!reader.open(filePath))
  {
    sendError("Unable to open " + filePath);
    return false;
  }

  // Preparing output
  mServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
  mServer->sendHeader("Content-Type", "text/html", true);
  mServer->sendHeader("Cache-Control", "no-cache");
  mServer->send(200);

  // Processing the file one char at the time
  char buffer[BUFFER_SIZE];
  unsigned int index = 0;
  unsigned int lastCurlyBracePosition;
  char ch;

  while (reader.readChar(ch))
  {
    // Template handling
    if (ch == OPENING_CURLY_BRACKET_CHAR)
    {
      // Clear out buffer if there is not enough space to handle 2 other readings
      if (index >= BUFFER_SIZE - 3)
      {
        buffer[index] = '\0';
        mServer->sendContent(buffer);
        index = 0;
      }

      lastCurlyBracePosition = reader.getCurrentPosition();
      buffer[index] = ch;
      index++;

      // Read the 2 next char to detect templating syntax
      for (unsigned int i = 0; i < 2 && reader.readChar(ch) ; i++)
      {
        buffer[index] = ch;
        index++;
      }

      // Wen we encounter two opening curly braces, it a template syntax that we must handle
      if (index >= 3 && buffer[index - 2] == OPENING_CURLY_BRACKET_CHAR && buffer[index - 3] == OPENING_CURLY_BRACKET_CHAR)
      {
        // if the last char is an escape char, remove it form the template and don't do the substitution
        if (buffer[index - 1] == ESCAPE_CHAR)
        {
          index--;
        }
        else
        {
          // if the buffer contain more than the last 3 chars, clear it omitting the last 3 chars 
          if (index > 3)
          {
            buffer[index - 3] = '\0';
            mServer->sendContent(buffer);
            // Set the first char of the key into the buffer as it was not an escape char
            buffer[0] = ch;
            index = 1;
          }

          // Extract the key for substitution
          bool found = false;
          char lastChar = ' ';

          while (!found && index < BUFFER_SIZE - 2 && reader.readChar(ch) && !reader.endOfLine())
          {
            if (ch == CLOSING_CURLY_BRACKET_CHAR && lastChar == CLOSING_CURLY_BRACKET_CHAR)
              found = true;
            else if (ch != CLOSING_CURLY_BRACKET_CHAR)
            {
              buffer[index] = ch;
              index++;
            }

            lastChar = ch;
          }

          // Check for bad exit.
          if (!found)
          {
            sendError("Parsing error, opening curly bracket found without corresponding closing brackets in file " + filePath + " at line " + String(reader.getCurrentLine()) + " position " + String(lastCurlyBracePosition) + "\n");
            return false;
          }

          // Get key value
          buffer[index] = '\0';
          mServer->sendContent(getKeyValueCallback(buffer));
          index = 0;
        }
      }
    }
    else 
    {
      buffer[index] = ch;
      index++;

      if (index >= BUFFER_SIZE-1)
      {
        buffer[BUFFER_SIZE-1] = '\0';
        mServer->sendContent(buffer);
        index = 0;
      }
    }
  }

  if (index > 0)
  {
    buffer[index] = '\0';
    mServer->sendContent(buffer);
  }

  mServer->sendContent("");

  return true;
}

void EspHtmlTemplateProcessor::sendError(const String& errorDescription) const
{
  mServer->sendContent("<br/><b>Error:</b> " + errorDescription);
  mServer->sendContent("");
  Serial.println("Error : " + errorDescription);
}

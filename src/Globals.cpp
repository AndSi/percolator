/*******************************************************************************
 Copyright 2006-2009 Lukas Käll <lukas.kall@cbr.su.se>

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 *******************************************************************************/

#include "Globals.h"
#include <iostream>
#include <tcutil.h>

Globals* Globals::glob = 0;

Globals::~Globals() {
  unredirectBuffer();
  if (log) {
    delete log;
    log = 0;
  }
}

Globals::Globals() {
  glob = this;
  verbose = 2;
  timeCheckPoint = false;
  fileLog = std::string(LOG_FILE);
  log = 0;
  buffer_redirected = false;
}

Globals* Globals::getInstance() {
  if (!glob) {
    new Globals();
  }
  return glob;
}

Logger* Globals::getLogger() {
  if(buffer_redirected)
  {
    std::cerr << "ERROR: cerr buffer is already being redirected.." << std::endl;
    exit(-1);
  }
  else if (!log) {
    initLogger();
  }
  return log;
}

void Globals::initLogger() {
  if(!fileLog.empty())
  {
    log = new Logger(fileLog.c_str());
  }
  else if(!log)
  {
    log = new Logger();
  }
}
void Globals::setLogFile(const std::string& filename) {
  if (!log) {
    initLogger();
  }
  log->attach_file(filename.c_str());
  fileLog = filename;
}

void Globals::checkTime(const std::string& message){
  if(timeCheckPoint){
    clock_t current = clock();
    std::cerr << "time required to " << message << ": " <<
        (current-checkTimeClock)/1000000 << " sec\n";
    checkTimeClock = clock(); // reset
  }
}

int Globals::redirectBuffer()
{
  if(!fileLog.empty())
  { 
    try
    {
      ferr.open (fileLog.c_str());
      save_sbuf_cerr = std::cerr.rdbuf();
      std::cerr.rdbuf(ferr.rdbuf());
      buffer_redirected = true;
      return 0;
    }catch (const std::exception& e)
    {
       std::cerr << "ERROR: " << e.what() << " redirecting cerr buffer.." << std::endl;
       return -1;
    }
  }
  else
  {
    std::cerr << "ERROR: trying to redirect cerr buffer to an empty file, have you called Globals::setLogFile or defined LOG_FILE?." << std::endl;
    return -1;
  }
}

void Globals::unredirectBuffer() {
  if(buffer_redirected)
  {
    std::cerr.rdbuf(save_sbuf_cerr);
    std::cerr.flush();
    ferr.close();
    buffer_redirected = false;
  }
  else if(log)
  {
    log->disactivate_file_log();
  }
}

void Globals::clean() {
  if (glob) {
    delete glob;
  }
  glob = 0;
}

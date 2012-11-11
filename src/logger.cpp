/* (C) Robert Kessl 2009
 */

#include "logger.hpp"
#include <time.h>
#include <iostream>
#include <string.h>
#include <cmath>
#include <stdio.h>

map<string, Logger *> *Logger::loggers = 0;
int Logger::log_message_number = 0;

static log_level_t log_level_from_env = DEFAULT_LOG_LEVEL;
static bool log_level_read = false;

#define MEGABYTES (1024*1024)

extern long get_program_size();

/** Manipulator that logs header of the log entry
 */
Logger & head(Logger& os)
{
  return os.loghead();
}

/** Manipulator that starts new entry
 */
Logger & ele(Logger & os) 
{
  if(os.get_detail_flag() == false) {
    os.put(os.widen('\n')).flush();
  }
  os.too_detailed(false);
  os.writing_le(false);
  return os;
}

Logger::Logger(const Logger &log) : ostream(log.rdbuf())
{
  logger_name = log.logger_name;
  loglevel = log.loglevel;
  local_timestamp = log.local_timestamp;
  log_head_format = log.log_head_format;
  log_message_number = log.log_message_number;
  stream_buffer = log.stream_buffer;
  _too_detailed = log._too_detailed;
  _this_entry_loglevel = log._this_entry_loglevel;
  have_source_information = false;
  _writing_le = false;
}

Logger::Logger(string logger_name, const char *log_head_format, message_type_t loglevel, int default_log_level, const ostream &output) 
  : ostream(output.rdbuf())
{
  //cout << "logger: " << logger_name << " constructed" << endl;
  this->logger_name = logger_name;
  this->loglevel = loglevel;
  this->local_timestamp = 0;
  this->log_head_format = (const char *)log_head_format;
  this->log_message_number = 0;
  this->_writing_le = false;
  stream_buffer = output.rdbuf();
  _too_detailed = false;
  _this_entry_loglevel = LG_INFO;
  have_source_information = false;
} // Logger


Logger::Logger(string logger_name, const char *log_head_format, message_type_t loglevel, int default_log_level, const char* filename)
  : ostream(new filebuf())
{
  
  //this->output = new ofstream(filename);
  this->logger_name = logger_name;
  this->loglevel = loglevel;
  this->local_timestamp = 0;
  this->log_head_format = log_head_format;
  this->log_message_number = 0;
  this->_writing_le = false;
  ((filebuf*)rdbuf())->open(filename, ios_base::out | ios_base::trunc);
  _too_detailed = false;
  _this_entry_loglevel = LG_INFO;
  have_source_information = false;
}

Logger::Logger(string logger_name, const char *log_head_format, message_type_t loglevel, int default_log_level, streambuf *obuf)
  : ostream(obuf)
{
  this->logger_name = logger_name;
  this->loglevel = loglevel;
  this->local_timestamp = 0;
  this->log_head_format = (const char *)log_head_format;
  this->log_message_number = 0;
  this->_writing_le = false;
  stream_buffer = obuf;//output.rdbuf();
  _too_detailed = false;
  _this_entry_loglevel = LG_INFO;
  have_source_information = false;
}


/** supported parameters:
 *    output.format
 *    output.
 *    logger.defaultlevel
 *    component.
 *    logger.name
 *    logger.mark
 */
void Logger::set_parameter(char *pname, char *value)
{
}


/** Zatim prijima nasledujici parametry:
 *  %p - cislo processoru
 *  %P - pocet procesoru
 *  %l - log level (text)
 *  %n - jmeno loggeru
 *  %T - timestamp (cas, datum ...)
 *  %t - cislo logovaci hlasky (v pripade ze je potreba ne absolutni cas, ale poradi logovaci hlasky). To obcas potreba je, pokud vystup ty hlasky prohazuje
 *  %a - cislo threadu
 *  %f - filename
 *  %L - line number
 */
Logger & Logger::loghead()
{
  if(_too_detailed == true) return *this;
  string result;
  unsigned int i;
  //  char *begin;
  char pnum[100];

  time_t ctime;
  tm *broken_ctime;

  _writing_le = true;

  for(i = 0; i < log_head_format.size(); i++) {
    if(log_head_format[i] == '\\' && log_head_format[i+1] == '%') {
      i++;
      result = result + log_head_format[i];
    } else if(log_head_format[i] == '%') {
      i++;
      switch(log_head_format[i]) {
      case 'P':
	sprintf(pnum, "%d", processor_count);
	result = result + pnum;
	break;
      case 'p':
	sprintf(pnum, "%d", processor_number);
	result = result + pnum;
	break;
      case 't':
	sprintf(pnum, "%d", log_message_number);
	log_message_number++;
	result =  result + pnum;
	break;
      case 'l':
	result = result + this_entry_loglevel();
	break;
      case 'n':
	result = result + logger_name;
	break;
      case 'T':
	time((time_t *)&ctime);
	broken_ctime = localtime((time_t *)&ctime);
	strftime(pnum, 100, "%b %d %H:%M:%S", broken_ctime);
	result = result + pnum;
	//strftime(pnum, 100, "%c", 
	break;
	/*      case 'a':
	//sprintf(pnum, "%ld", pthread_self());
	result = result + pnum;
	break;
	*/
      case 'L':
	if(have_source_information) {
	  sprintf(pnum, "%d", line);
	  result = result + pnum;
	} else result += "<line>";
	break;
      case 'f':
	if(have_source_information) {
	  result = result + file;
	} else result = result + "<file>";
	break;
      default:
	break;
      } // switch
    } else {
      result = result + log_head_format[i];
    } //if-else
  } // for i
  (*this) << result.c_str();
  return *this;
} // loghead


Logger *Logger::get_logger(string component, string format)
{
  if(log_level_read == false) log_level_from_env = get_log_level_from_environment();
  if(loggers == 0) loggers = new map<string, Logger *>;
  if(loggers->find(component) == loggers->end()) {
    loggers->insert(pair<string,Logger*>(component, new Logger(component, format.c_str(), log_level_from_env, 0, cout)));
  }
  return loggers->find(component)->second;
}


Logger *Logger::get_logger(string component, string format, string filename)
{
  if(log_level_read == false) log_level_from_env = get_log_level_from_environment();
  if(loggers == 0) loggers = new map<string, Logger *>;
  if(loggers->find(component) == loggers->end()) {
    loggers->insert(pair<string,Logger*>(component, new Logger(component, format.c_str(), log_level_from_env, 0, filename.c_str())));
  }
  return loggers->find(component)->second;
}

Logger *Logger::get_logger(string component, string format, streambuf *obuf)
{
  if(log_level_read == false) log_level_from_env = get_log_level_from_environment();
  if(loggers == 0) loggers = new map<string, Logger *>;
  if(loggers->find(component) == loggers->end()) {
    loggers->insert(pair<string,Logger*>(component, new Logger(component, format.c_str(), log_level_from_env, 0, obuf)));
  }
  return loggers->find(component)->second;
}


Logger *Logger::get_logger(string component)
{
  if(log_level_read == false) log_level_from_env = get_log_level_from_environment();
  if(loggers == 0) loggers = new map<string, Logger *>;
  if(loggers->find(component) == loggers->end()) {
    loggers->insert(pair<string,Logger*>(component, new Logger(component, "[%l] %n@%a [%f@%L] - ", log_level_from_env, 0, cout)));
  }
  return loggers->find(component)->second;
}

void Logger::free_loggers()
{
  for(map<string, Logger *>::iterator itL = loggers->begin(); itL != loggers->end(); itL++) {
    delete itL->second;
  } // for
  delete loggers;
  loggers = 0;
} // free_loggers





bool Logger::log_pct(double max, double current, double pct)
{
  //  return true;
  double maxiter = int(1.0L/(pct/100.0L));
  double tmp = floor(max/maxiter);
  if(current/tmp - floor(current/tmp) < 0.00001) {
    //cout << "max=" << max << "; current=" << current << "; tmp=" << tmp << "; current/tmp=" << current/tmp << "; floored=" << floor(current/tmp) << endl;
    //cout << "maxiter=" << maxiter << endl;
    time_t ctime;
    tm *broken_ctime;
    char pnum[100];

    time((time_t *)&ctime);
    broken_ctime = localtime((time_t *)&ctime);
    strftime(pnum, 100, "%b %d %H:%M:%S", broken_ctime);

    cout << pnum << " - done: " << nearbyint((current/max*100.0L)) << "%; prg size usage(MB): " << ((double)get_program_size())/((double)MEGABYTES) << endl;
    return true;
  }
  return false;
}

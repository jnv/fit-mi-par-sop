/* (C) Robert Kessl 2009
 */

#ifndef _LOGGER_HPP_
#define _LOGGER_HPP_
#include <fstream>
#include <map>
#include <iostream>
#include <vector>
#include <sstream>
#include <string.h>
#include <stdlib.h>

//#include "common.hpp"

#define DEFAULT_LOG_LEVEL LG_TRACE
typedef enum {LG_OFF, LG_CRITICAL_ERROR, LG_ERROR, LG_WARNING, LG_INFO, LG_DEBUG, LG_TRACE} message_type_t;

typedef message_type_t log_level_t;

using namespace std;

/* 
 * CRITICAL_ERROR -
 * ERROR - chyba ze ktere se obcas lze zotavit 
 * WARNING - chybne parametry, spatna konfigurace
 * INFO - informace pro usera, mozna by melo byt vice urovni INFO
 * DEBUG - detailni informace o behu
 * TRACE - vypisuje kazdy krok
 */


// if the user does not want to log anything he must use -DLOG_NOLOG
#ifndef LOG_NOLOG
#define _CRITICAL_ERROR() critical_error(__LINE__, __FILE__)
#define _ERROR() error(__LINE__, __FILE__)
#define _WARNING() warning(__LINE__, __FILE__)
#define _INFO() info(__LINE__, __FILE__)
#define _DEBUG() debug(__LINE__, __FILE__)
#define _TRACE() trace(__LINE__, __FILE__)


#define CRITICAL_ERROR(__log__, __message__) (__log__)._CRITICAL_ERROR() << __message__ << flush << ele
#define ERROR(__log__, __message__) (__log__)._ERROR() << __message__ << flush << ele
#define WARNING(__log__, __message__) (__log__)._WARNING() << __message__ << flush << ele
#define INFO(__log__, __message__) (__log__)._INFO() << __message__ << flush << ele
//#define DEBUG(__log__, __message__) (__log__)._DEBUG() << __message__ << flush << ele
//#define TRACE(__log__, __message__) (__log__)._TRACE() << __message__ << flush << ele
#define LOG_VALUE(__log__, __value_name__, __value__) INFO(__log__, "[" << __value_name__ << "=" << __value__ << "]")
#endif

// if user does not want to log anything ...
#ifdef LOG_NOLOG
#define CRITICAL_ERROR(__log__, __message__)
#define ERROR(__log__, __message__)
#define WARNING(__log__, __message__)
#define INFO(__log__, __message__)
#define LOG_VALUE(__log__, __value_name__, __value__)
#define DEBUG(__log__, __message__)
#define TRACE(__log__, __message__)
#endif

#ifdef LOG_DEBUG 
#define DEBUG(__log__, __message__) (__log__)._DEBUG() << __message__ << flush << ele
#define TRACE(__log__, __message__)
#endif

#ifdef LOG_TRACE
#define DEBUG(__log__, __message__) (__log__)._DEBUG() << __message__ << flush << ele
#define TRACE(__log__, __message__) (__log__)._TRACE() << __message__ << flush << ele
#endif

#if !defined(LOG_TRACE) && !defined(LOG_DEBUG)
#define DEBUG(__log__, __message__) 
#define TRACE(__log__, __message__)
#endif


class Logger;
Logger & head(Logger& os);
Logger & ele(Logger & os);


class Logger : public basic_ostream<char, char_traits<char> > {
  static int log_message_number;

  message_type_t loglevel;
  message_type_t _this_entry_loglevel;
  string logger_name;
  string log_head_format;

  static int global_timestamp;
  int local_timestamp;
  int processor_count;
  int processor_number;
  streambuf *stream_buffer;

  int line;
  string file;
  bool have_source_information;

  bool _too_detailed;
  bool _writing_le;
  static map<string, Logger *> *loggers;
public:
  //copy constructor, zkopiruje output, urovne logovani, jmeno komponenty
  Logger(const Logger &log);

  //Logger(const char *logger_name, const char *output_format, int loglevel, int default_log_level, int file_handle);
  Logger(string logger_name, const char *head_format, message_type_t loglevel, int default_log_level, const char* filename);
  Logger(string logger_name, const char *head_format, message_type_t loglevel, int default_log_level, const ostream &output);
  Logger(string logger_name, const char *output_format, message_type_t loglevel, int default_log_level, streambuf *obuf);

  virtual ~Logger() {}

  void set_processor_count(int pnum) { processor_count = pnum; }
  void set_processor_number(int pnum) { processor_number = pnum; }

  virtual void set_parameter(char *pname, char *value);

  void set_loglevel(message_type_t level) { loglevel = level; }
  void set_loglevel(const string &loglevel) { 
    if(loglevel == "ERROR") set_loglevel(LG_ERROR);
    else if(loglevel == "INFO") set_loglevel(LG_INFO);
    else if(loglevel == "WARNING") set_loglevel(LG_WARNING);
    else if(loglevel == "CRITICAL_ERROR") set_loglevel(LG_CRITICAL_ERROR);
    else if(loglevel == "DEBUG") set_loglevel(LG_DEBUG);
    else if(loglevel == "TRACE") set_loglevel(LG_TRACE);
  }
  int get_loglevel() { return loglevel; }

  string get_current_state() {
    stringstream ss;
    ss << "_too_detailed=" << _too_detailed << endl;
    ss << "_writing_le=" << _writing_le;
    return ss.str();
  }

  /*
  void set_default_loglevel() { loglevel = default_loglevel; }
  void set_next_entry_level(int level) { next_entry_level = level; }
  */

  Logger & critical_error() {
    if(_writing_le) (*this) << endl;
    _writing_le = true;
    have_source_information = false;
    _this_entry_loglevel = LG_CRITICAL_ERROR;
    if(loglevel < LG_CRITICAL_ERROR) _too_detailed = true; 
    return (*this << head);
  }

  Logger & critical_error(int line, string file) {
    if(_writing_le) (*this) << endl;
    _writing_le = true;
    have_source_information = true;
    this->line = line;
    this->file = file;
    _this_entry_loglevel = LG_CRITICAL_ERROR;
    if(loglevel < LG_CRITICAL_ERROR) _too_detailed = true; 
    return (*this << head);
  }

  Logger & error() {
    if(_writing_le) (*this) << endl;
    _writing_le = true;
    have_source_information = false;
    _this_entry_loglevel = LG_ERROR;
    if(loglevel < LG_ERROR) _too_detailed = true; 
    return (*this << head);
  }

  Logger & error(int line, string file) {
    if(_writing_le) (*this) << endl;
    _writing_le = true;
    have_source_information = true;
    this->line = line;
    this->file = file;
    _this_entry_loglevel = LG_ERROR;
    if(loglevel < LG_ERROR) _too_detailed = true; 
    return (*this << head);
  }

  Logger & warning() {
    if(_writing_le) (*this) << endl;
    _writing_le = true;
    have_source_information = false;
    _this_entry_loglevel = LG_WARNING;
    if(loglevel < LG_WARNING) _too_detailed = true; 
    return (*this << head);
  }

  Logger & warning(int line, string file) {
    if(_writing_le) (*this) << endl;
    _writing_le = true;
    have_source_information = true;
    this->line = line;
    this->file = file;
    _this_entry_loglevel = LG_WARNING;
    if(loglevel < LG_WARNING) _too_detailed = true; 
    return (*this << head);
  }

  Logger & info() {
    if(_writing_le) (*this) << endl;
    _writing_le = true;
    have_source_information = false;
    _this_entry_loglevel = LG_INFO;
    if(loglevel < LG_INFO) _too_detailed = true; 
    return (*this << head);
  }

  Logger & info(int line, string file) {
    if(_writing_le) (*this) << endl;
    _writing_le = true;
    have_source_information = true;
    this->line = line;
    this->file = file;
    _this_entry_loglevel = LG_INFO;
    if(loglevel < LG_INFO) _too_detailed = true; 
    return (*this << head);
  }


  Logger & debug() {
    if(_writing_le) (*this) << endl;
    _writing_le = true;
    have_source_information = false;
    _this_entry_loglevel = LG_DEBUG;
    if(loglevel < LG_DEBUG) _too_detailed = true; 
    return (*this << head);
  }

  Logger &debug(int line, string file) {
    if(_writing_le) (*this) << endl;
    _writing_le = true;
    have_source_information = true;
    this->line = line;
    string::size_type tmp = file.find_last_of("/\\");
    if(tmp != string::npos)
      file = file.substr(tmp+1);
    this->file = file;
    _this_entry_loglevel = LG_DEBUG;
    if(loglevel < LG_DEBUG) _too_detailed = true; 
    return (*this << head);
  }

  Logger & trace() {
    if(_writing_le) (*this) << endl;
    _writing_le = true;
    have_source_information = false;
    _this_entry_loglevel = LG_TRACE;
    if(loglevel < LG_TRACE) _too_detailed = true; 
    return (*this << head);
  }

  Logger & trace(int line, string file) {
    if(_writing_le) (*this) << endl;
    _writing_le = true;
    have_source_information = true;
    this->line = line;
    this->file = file;
    _this_entry_loglevel = LG_TRACE;
    if(loglevel < LG_TRACE) _too_detailed = true; 
    return (*this << head);
  }

  void too_detailed(bool b = true) { _too_detailed = b; }
  bool get_detail_flag() { return _too_detailed; }
  void writing_le(bool b = true) { _writing_le = b; }

  virtual void set_log_head_format(const char *fmt) {log_head_format = (char *) fmt;}
  virtual Logger & loghead();


  Logger &operator << (Logger & (*pf)(Logger &)) {
    if(pf == ele) return  (*pf)(*this);

    if(_too_detailed) return *this;
    return  (*pf)(*this);
  }

  Logger &operator << (Logger & (*pf)(basic_ostream<char, char_traits<char> > &)) {
    if(_too_detailed) return *this;
    return  (*pf)(*this);
  }

  Logger &operator << (basic_ostream<char, char_traits<char> > & (*pf)(Logger &)) {
    if(_too_detailed) return *this;
    return  (Logger &) (*pf)(*this);
  }

  Logger &operator << (basic_ostream<char, char_traits<char> > & (*pf)(basic_ostream<char, char_traits<char> > &)) {
    if(_too_detailed) return *this;
    return  (Logger &) (*pf)(*this);
  }

  template<class Type> Logger &operator << (Type t) {
    if(_too_detailed) return *this;
    (basic_ostream<char, char_traits<char> > &)*this << t;
    return  *this;
  }

  const char *this_entry_loglevel() {
    switch(_this_entry_loglevel) {
    case LG_CRITICAL_ERROR: return "CRITICAL_ERROR";
    case LG_ERROR: return "ERROR";
    case LG_WARNING: return "WARNING";
    case LG_INFO: return "INFO";
    case LG_DEBUG: return "DEBUG";
    case LG_TRACE: return "TRACE";
    }
    return "?????????????????????";
  }

  static const char *get_log_level(log_level_t l) {
    switch(l) {
    case LG_CRITICAL_ERROR: return "CRITICAL_ERROR";
    case LG_ERROR: return "ERROR";
    case LG_WARNING: return "WARNING";
    case LG_INFO: return "INFO";
    case LG_DEBUG: return "DEBUG";
    case LG_TRACE: return "TRACE";
    } // switch
  } // get_log_level

  static log_level_t get_log_level(const char *l) {
    if(strcmp(l, "CRITICAL_ERROR") == 0) return LG_CRITICAL_ERROR;
    else if(strcmp(l, "ERROR") == 0) return LG_ERROR;
    else if(strcmp(l, "WARNING") == 0) return LG_WARNING;
    else if(strcmp(l, "INFO") == 0) return LG_INFO;
    else if(strcmp(l, "DEBUG") == 0) return LG_DEBUG;
    else if(strcmp(l, "TRACE") == 0) return LG_TRACE;
    else return DEFAULT_LOG_LEVEL;
  }

  static log_level_t get_log_level_from_environment() {
    char * l = getenv("LOG_LEVEL");
    log_level_t ll = DEFAULT_LOG_LEVEL;
    if(l != 0) ll = get_log_level(l);
    //cout << "log level=" << get_log_level(ll) << endl;
    return ll;
  }

  static Logger *get_logger(string component);
  static Logger *get_logger(string component, string format);
  static Logger *get_logger(string component, string format, string filename);
  static Logger *get_logger(string component, string format, streambuf *obuf);
  static void free_loggers();

  static bool log_pct(double max, double current, double pct);
};

#endif

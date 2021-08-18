/*
 * 
 * K2HDKC
 * 
 * Copyright 2016 Yahoo Japan Corporation.
 * 
 * K2HDKC is k2hash based distributed KVS cluster.
 * K2HDKC uses K2HASH, CHMPX, FULLOCK libraries. K2HDKC supports
 * distributed KVS cluster server program and client libraries.
 * 
 * For the full copyright and license information, please view
 * the license file that was distributed with this source code.
 *
 * AUTHOR:   Takeshi Nakatani
 * CREATE:   Mon Jul 11 2016
 * REVISION:
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <libgen.h>

#include <k2hash/k2hutil.h>
#include <chmpx/chmpx.h>
#include <chmpx/chmconfutil.h>

#include "k2hdkccommon.h"
#include "k2hdkc.h"
#include "k2hdkccomnum.h"
#include "k2hdkcslave.h"
#include "k2hdkccom.h"
#include "k2hdkcdbg.h"

using namespace std;

//---------------------------------------------------------
// Symbols
//---------------------------------------------------------
#define	MAX_ONE_VALUE_LENGTH				(10 * 1024 * 1024)				// 10MB

typedef enum k2hdkclinetool_cas_type{
	CAS_TYPE_NO,
	CAS_TYPE_8,
	CAS_TYPE_16,
	CAS_TYPE_32,
	CAS_TYPE_64
}CASTYPE;

//---------------------------------------------------------
// Utilities for debugging
//---------------------------------------------------------
static inline void PRN(const char* format, ...)
{
	if(format){
		va_list ap;
		va_start(ap, format);
		vfprintf(stdout, format, ap); 
		va_end(ap);
	}
	fprintf(stdout, "\n");
}

static inline void DKCTOOL_PRINT(const char* prefix, const char* format, ...)
{
	if(!DKCEMPTYSTR(prefix)){
		fprintf(stderr, "%s ", prefix);
	}
	if(format){
		va_list ap;
		va_start(ap, format);
		vfprintf(stderr, format, ap); 
		va_end(ap);
	}
	fprintf(stderr, "\n");
}

inline std::string PRN_TIMESPEC(const timespec& ts)
{
	char	szBuff[64];
	string	strResult;

	if(0 < ts.tv_sec){
		sprintf(szBuff, "%zus ", ts.tv_sec);
		strResult += szBuff;
	}
	sprintf(szBuff, "%ldms ", (ts.tv_nsec % (1000 * 1000)));
	strResult += szBuff;

	sprintf(szBuff, "%ldus ", ((ts.tv_nsec % (1000 * 1000)) / 1000));
	strResult += szBuff;

	sprintf(szBuff, "%ldns ", (ts.tv_nsec % 1000));
	strResult += szBuff;

	if(0 < ts.tv_sec){
		sprintf(szBuff, "(%zu%09ldns)", ts.tv_sec, ts.tv_nsec);
	}else{
		sprintf(szBuff, "(%ldns)", ts.tv_nsec);
	}
	strResult += szBuff;

	return strResult;
}

// Print message mode
static bool is_print_msg = false;
static bool is_print_wan = false;

#define	MSG(...)			if(is_print_msg){ DKCTOOL_PRINT("[MSG]", __VA_ARGS__); }
#define	WAN(...)			if(is_print_wan){ DKCTOOL_PRINT("[WAN]", __VA_ARGS__); }
#define	ERR(...)			DKCTOOL_PRINT("[ERR]", __VA_ARGS__)

//---------------------------------------------------------
// Typedefs
//---------------------------------------------------------
typedef strarr_t						params_t;
typedef map<string, params_t>			option_t;

typedef struct option_type{
	const char*	option;
	const char*	norm_option;
	int			min_param_cnt;
	int			max_param_cnt;
}OPTTYPE, *POPTTYPE;

typedef	const struct option_type		*CPOPTTYPE;
typedef	const char						*const_pchar;

//---------------------------------------------------------
// Class LapTime
//---------------------------------------------------------
class LapTime
{
	private:
		static bool	isEnable;

	private:
		static bool Set(bool enable);

		struct timeval	start;

	public:
		static bool Toggle(void) { return Set(!LapTime::isEnable); }
		static bool Enable(void) { return Set(true); }
		static bool Disable(void) { return Set(false); }
		static bool IsEnable(void) { return isEnable; }

		LapTime();
		virtual ~LapTime();
};

bool LapTime::isEnable = false;

bool LapTime::Set(bool enable)
{
	bool	old = LapTime::isEnable;
	LapTime::isEnable = enable;
	return old;
}

LapTime::LapTime()
{
	memset(&start, 0, sizeof(struct timeval));
	gettimeofday(&start, NULL);
}

LapTime::~LapTime()
{
	if(LapTime::isEnable){
		struct timeval	end;
		struct timeval	lap;

		memset(&end, 0, sizeof(struct timeval));
		gettimeofday(&end, NULL);

		memset(&lap, 0, sizeof(struct timeval));
		timersub(&end, &start, &lap);

		time_t	hour, min, sec, msec, usec;

		sec	 = lap.tv_sec % 60;
		min	 = (lap.tv_sec / 60) % 60;
		hour = (lap.tv_sec / 60) / 60;
		msec = lap.tv_usec / 1000;
		usec = lap.tv_usec % 1000;

		PRN(NULL);
		PRN("Lap time: %jdh %jdm %jds %jdms %jdus(%jds %jdus)\n",
			static_cast<intmax_t>(hour),
			static_cast<intmax_t>(min),
			static_cast<intmax_t>(sec),
			static_cast<intmax_t>(msec),
			static_cast<intmax_t>(usec),
			static_cast<intmax_t>(lap.tv_sec),
			static_cast<intmax_t>(lap.tv_usec));
	}
}

//---------------------------------------------------------
// Class ConsoleInput
//---------------------------------------------------------
class ConsoleInput
{
	protected:
		static const int	DEFAULT_HISTORY_MAX	= 500;

		size_t				history_max;
		string				prompt;
		strarr_t			history;
		ssize_t				history_pos;
		string				input;
		size_t				input_pos;	// == cursor pos
		struct termios		tty_backup;
		bool				is_set_terminal;
		int					last_errno;

	protected:
		bool SetTerminal(void);
		bool UnsetTerminal(void);
		bool ReadByte(char& cInput);
		void ClearInput(void);
		void ClearLine(void);

	public:
		size_t SetMax(size_t max);
		size_t GetMax(void) const { return history_max; }
		bool SetPrompt(const char* pprompt);

		ConsoleInput();
		virtual ~ConsoleInput();

		bool Clean(void);
		bool GetCommand(void);
		bool PutHistory(const char* pCommand);
		bool RemoveLastHistory(void);
		int LastErrno(void) const { return last_errno; }
		const string& str(void) const { return input; }
		const char* c_str(void) const { return input.c_str(); }
		const strarr_t& GetAllHistory(void) const { return history; }
};

//
// Class ConsoleInput::Methods
//
ConsoleInput::ConsoleInput() : history_max(DEFAULT_HISTORY_MAX), prompt("PROMPT> "), history_pos(-1L), input(""), input_pos(0UL), is_set_terminal(false), last_errno(0)
{
}

ConsoleInput::~ConsoleInput()
{
	UnsetTerminal();
	Clean();
}

bool ConsoleInput::Clean(void)
{
	history.clear();
	prompt.clear();
	input.clear();
	return true;
}

size_t ConsoleInput::SetMax(size_t max)
{
	size_t	old = history_max;
	if(0 != max){
		history_max = max;
	}
	return old;
}

bool ConsoleInput::SetPrompt(const char* pprompt)
{
	if(DKCEMPTYSTR(pprompt)){
		return false;
	}
	prompt = pprompt;
	return true;
}

bool ConsoleInput::SetTerminal(void)
{
	if(is_set_terminal){
		// already set
		return true;
	}

	struct termios tty_change;

	// backup
	tcgetattr(0, &tty_backup);
	tty_change				= tty_backup;
	tty_change.c_lflag		&= ~(ECHO | ICANON);
	tty_change.c_cc[VMIN]	= 0;
	tty_change.c_cc[VTIME]	= 1;

	// set
	tcsetattr(0, TCSAFLUSH, &tty_change);
	is_set_terminal = true;

	return true;
}

bool ConsoleInput::UnsetTerminal(void)
{
	if(!is_set_terminal){
		// already unset
		return true;
	}

	// unset
	tcsetattr(0, TCSAFLUSH, &tty_backup);
	is_set_terminal = false;

	return true;
}

//
// If error occurred, return 0x00
//
bool ConsoleInput::ReadByte(char& cInput)
{
	cInput = '\0';
	if(-1 == read(0, &cInput, sizeof(char))){
		last_errno = errno;
		return false;
	}
	last_errno = 0;
	return true;
}

void ConsoleInput::ClearInput(void)
{
	history_pos	= -1L;
	input_pos	= 0UL;
	last_errno	= 0;
	input.erase();
}

void ConsoleInput::ClearLine(void)
{
	for(size_t Count = 0; Count < input_pos; Count++){		// cursor to head
		putchar('\x08');
	}
	for(size_t Count = 0; Count < input.length(); Count++){	// clear by space
		putchar(' ');
	}
	for(size_t Count = 0; Count < input.length(); Count++){	// rewind cursor to head
		putchar('\x08');
	}
	fflush(stdout);
}

// 
// [Input key value]
//	0x1b 0x5b 0x41			Up
//	0x1b 0x5b 0x42			Down
//	0x1b 0x5b 0x43			Right
//	0x1b 0x5b 0x44			Left
//	0x7f					Delete
//	0x08					backSpace
//	0x01					CTRL-A
//	0x05					CTRL-E
//	0x1b 0x5b 0x31 0x7e		HOME
//	0x1b 0x5b 0x34 0x7e		END
// 
bool ConsoleInput::GetCommand(void)
{
	ClearInput();
	SetTerminal();

	// prompt
	printf("%s", ConsoleInput::prompt.c_str());
	fflush(stdout);

	char	input_char;
	while(true){
		// read one character
		if(!ReadByte(input_char)){
			if(EINTR == last_errno){
				last_errno = 0;
				continue;
			}
			break;
		}
		if('\n' == input_char){
			// finish input one line
			putchar('\n');
			fflush(stdout);
			PutHistory(input.c_str());
			break;

		}else if('\x1b' == input_char){
			// escape character --> next byte read
			if(!ReadByte(input_char)){
				break;
			}
			if('\x5b' == input_char){
				// read more character
				if(!ReadByte(input_char)){
					break;
				}
				if('\x41' == input_char){
					// Up key
					if(0 != history_pos && 0 < history.size()){
						ClearLine();	// line clear

						if(-1L == history_pos){
							history_pos = static_cast<ssize_t>(history.size() - 1UL);
						}else if(0 != history_pos){
							history_pos--;
						}
						input = history[history_pos];

						for(input_pos = 0UL; input_pos < input.length(); input_pos++){
							putchar(input[input_pos]);
						}
						fflush(stdout);
					}

				}else if('\x42' == input_char){
					// Down key
					if(-1L != history_pos && static_cast<size_t>(history_pos) < history.size()){
						ClearLine();	// line clear

						if(history.size() <= static_cast<size_t>(history_pos) + 1UL){
							history_pos = -1L;
							input.erase();
							input_pos = 0UL;
						}else{
							history_pos++;
							input = history[history_pos];
							input_pos = input.length();

							for(input_pos = 0UL; input_pos < input.length(); input_pos++){
								putchar(input[input_pos]);
							}
							fflush(stdout);
						}
					}

				}else if('\x43' == input_char){
					// Right key
					if(input_pos < input.length()){
						putchar(input[input_pos]);
						fflush(stdout);
						input_pos++;
					}

				}else if('\x44' == input_char){
					// Left key
					if(0 < input_pos){
						input_pos--;
						putchar('\x08');
						fflush(stdout);
					}

				}else if('\x31' == input_char){
					// read more character
					if(!ReadByte(input_char)){
						break;
					}
					if('\x7e' == input_char){
						// Home key
						for(size_t Count = 0; Count < input_pos; Count++){
							putchar('\x08');
						}
						input_pos = 0UL;
						fflush(stdout);
					}

				}else if('\x34' == input_char){
					// read more character
					if(!ReadByte(input_char)){
						break;
					}
					if('\x7e' == input_char){
						// End key
						for(size_t Count = input_pos; Count < input.length(); Count++){
							putchar(input[Count]);
						}
						input_pos = input.length();
						fflush(stdout);
					}

				}else if('\x33' == input_char){
					// read more character
					if(!ReadByte(input_char)){
						break;
					}
					if('\x7e' == input_char){
						// BackSpace key on OSX
						if(0 < input_pos){
							input.erase((input_pos - 1), 1);
							input_pos--;
							putchar('\x08');
							for(size_t Count = input_pos; Count < input.length(); Count++){
								putchar(input[Count]);
							}
							putchar(' ');
							for(size_t Count = input_pos; Count < input.length(); Count++){
								putchar('\x08');
							}
							putchar('\x08');
							fflush(stdout);
						}
					}
				}
			}

		}else if('\x7f' == input_char){
			// Delete
			if(0 < input.length()){
				input.erase(input_pos, 1);

				for(size_t Count = input_pos; Count < input.length(); Count++){
					putchar(input[Count]);
				}
				putchar(' ');
				for(size_t Count = input_pos; Count < input.length(); Count++){
					putchar('\x08');
				}
				putchar('\x08');
				fflush(stdout);
			}

		}else if('\x08' == input_char){
			// BackSpace
			if(0 < input_pos){
				input.erase((input_pos - 1), 1);
				input_pos--;
				putchar('\x08');
				for(size_t Count = input_pos; Count < input.length(); Count++){
					putchar(input[Count]);
				}
				putchar(' ');
				for(size_t Count = input_pos; Count < input.length(); Count++){
					putchar('\x08');
				}
				putchar('\x08');
				fflush(stdout);
			}

		}else if('\x01' == input_char){
			// ctrl-A
			for(size_t Count = 0; Count < input_pos; Count++){
				putchar('\x08');
			}
			input_pos = 0;
			fflush(stdout);

		}else if('\x05' == input_char){
			// ctrl-E
			for(size_t Count = input_pos; Count < input.length(); Count++){
				putchar(input[Count]);
			}
			input_pos = input.length();
			fflush(stdout);

		}else if(isprint(input_char)){
			// normal character
			input.insert(input_pos, 1, input_char);
			for(size_t Count = input_pos; Count < input.length(); Count++){
				putchar(input[Count]);
			}
			input_pos++;
			for(size_t Count = input_pos; Count < input.length(); Count++){
				putchar('\x08');
			}
			fflush(stdout);
		}
	}
	UnsetTerminal();

	if(0 != last_errno){
		return false;
	}
	return true;
}

bool ConsoleInput::PutHistory(const char* pCommand)
{
	if(DKCEMPTYSTR(pCommand)){
		return false;
	}
	history.push_back(string(pCommand));
	if(ConsoleInput::history_max < history.size()){
		history.erase(history.begin());
	}
	return true;
}

bool ConsoleInput::RemoveLastHistory(void)
{
	if(0 < history.size()){
		history.pop_back();
	}
	return true;
}

//---------------------------------------------------------
// Global
//---------------------------------------------------------
static string	strConfig("");
static bool		isModeCAPI		= false;
static short	CntlPort		= CHM_INVALID_PORT;
static string	strCuk("");
static bool		isPermanent		= false;
static bool		isAutoRejoin	= false;
static bool		isNoGiveupRejoin= false;
static bool		isCleanupBup	= true;

//---------------------------------------------------------
// Utilities: Help
//---------------------------------------------------------
// 
// -help(h)           help display
// -conf <filename>   chmpx configuration file path
// -ctlport <port>    slave node chmpx control port
// -cuk <cuk string>  slave node chmpx cuk string
// -lap               print lap time after line command
// -capi              use C API for calling internal library
// -perm              use permanent chmpx handle
// -rejoin            chmpx auto rejoin if disconnect
// -nogiveup          no give up auto rejoin
// -nocleanup         not cleanup backup chmpx files
// -comlog [on | off] enable communication command for debug
// -d <debug level>   print debugging message mode: SILENT(SLT)/ERROR(ERR)/WARNING(WAN)/INFO(MSG)/DUMP(DMP)
// -dfile <file path> output file for debugging message(default stderr)
// -his <count>       set history count(default 500)
// -libversion        display k2hdkc library version
// -run <file path>   run command(history) file.
// 
static void Help(const char* progname)
{
	PRN(NULL);
	PRN("Usage: %s [-conf <file> | -json <string>] [-ctlport <port>] [-cuk <cuk string>] [options...]", progname ? progname : "program");
	PRN("       %s -help", progname ? progname : "program");
	PRN(NULL);
	PRN("Option -help(h)           help display");
	PRN("       -conf <filename>   k2hdkc configuration file path(.ini .yaml .json)");
	PRN("       -json <string>     k2hdkc configuration by json string");
	PRN("       -ctlport <port>    slave node chmpx control port");
	PRN("       -cuk <cuk string>  slave node chmpx cuk string");
	PRN("       -lap               print lap time after line command");
	PRN("       -capi              use C API for calling internal library");
	PRN("       -perm              use permanent chmpx handle");
	PRN("       -rejoin            chmpx auto rejoin if disconnect");
	PRN("       -nogiveup          no give up auto rejoin");
	PRN("       -nocleanup         not cleanup backup chmpx files");
	PRN("       -comlog [on | off] enable/disable communication command for debug");
	PRN("       -d <debug level>   print debugging message mode: SILENT(SLT)/ERROR(ERR)/WARNING(WAN)/INFO(MSG)/DUMP(DMP)");
	PRN("       -dfile <file path> output file for debugging message(default stderr)");
	PRN("       -his <count>       set history count(default 500)");
	PRN("       -libversion        display k2hdkc library version");
	PRN("       -run <file path>   run command(history) file.");
	PRN(NULL);
	PRN("(*) You can specify \"K2HDKCCONFFILE\" or \"K2HDKCJSONCONF\" environment instead of");
	PRN("    \"-conf\" or \"-json\" option for configuration.");
	PRN("(*) You can set debug level by another way which is setting environment as \"DKCDBGMODE\".");
	PRN("    \"DKCDBGMODE\" environment is took as \"SILENT(SLT)\", \"ERROR(ERR)\", \"WARNING(WAN)\",");
	PRN("    \"INFO(MSG)\" or \"DUMP(DMP)\" value.");
	PRN("    When this process gets SIGUSR1 signal, the debug level is bumpup.");
	PRN("    (The debug level is changed as \"SLT\"->\"ERR\"->\"WAN\"->\"MSG\"->\"DMP\"->...)");
	PRN("(*) You can set debugging message log file by the environment \"DKCDBGFILE\".");
	PRN(NULL);
}

// 
// Command: [command] [parameters...]
// 
// help(h)                                                                                       print help
// quit(q)/exit                                                                                  quit
// print(p) <key> [all] [noattrcheck] [pass=....] [dump]                                         print value by key(noattrcheck and pass parameter are mutually exclusive).
// directprint(dp) <key> <length> <offset> [dump]                                                print value from offset and length directly.
// printattr(pa) <key> [name=<attr name>] [dump]                                                 print attributes.
// copyfile(cf) <key> <length> <offset> <file>                                                   output directly key-value to file.
// set(s) <key> <value> [rmsublist | rmsub] [noattrcheck] [pass=....] [expire=sec]               set value(if value is "null", it means no value).
// directset(dset) <key> <value> <offset>                                                        set value from offset directly.
// setfile(sf) <key> <offset> <file>                                                             set directly key-value from file.
// fill(f) <prefix> <value> <count> [rmsublist | rmsub] [noattrcheck] [pass=....] [expire=sec]   set key-value by prefix repeating by count
// fillsub(fs) <parent key> <prefix> <value> <count> [noattrcheck] [pass=....] [expire=sec]      set key-value under parent key by prefix repeating by count
// rm(del) <key> [all]                                                                           delete(remove) key, if all parameter is specified, remove all sub key under key
// setsub(ss) <parent key> <key> [noattrcheck]                                                   set only subkey name into parent key.
// setsub(ss) <parent key> <key> <value> [noattrcheck] [pass=....] [expire=sec]                  set subkey-value under parent key. if need to set null value, must specify "null".
// rmsub(delsub) <parent key> [<key> | all] [rmsub] [noattrcheck]                                remove key under parent key
// rename(ren) <key> <new key>                                                                   rename key to new key name
// queue(que) <name> push <fifo | lifo> <value> [noattrcheck] [pass=....] [expire=sec]           push the value to queue(fifo/lifo)
// queue(que) <name> pop <fifo | lifo> [noattrcheck] [pass=...] [dump]                           pop the value from queue
// queue(que) <name> remove <fifo | lifo> <count> [pass=...]                                     remove count of values in queue
// keyqueue(kque) <name> push <fifo | lifo> <key> <value> [noattrcheck] [pass=....] [expire=sec] push the key to queue(fifo/lifo)
// keyqueue(kque) <name> pop <fifo | lifo> [noattrcheck] [pass=...] [dump]                       pop the key from queue
// keyqueue(kque) <name> remove <fifo | lifo> <count> [pass=...]                                 remove count of keys in queue
// cas[8 | 16 | 32 | 64] init <key> <value> [pass=....] [expire=sec]                             initialize compare and swap key(default value size is 8, and value is 0)
// cas[8 | 16 | 32 | 64] get <key> [pass=....]                                                   get compare and swap key(default value size is 8)
// cas[8 | 16 | 32 | 64] set <key> <old value> <new value> [pass=....] [expire=sec]              set compare and swap key(default value size is 8)
// cas [increment(inc) | decrement(dec)] <key> [pass=....] [expire=sec]                          increment/decrement compare and swap key
// status chmpx [self] [full]                                                                    print chmpx status
// status node [full]                                                                            print k2hash states on all server nodes
// comlog <on | off>                                                                             toggle or enable/disable communication command log
// dbglevel [slt | err | wan | msg | dmp]                                                        bumpup debugging level or specify level
// history(his)                                                                                  display all history, you can use a command line in history by "!<number>".
// save <file path>                                                                              save history to file.
// load <file path>                                                                              load and run command file.
// shell                                                                                         exit shell(same as "!" command).
// echo <string>...                                                                              echo string
// sleep <second>                                                                                sleep seconds
// 
static void LineHelp(void)
{
	PRN(NULL);
	PRN("Command: [command] [parameters...]");
	PRN(NULL);
	PRN("help(h)                                                 print help");
	PRN("quit(q)/exit                                            quit");
	PRN("print(p) <key> [all] [noattrcheck] [pass=....] [dump]   print value by key(noattrcheck and pass parameter are mutually exclusive).");
	PRN("directprint(dp) <key> <length> <offset> [dump]          print value from offset and length directly.");
	PRN("printattr(pa) <key> [name=<attr name>] [dump]           print attributes.");
	PRN("copyfile(cf) <key> <length> <offset> <file>             output directly key-value to file.");
	PRN("set(s) <key> <value> [rmsublist | rmsub] [noattrcheck] [pass=....] [expire=sec]");
	PRN("                                                        set value(if value is \"null\", it means no value).");
	PRN("directset(dset) <key> <value> <offset>                  set value from offset directly.");
	PRN("setfile(sf) <key> <offset> <file>                       set directly key-value from file.");
	PRN("fill(f) <prefix> <value> <count> [rmsublist | rmsub] [noattrcheck] [pass=....] [expire=sec]");
	PRN("                                                        set key-value by prefix repeating by count.");
	PRN("fillsub(fs) <parent key> <prefix> <value> <count> [noattrcheck] [pass=....] [expire=sec]");
	PRN("                                                        set key-value under parent key by prefix repeating by count");
	PRN("rm(del) <key> [all]                                     delete(remove) key, if all parameter is specified, remove all sub key under key");
	PRN("setsub(ss) <parent key> <key> [noattrcheck]             set only subkey name into parent key.");
	PRN("setsub(ss) <parent key> <key> <value> [noattrcheck] [pass=....] [expire=sec]");
	PRN("                                                        set subkey-value under parent key. if need to set null value, must specify \"null\".");
	PRN("rmsub(delsub) <parent key> [<key> | all] [rmsub] [noattrcheck]");
	PRN("                                                        remove key under parent key.");
	PRN("rename(ren) <key> <new key> [parent=<key>] [noattrcheck] [pass=....] [expire=sec]");
	PRN("                                                        rename key to new key name.");
	PRN("queue(que) <name> push <fifo | lifo> <value> [noattrcheck] [pass=....] [expire=sec]");
	PRN("                                                        push the value to queue(fifo/lifo)");
	PRN("queue(que) <name> pop <fifo | lifo> [noattrcheck] [pass=...] [dump]");
	PRN("                                                        pop the value from queue");
	PRN("queue(que) <name> remove <fifo | lifo> <count> [pass=...]");
	PRN("                                                        remove count of values in queue");
	PRN("keyqueue(kque) <name> push <fifo | lifo> <key> <value> [noattrcheck] [pass=....] [expire=sec]");
	PRN("                                                        push the key to queue(fifo/lifo)");
	PRN("keyqueue(kque) <name> pop <fifo | lifo> [noattrcheck] [pass=...] [dump]");
	PRN("                                                        pop the key from queue");
	PRN("keyqueue(kque) <name> remove <fifo | lifo> <count> [pass=...]");
	PRN("                                                        remove count of keys in queue");
	PRN("cas[8 | 16 | 32 | 64] init <key> <value> [pass=....] [expire=sec]");
	PRN("                                                        initialize compare and swap key(default value size is 8, and value is 0)");
	PRN("cas[8 | 16 | 32 | 64] get <key> [pass=....]             get compare and swap key(default value size is 8)");
	PRN("cas[8 | 16 | 32 | 64] set <key> <old value> <new value> [pass=....] [expire=sec]");
	PRN("                                                        set compare and swap key(default value size is 8)");
	PRN("cas [increment(inc) | decrement(dec)] <key> [pass=....] [expire=sec]");
	PRN("                                                        increment/decrement compare and swap key");
	PRN("status chmpx [self] [full]                              print chmpx status");
	PRN("status node [full]                                      print k2hash states on all server nodes");
	PRN("comlog [on | off]                                       toggle or enable/disable communication command log");
	PRN("dbglevel [slt | err | wan | msg | dmp]                  bumpup debugging level or specify level");
	PRN("history(his)                                            display all history, you can use a command line in history by \"!<number>\".");
	PRN("save <file path>                                        save history to file.");
	PRN("load <file path>                                        load and run command file.");
	PRN("shell                                                   exit shell(same as \"!\" command).");
	PRN("echo <string>...                                        echo string");
	PRN("sleep <second>                                          sleep seconds");
	PRN(NULL);
}

//---------------------------------------------------------
// Utilities: Command Parser
//---------------------------------------------------------
const OPTTYPE ExecOptionTypes[] = {
	{"-help",			"-help",			0,	0},
	{"-h",				"-help",			0,	0},
	{"-conf",			"-conf",			1,	1},
	{"-json",			"-json",			1,	1},
	{"-ctlport",		"-ctlport",			1,	1},
	{"-cntlport",		"-ctlport",			1,	1},
	{"-cntrlport",		"-ctlport",			1,	1},
	{"-cuk",			"-cuk",				1,	1},
	{"-lap",			"-lap",				0,	0},
	{"-capi",			"-capi",			0,	0},
	{"-perm",			"-permanent",		0,	0},
	{"-permanent",		"-permanent",		0,	0},
	{"-rejoin",			"-rejoin",			0,	0},
	{"-nogiveup",		"-nogiveup",		0,	0},
	{"-nocleanup",		"-nocleanup",		0,	0},
	{"-comlog",			"-comlog",			1,	1},
	{"-d",				"-d",				1,	1},
	{"-g",				"-d",				1,	1},		// for compatible k2hlinetool
	{"-dfile",			"-dfile",			1,	1},
	{"-glog",			"-dfile",			1,	1},		// for compatible k2hlinetool
	{"-history",		"-history",			1,	1},
	{"-his",			"-history",			1,	1},
	{"-libversion",		"-libversion",		0,	0},
	{"-run",			"-run",				1,	1},
	{NULL,				NULL,				0,	0}
};

const OPTTYPE LineOptionTypes[] = {
	{"help",			"help",				0,	0},
	{"h",				"help",				0,	0},
	{"quit",			"quit",				0,	0},
	{"q",				"quit",				0,	0},
	{"exit",			"quit",				0,	0},
	{"p",				"print",			1,	5},
	{"print",			"print",			1,	5},
	{"dp",				"dp",				3,	4},
	{"dprint",			"dp",				3,	4},
	{"directprint",		"dp",				3,	4},
	{"pa",				"pa",				1,	3},
	{"printattr",		"pa",				1,	3},
	{"cf",				"cf",				4,	4},
	{"copyf",			"cf",				4,	4},
	{"copyfile",		"cf",				4,	4},
	{"s",				"set",				2,	6},
	{"set",				"set",				2,	6},
	{"dset",			"dset",				3,	3},
	{"directset",		"dset",				3,	3},
	{"setdirect",		"dset",				3,	3},
	{"sf",				"sf",				3,	3},
	{"setf",			"sf",				3,	3},
	{"setfile",			"sf",				3,	3},
	{"fset",			"sf",				3,	3},
	{"fileset",			"sf",				3,	3},
	{"f",				"f",				3,	7},
	{"fill",			"f",				3,	7},
	{"fs",				"fs",				4,	7},
	{"fillsub",			"fs",				4,	7},
	{"rm",				"rm",				1,	2},
	{"del",				"rm",				1,	2},
	{"ss",				"ss",				2,	6},
	{"setsub",			"ss",				2,	6},
	{"rs",				"rs",				2,	4},
	{"rmsub",			"rs",				2,	4},
	{"delsub",			"rs",				2,	4},
	{"ren",				"ren",				2,	6},
	{"rename",			"ren",				2,	6},
	{"mv",				"ren",				2,	6},
	{"q",				"q",				3,	7},
	{"que",				"q",				3,	7},
	{"queue",			"q",				3,	7},
	{"kq",				"kq",				3,	8},
	{"kque",			"kq",				3,	8},
	{"keyqueue",		"kq",				3,	8},
	{"cas",				"cas8",				2,	6},
	{"cas8",			"cas8",				2,	6},
	{"cas16",			"cas16",			2,	6},
	{"cas32",			"cas32",			2,	6},
	{"cas64",			"cas64",			2,	6},
	{"st",				"st",				1,	3},
	{"sts",				"st",				1,	3},
	{"stat",			"st",				1,	3},
	{"stats",			"st",				1,	3},
	{"status",			"st",				1,	3},
	{"comlog",			"comlog",			0,	1},
	{"dbglevel",		"dbglevel",			0,	1},
	{"history",			"history",			0,	0},
	{"his",				"history",			0,	0},
	{"save",			"save",				1,	1},
	{"load",			"load",				1,	1},
	{"shell",			"shell",			0,	0},
	{"sh",				"shell",			0,	0},
	{"echo",			"echo",				1,	9999},
	{"sleep",			"sleep",			1,	1},
	{NULL,				NULL,				0,	0}
};

inline void CleanOptionMap(option_t& opts)
{
	for(option_t::iterator iter = opts.begin(); iter != opts.end(); opts.erase(iter++)){
		iter->second.clear();
	}
}

static bool BaseOptionParser(strarr_t& args, CPOPTTYPE pTypes, option_t& opts)
{
	if(!pTypes){
		return false;
	}
	opts.clear();

	for(size_t Count = 0; Count < args.size(); Count++){
		if(0 < args[Count].length() && '#' == args[Count].at(0)){
			// comment line
			return false;
		}
		size_t Count2;
		for(Count2 = 0; pTypes[Count2].option; Count2++){
			if(0 == strcasecmp(args[Count].c_str(), pTypes[Count2].option)){
				if(args.size() < ((Count + 1) + pTypes[Count2].min_param_cnt)){
					ERR("Option(%s) needs %d parameter.", args[Count].c_str(), pTypes[Count2].min_param_cnt);
					return false;
				}

				size_t		Count3;
				params_t	params;
				params.clear();
				for(Count3 = 0; Count3 < static_cast<size_t>(pTypes[Count2].max_param_cnt); Count3++){
					if(args.size() <= ((Count + 1) + Count3)){
						break;
					}
					params.push_back(args[(Count + 1) + Count3].c_str());
				}
				Count += Count3;
				opts[pTypes[Count2].norm_option] = params;
				break;
			}
		}
		if(!pTypes[Count2].option){
			ERR("Unknown option(%s).", args[Count].c_str());
			return false;
		}
	}
	return true;
}

static bool ExecOptionParser(int argc, char** argv, option_t& opts, string& prgname)
{
	if(0 == argc || !argv){
		return false;
	}
	prgname = basename(argv[0]);

	strarr_t	args;
	for(int nCnt = 1; nCnt < argc; nCnt++){
		args.push_back(argv[nCnt]);
	}

	opts.clear();
	return BaseOptionParser(args, ExecOptionTypes, opts);
}

static bool LineOptionParser(const char* pCommand, option_t& opts)
{
	opts.clear();

	if(!pCommand){
		return false;
	}
	if(0 == strlen(pCommand)){
		return true;
	}

	strarr_t	args;
	string		strParameter;
	bool		isMakeParamter	= false;
	bool		isQuart			= false;
	for(const_pchar pPos = pCommand; '\0' != *pPos && '\n' != *pPos; ++pPos){
		if(isMakeParamter){
			// keeping parameter
			if(isQuart){
				// pattern: "...."
				if('\"' == *pPos){
					if(0 == isspace(*(pPos + sizeof(char))) && '\0' != *(pPos + sizeof(char))){
						ERR("Quart is not matching.");
						return false;
					}
					// end of quart
					isMakeParamter	= false;
					isQuart			= false;

				}else if('\\' == *pPos && '\"' == *(pPos + sizeof(char))){
					// escaped quart
					pPos++;
					strParameter += *pPos;
				}else{
					strParameter += *pPos;
				}

			}else{
				// normal pattern
				if(0 == isspace(*pPos)){
					if('\\' == *pPos){
						continue;
					}
					strParameter += *pPos;
				}else{
					isMakeParamter = false;
				}
			}
			if(!isMakeParamter){
				// end of one parameter
				if(0 < strParameter.length()){
					args.push_back(strParameter);
					strParameter.clear();
				}
			}
		}else{
			// not keeping parameter
			if(0 == isspace(*pPos)){
				strParameter.clear();
				isMakeParamter	= true;

				if('\"' == *pPos){
					isQuart		= true;
				}else{
					isQuart		= false;

					if('\\' == *pPos){
						// found escape character
						pPos++;
						if('\0' == *pPos || '\n' == *pPos){
							break;
						}
					}
					strParameter += *pPos;
				}
			}
			// skip space
		}
	}
	// last check
	if(isMakeParamter){
		if(isQuart){
			ERR("Quart is not matching.");
			return false;
		}
		if(0 < strParameter.length()){
			args.push_back(strParameter);
			strParameter.clear();
		}
	}

	if(!BaseOptionParser(args, LineOptionTypes, opts)){
		return false;
	}
	if(1 < opts.size()){
		ERR("Too many option parameter.");
		return false;
	}
	return true;
}

//---------------------------------------------------------
// Utilities : Read from file
//---------------------------------------------------------
//
// Return: if left lines, returns true.
//
static bool ReadLine(int fd, string& line)
{
	line.erase();
	while(true){
		char	szBuff;
		ssize_t	readlength;

		szBuff = '\0';
		// read one character
		if(-1 == (readlength = read(fd, &szBuff, 1))){
			line.erase();
			return false;
		}

		// check EOF
		if(0 == readlength){
			return false;
		}

		// check character
		if('\r' == szBuff || '\0' == szBuff){
			// skip words

		}else if('\n' == szBuff){
			// skip comment line & no command line
			bool	isSpace		= true;
			bool	isComment	= false;
			for(size_t cPos = 0; isSpace && cPos < line.length(); cPos++){
				if(0 == isspace(line.at(cPos))){
					isSpace = false;

					if('#' == line.at(cPos)){
						isComment = true;
					}
					break;
				}
			}
			if(!isComment && !isSpace){
				break;
			}
			// this line is comment or empty, so read next line.
			line.erase();

		}else{
			line += szBuff;
		}
	}
	return true;
}

//
// Get File size
//
static bool GetFileSize(const char* pFile, size_t& length)
{
	if(DKCEMPTYSTR(pFile)){
		ERR("Parameter is wrong.");
		return false;
	}

	// file size
	struct stat	st;
	if(-1 == stat(pFile, &st)){
		ERR("Could not get stat for file(%s) by errno(%d)", pFile, errno);
		return false;
	}
	length = static_cast<size_t>(st.st_size);
	return true;
}

//
// Read from file
//
static bool ReadContentsFromFile(const char* pFile, off_t offset, size_t reqlength, unsigned char** ppval, size_t& vallength)
{
	if(DKCEMPTYSTR(pFile) || !ppval){
		ERR("Parameter is wrong.");
		return false;
	}
	*ppval		= NULL;
	vallength	= 0UL;

	// file open
	int	fd;
	if(-1 == (fd = open(pFile, O_RDONLY))){
		ERR("Could not open file(%s).", pFile);
		return false;
	}

	// file size
	struct stat	st;
	if(-1 == fstat(fd, &st)){
		ERR("Could not get stat for file(%s) by errno(%d)", pFile, errno);
		DKC_CLOSE(fd);
		return false;
	}
	if(st.st_size <= 0){
		ERR("file(%s) does not have any contents", pFile);
		DKC_CLOSE(fd);
		return false;
	}
	if(static_cast<off_t>(st.st_size) <= offset){
		ERR("file(%s) does not have any contents after %zd offset", pFile, offset);
		DKC_CLOSE(fd);
		return false;
	}

	// allocation
	if(0 == reqlength || static_cast<size_t>(st.st_size) < reqlength){
		reqlength = static_cast<size_t>(st.st_size);
	}
	if(NULL == ((*ppval) = reinterpret_cast<unsigned char*>(malloc(reqlength)))){
		ERR("Could not allocate memory.");
		DKC_CLOSE(fd);
		return false;
	}

	// read
	ssize_t	read_cnt;
	ssize_t	one_read;
	for(read_cnt = 0L, one_read = 0L; static_cast<size_t>(read_cnt) < reqlength; read_cnt += one_read){
		if(-1 == (one_read = pread(fd, &((*ppval)[read_cnt]), (reqlength - static_cast<size_t>(read_cnt)), static_cast<off_t>(offset + read_cnt)))){
			ERR("Failed to read from fd(%d : %jd : %zu) by errno(%d)", fd, static_cast<intmax_t>(offset + read_cnt), reqlength - static_cast<size_t>(read_cnt), errno);
			DKC_CLOSE(fd);
			DKC_FREE(*ppval);
			vallength = 0UL;
			return false;
		}
		if(0 == one_read){
			break;
		}
	}
	if(read_cnt <= 0){
		ERR("Could not read any byte from file(%s), it should be %zu byte", pFile, reqlength);
		DKC_CLOSE(fd);
		DKC_FREE(*ppval);
		vallength = 0UL;
		return false;
	}
	if(static_cast<size_t>(read_cnt) < reqlength){
		WAN("Read only %zd byte from file(%s), it should be %zu byte", read_cnt, pFile, reqlength);
	}
	vallength = static_cast<size_t>(read_cnt);
	DKC_CLOSE(fd);

	return true;
}

//
// Write(Append) to file
//
static bool AppendContentsToFile(const char* pFile, const unsigned char* pval, size_t vallength)
{
	if(DKCEMPTYSTR(pFile) || !pval || 0 == vallength){
		ERR("Parameter is wrong.");
		return false;
	}

	// file open
	int	fd;
	if(-1 == (fd = open(pFile, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH))){
		ERR("Could not open file(%s).", pFile);
		return false;
	}
	// write
	for(ssize_t write_cnt = 0L, one_write = 0L; static_cast<size_t>(write_cnt) < vallength; write_cnt += one_write){
		if(-1 == (one_write = write(fd, &(pval[write_cnt]), (vallength - static_cast<size_t>(write_cnt))))){
			ERR("Failed to write to fd(%d:%zd:%zu), errno = %d", fd, write_cnt, vallength - static_cast<size_t>(write_cnt), errno);
			DKC_CLOSE(fd);
			return false;
		}
	}
	return true;
}

//---------------------------------------------------------
// Utilities : Print datas
//---------------------------------------------------------
#define	BINARY_DUMP_BYTE_SIZE		16

static string GetHeadNestString(int nestcnt, const char* nestpref)
{
	string	strNest("");
	if(0 < nestcnt){
		for(int cnt = 0; cnt < nestcnt; ++cnt){
			strNest += "  ";
		}
		if(!DKCEMPTYSTR(nestpref)){
			strNest += nestpref;
		}
	}
	return strNest;
}

static string BinaryDumpLineUtility(const unsigned char* value, size_t vallen)
{
	if(!value || 0 == vallen){
		return string("");
	}

	const char*	pstrvalue = reinterpret_cast<const char*>(value);
	string		stroutput;
	string		binoutput;
	char		szBuff[8];
	size_t		pos;
	for(pos = 0; pos < vallen && pos < BINARY_DUMP_BYTE_SIZE; pos++){
		if(0 != isprint(pstrvalue[pos])){
			stroutput += value[pos];
		}else{
			stroutput += static_cast<char>(0xFF);
		}

		sprintf(szBuff, "%02X ", value[pos]);
		binoutput += szBuff;
		if(8 == (pos + 1)){
			binoutput += " ";
		}
	}
	for(; pos < BINARY_DUMP_BYTE_SIZE; pos++){
		stroutput += " ";
	}
	stroutput += "    ";

	// append
	stroutput += binoutput;

	return stroutput;
}

static bool BinaryDumpUtility(const char* prefix, const unsigned char* value, size_t vallen, int nestcnt)
{
	if(!value || 0 == vallen){
		return false;
	}

	string	strpref = GetHeadNestString(nestcnt, "+");
	if(DKCEMPTYSTR(prefix)){
		strpref += "VALUE(DUMP) => ";
	}else{
		strpref += "\"";
		strpref += prefix;
		strpref += "\" => ";
	}
	string	spacer;
	for(size_t len = strpref.size(); 0 < len; len--){
		spacer += " ";
	}

	string	output;
	string	line;
	size_t	restcnt = vallen;
	for(size_t pos = 0; pos < vallen; pos += BINARY_DUMP_BYTE_SIZE){
		if(0 == pos){
			output += strpref;
		}else{
			output += "\n";
			output += spacer;
		}
		line	= BinaryDumpLineUtility(&value[pos], restcnt);
		output += line;
		restcnt-= (BINARY_DUMP_BYTE_SIZE <= restcnt ? BINARY_DUMP_BYTE_SIZE : restcnt);
	}
	PRN("%s", output.c_str());

	return true;
}

static char* GetPrintableString(const unsigned char* byData, size_t length)
{
	if(!byData || 0 == length){
		length = 0;
	}
	char*	result;
	if(NULL == (result = reinterpret_cast<char*>(calloc(length + 1, sizeof(char))))){
		ERR("Could not allocate memory.");
		return NULL;
	}
	for(size_t pos = 0; pos < length; ++pos){
		result[pos] = isprint(byData[pos]) ? byData[pos] : (0x00 != byData[pos] ? 0xFF : (pos + 1 < length ? ' ' : 0x00));
	}
	return result;
}

//---------------------------------------------------------
// Command Processing
//---------------------------------------------------------
static bool CommandStringHandle(k2hdkc_chmpx_h chmpxhandle, ConsoleInput& InputIF, const char* pCommand, bool& is_exit);

//
// Command Line: comlog <on | off>
//
static bool ComlogCommand(params_t& params)
{
	bool	now_enable;
	if(isModeCAPI){
		now_enable = k2hdkc_is_enable_comlog();
	}else{
		now_enable = K2hdkcComNumber::IsEnable();
	}

	if(params.empty()){
		// toggle comlog mode
		if(!now_enable){
			if(isModeCAPI){
				k2hdkc_enable_comlog();
			}else{
				K2hdkcComNumber::Enable();
			}
			PRN("Enable communication log.");
		}else{
			if(isModeCAPI){
				k2hdkc_disable_comlog();
			}else{
				K2hdkcComNumber::Disable();
			}
			PRN("Disable communication log.");
		}

	}else if(1 < params.size()){
		ERR("Unknown parameter(%s) for comlog command.", params[1].c_str());

	}else{
		if(0 == strcasecmp(params[0].c_str(), "on") || 0 == strcasecmp(params[0].c_str(), "yes") || 0 == strcasecmp(params[0].c_str(), "y")){
			// enable
			if(now_enable){
				ERR("Already comlog is enabled.");
			}else{
				if(isModeCAPI){
					k2hdkc_enable_comlog();
				}else{
					K2hdkcComNumber::Enable();
				}
				PRN("Enable communication log.");
			}

		}else if(0 == strcasecmp(params[0].c_str(), "off") || 0 == strcasecmp(params[0].c_str(), "no") || 0 == strcasecmp(params[0].c_str(), "n")){
			// disable
			if(!now_enable){
				ERR("Already comlog is disabled.");
			}else{
				if(isModeCAPI){
					k2hdkc_disable_comlog();
				}else{
					K2hdkcComNumber::Disable();
				}
				PRN("Disable communication log.");
			}
		}else{
			ERR("Unknown parameter(%s) for comlog command.", params[0].c_str());
		}
	}
	return true;	// for continue.
}

//
// Command Line: dbglevel [slt | err | wan | msg | dmp]       bumpup debugging level or specify level
//
static bool DbglevelCommand(params_t& params)
{
	K2hdkcDbgMode	mode = GetK2hdkcDbgMode();

	if(params.empty()){
		// bumpup
		if(isModeCAPI){
			k2hdkc_bump_debug_level();
		}else{
			BumpupK2hdkcDbgMode();
		}
		mode = GetK2hdkcDbgMode();
		if(DKCDBG_SILENT == mode){
			PRN("Debug level is changed to SILENT.");
		}else if(DKCDBG_ERR == mode){
			PRN("Debug level is changed to ERROR.");
		}else if(DKCDBG_WARN == mode){
			PRN("Debug level is changed to WARNING.");
		}else if(DKCDBG_MSG == mode){
			PRN("Debug level is changed to INFO(MSG).");
		}else{	// DKCDBG_DUMP
			PRN("Debug level is changed to DUMP.");
		}
	}else if(1 < params.size()){
		ERR("Unknown parameter(%s) for dbglevel command.", params[1].c_str());

	}else{
		if(0 == strcasecmp(params[0].c_str(), "silent") || 0 == strcasecmp(params[0].c_str(), "slt")){
			if(DKCDBG_SILENT == mode){
				ERR("Already debugging mode is SILENT.");
			}else{
				if(isModeCAPI){
					k2hdkc_set_debug_level_silent();
				}else{
					SetK2hdkcDbgMode(DKCDBG_SILENT);
				}
				PRN("Debug level is changed to SILENT.");
			}

		}else if(0 == strcasecmp(params[0].c_str(), "error") || 0 == strcasecmp(params[0].c_str(), "err")){
			if(DKCDBG_ERR == mode){
				ERR("Already debugging mode is ERROR.");
			}else{
				if(isModeCAPI){
					k2hdkc_set_debug_level_error();
				}else{
					SetK2hdkcDbgMode(DKCDBG_ERR);
				}
				PRN("Debug level is changed to ERROR.");
			}
		}else if(0 == strcasecmp(params[0].c_str(), "warning") || 0 == strcasecmp(params[0].c_str(), "wan")){
			if(DKCDBG_WARN == mode){
				ERR("Already debugging mode is WARNING.");
			}else{
				if(isModeCAPI){
					k2hdkc_set_debug_level_warning();
				}else{
					SetK2hdkcDbgMode(DKCDBG_WARN);
				}
				PRN("Debug level is changed to WARNING.");
			}
		}else if(0 == strcasecmp(params[0].c_str(), "info") || 0 == strcasecmp(params[0].c_str(), "msg")){
			if(DKCDBG_MSG == mode){
				ERR("Already debugging mode is INFO(MSG).");
			}else{
				if(isModeCAPI){
					k2hdkc_set_debug_level_message();
				}else{
					SetK2hdkcDbgMode(DKCDBG_MSG);
				}
				PRN("Debug level is changed to INFO(MSG).");
			}
		}else if(0 == strcasecmp(params[0].c_str(), "dump") || 0 == strcasecmp(params[0].c_str(), "dmp")){
			if(DKCDBG_DUMP == mode){
				ERR("Already debugging mode is DUMP.");
			}else{
				if(isModeCAPI){
					k2hdkc_set_debug_level_dump();
				}else{
					SetK2hdkcDbgMode(DKCDBG_DUMP);
				}
				PRN("Debug level is changed to DUMP.");
			}
		}else{
			ERR("Unknown parameter(%s) for dbglevel command.", params[0].c_str());
		}
	}
	return true;	// for continue.
}

// [NOTE]
// This declared function is presented by C API.
//
extern bool K2hdkcCvtSubkeysToPack(K2HSubKeys* pSubKeys, PK2HDKCKEYPCK* ppskeypck, int* pskeypckcnt);

// [NOTE]
// For backward reference
//
static bool RawPrintCommand(k2hdkc_chmpx_h chmpxhandle, const unsigned char* pkey, size_t keylength, bool is_all, bool is_noattr, const char* passphrase, bool is_dump, int nestcnt);

//
// Utility for getting subkeys : SubkeyPrintCommand(), SetSubkeyCommand()
//
static bool GetSubkeysCommand(k2hdkc_chmpx_h chmpxhandle, const unsigned char* pkey, size_t keylength, bool is_noattr, PK2HDKCKEYPCK* ppskeypck, int* pskeypckcnt, bool is_result_print = true)
{
	if(!pkey || 0 == keylength || !ppskeypck || !pskeypckcnt){
		ERR("Parameter is wrong.");
		return false;
	}
	*ppskeypck		= NULL;
	*pskeypckcnt	= 0;

	// do command
	bool			result;
	dkcres_type_t	rescode		= DKC_NORESTYPE;
	if(K2HDKC_INVALID_HANDLE == chmpxhandle){
		// use one-time chmpx object
		if(isModeCAPI){
			if(is_noattr){
				// no check attribute type
				result = k2hdkc_full_get_subkeys_np(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pkey, keylength, ppskeypck, pskeypckcnt);
			}else{
				// normal
				result = k2hdkc_full_get_subkeys(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pkey, keylength, ppskeypck, pskeypckcnt);
			}
			rescode = k2hdkc_get_lastres_code();

		}else{
			K2hdkcComGetSubkeys*	pComObj	= GetOtSlaveK2hdkcComGetSubkeys(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin);
			K2HSubKeys*				pSubKeys= NULL;
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return false;
			}
			if(false != (result = pComObj->CommandSend(pkey, keylength, !is_noattr, &pSubKeys, &rescode)) && pSubKeys){
				if(!K2hdkcCvtSubkeysToPack(pSubKeys, ppskeypck, pskeypckcnt)){
					ERR("Could not get subkeys from subkeys class.");
					DKC_DELETE(pSubKeys);
					return false;
				}
				DKC_DELETE(pSubKeys);
			}
			DKC_DELETE(pComObj);
		}

	}else{
		// use permanent chmpx object
		if(isModeCAPI){
			if(is_noattr){
				// no check attribute type
				result = k2hdkc_pm_get_subkeys_np(chmpxhandle, pkey, keylength, ppskeypck, pskeypckcnt);
			}else{
				// normal
				result = k2hdkc_pm_get_subkeys(chmpxhandle, pkey, keylength, ppskeypck, pskeypckcnt);
			}
			rescode = k2hdkc_get_res_code(chmpxhandle);

		}else{
			K2hdkcSlave*			pSlave	= reinterpret_cast<K2hdkcSlave*>(chmpxhandle);
			K2hdkcComGetSubkeys*	pComObj	= GetPmSlaveK2hdkcComGetSubkeys(pSlave);
			K2HSubKeys*				pSubKeys= NULL;
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return false;
			}
			if(false != (result = pComObj->CommandSend(pkey, keylength, !is_noattr, &pSubKeys, &rescode)) && pSubKeys){
				if(!K2hdkcCvtSubkeysToPack(pSubKeys, ppskeypck, pskeypckcnt)){
					ERR("Could not get subkeys from subkeys class.");
					DKC_DELETE(pSubKeys);
					return false;
				}
				DKC_DELETE(pSubKeys);
			}
			DKC_DELETE(pComObj);
		}
	}

	// check result
	if(!result){
		ERR("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
		DKC_FREE_KEYPACK(*ppskeypck, *pskeypckcnt);
		return false;
	}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
		if(is_result_print){
			MSG("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
		}
	}
	return true;
}

//
// Utility for subkeys printing by RawPrintCommand()
//
static bool SubkeyPrintCommand(k2hdkc_chmpx_h chmpxhandle, const unsigned char* pkey, size_t keylength, bool is_all, bool is_noattr, const char* passphrase, bool is_dump, int nestcnt, bool is_result_print = true)
{
	if(!pkey || 0 == keylength){
		ERR("Parameter is wrong.");
		return false;
	}

	// do command(get subkeys)
	PK2HDKCKEYPCK	pskeypck	= NULL;
	int				skeypckcnt	= 0;
	if(!GetSubkeysCommand(chmpxhandle, pkey, keylength, is_noattr, &pskeypck, &skeypckcnt, is_result_print)){
		ERR("Something error occurred during getting subkeys.");
		return false;
	}

	if(is_all){
		// print all subkeys(reentrant key printing)
		for(int cnt = 0; pskeypck && cnt < skeypckcnt; ++cnt){
			if(!RawPrintCommand(chmpxhandle, pskeypck[cnt].pkey, pskeypck[cnt].length, is_all, is_noattr, passphrase, is_dump, nestcnt)){
				WAN("Something wrong occurred during printing subkey, but continue...");
			}
		}
	}else if(0 < skeypckcnt){
		// list all subkeys
		string	strskeys("");
		for(int cnt = 0; pskeypck && cnt < skeypckcnt; ++cnt){
			if(0 < cnt){
				strskeys += ", ";
			}
			char*	pValue = GetPrintableString(pskeypck[cnt].pkey, pskeypck[cnt].length);
			strskeys += "\"";
			strskeys += (DKCEMPTYSTR(pValue) ? "" : pValue);
			strskeys += "\"";
			DKC_FREE(pValue);
		}
		PRN("  %s%s", GetHeadNestString(nestcnt, "subkey: ").c_str(), strskeys.c_str());
	}
	DKC_FREE_KEYPACK(pskeypck, skeypckcnt);

	return true;
}

//
// Common utility function for PrintCommand()
//
static bool RawPrintCommand(k2hdkc_chmpx_h chmpxhandle, const unsigned char* pkey, size_t keylength, bool is_all, bool is_noattr, const char* passphrase, bool is_dump, int nestcnt)
{
	if(!pkey || 0 == keylength){
		ERR("Parameter is wrong.");
		return false;
	}
	if(is_noattr && !DKCEMPTYSTR(passphrase)){
		ERR("Parameter noattrcheck and passphrase are mutually exclusive.");
		return false;
	}

	// do command
	bool					result;
	unsigned char*			pval		= NULL;
	const unsigned char*	pconstval	= NULL;
	size_t					vallength	= 0;
	dkcres_type_t			rescode		= DKC_NORESTYPE;
	if(K2HDKC_INVALID_HANDLE == chmpxhandle){
		// use one-time chmpx object
		if(isModeCAPI){
			if(is_noattr){
				// no check attribute type
				result = k2hdkc_full_get_value_np(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pkey, keylength, &pval, &vallength);
			}else if(!DKCEMPTYSTR(passphrase)){
				// pass phrase
				result = k2hdkc_full_get_value_wp(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pkey, keylength, passphrase, &pval, &vallength);
			}else{
				// normal
				result = k2hdkc_full_get_value(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pkey, keylength, &pval, &vallength);
			}
			rescode		= k2hdkc_get_lastres_code();
			pconstval	= pval;

		}else{
			K2hdkcComGet*	pComObj = GetOtSlaveK2hdkcComGet(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin);
			if(pComObj){
				result = pComObj->CommandSend(pkey, keylength, !is_noattr, passphrase, &pconstval, &vallength, &rescode);
			}else{
				ERR("Something internal error occurred. could not make command object.");
				return false;
			}
			if(pconstval && 0 < vallength){
				pval = k2hbindup(pconstval, vallength);
			}else{
				vallength = 0;
			}
			pconstval = pval;

			DKC_DELETE(pComObj);
		}

	}else{
		// use permanent chmpx object
		if(isModeCAPI){
			if(is_noattr){
				// no check attribute type
				result = k2hdkc_pm_get_value_np(chmpxhandle, pkey, keylength, &pval, &vallength);
			}else if(!DKCEMPTYSTR(passphrase)){
				// pass phrase
				result = k2hdkc_pm_get_value_wp(chmpxhandle, pkey, keylength, passphrase, &pval, &vallength);
			}else{
				// normal
				result = k2hdkc_pm_get_value(chmpxhandle, pkey, keylength, &pval, &vallength);
			}
			rescode		= k2hdkc_get_res_code(chmpxhandle);
			pconstval	= pval;

		}else{
			K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(chmpxhandle);
			K2hdkcComGet*	pComObj= GetPmSlaveK2hdkcComGet(pSlave);
			if(pComObj){
				result = pComObj->CommandSend(pkey, keylength, !is_noattr, passphrase, &pconstval, &vallength, &rescode);
			}else{
				ERR("Something internal error occurred. could not make command object.");
				return false;
			}
			if(pconstval && 0 < vallength){
				pval = k2hbindup(pconstval, vallength);
			}else{
				vallength = 0;
			}
			pconstval = pval;

			DKC_DELETE(pComObj);
		}
	}

	// check result
	if(!result){
		ERR("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
		DKC_FREE(pval);
		return false;
	}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
		MSG("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
	}

	// print result
	char*	pPrnKey = GetPrintableString(pkey, keylength);
	if(pconstval && 0 < vallength){
		if(!is_dump){
			char*	pValue = GetPrintableString(pconstval, vallength);
			PRN("%s\"%s\" => \"%s\"", GetHeadNestString(nestcnt, "+").c_str(), pPrnKey, pValue);
			DKC_FREE(pValue);
		}else{
			BinaryDumpUtility(pPrnKey, pconstval, vallength, nestcnt);
		}
	}else{
		PRN("%s\"%s\" => value is not found", GetHeadNestString(nestcnt, "+").c_str(), pPrnKey);
	}
	DKC_FREE(pval);
	DKC_FREE(pPrnKey);

	// print subkeys(list or dump)
	if(!SubkeyPrintCommand(chmpxhandle, pkey, keylength, is_all, is_noattr, passphrase, is_dump, nestcnt + 1, false)){
		WAN("Something wrong occurred during printing subkey.");
	}
	return true;
}

//
// Command Line: print(p) <key> [noattrcheck] [pass=....]
//
static bool PrintCommand(k2hdkc_chmpx_h chmpxhandle, params_t& params)
{
	// check parameter
	string	strKey		= params[0].c_str();
	bool	is_all		= false;
	bool	is_noattr	= false;
	bool	is_dump		= false;
	string	PassPhrase("");
	for(size_t pos = 1; pos < params.size(); ++pos){
		if(0 == strcasecmp(params[pos].c_str(), "all")){
			is_all = true;
		}else if(0 == strcasecmp(params[pos].c_str(), "noattrcheck")){
			is_noattr = true;
		}else if(0 == strncasecmp(params[pos].c_str(), "PASS=", 5)){
			PassPhrase = params[pos].substr(5);
		}else if(0 == strcasecmp(params[pos].c_str(), "dump")){
			is_dump = true;
		}else{
			ERR("Unknown parameter(%s) for print command.", params[pos].c_str());
			return true;	// for continue.
		}
	}
	if(is_noattr && !PassPhrase.empty()){
		ERR("Parameter noattrcheck and pass are mutually exclusive.");
		return true;		// for continue.
	}

	// do command
	if(!RawPrintCommand(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, is_all, is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), is_dump, 0)){
		ERR("Something error occurred.");
	}
	return true;
}

//
// Common utility function for DirectPrintCommand() and CopyFileCommand()
//
static bool RawDirectGetCommand(k2hdkc_chmpx_h chmpxhandle, const unsigned char* pkey, size_t keylen, off_t offset, size_t length, unsigned char** ppval, size_t& vallen)
{
	if(!pkey || 0 == keylen || offset < 0 || 0 == length || !ppval){
		ERR("Parameters are wrong.");
		return false;
	}
	*ppval	= NULL;
	vallen	= 0;

	// do command
	bool			result;
	dkcres_type_t	rescode = DKC_NORESTYPE;
	if(K2HDKC_INVALID_HANDLE == chmpxhandle){
		// use one-time chmpx object
		if(isModeCAPI){
			result		= k2hdkc_full_da_get_value(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pkey, keylen, offset, length, ppval, &vallen);
			rescode		= k2hdkc_get_lastres_code();

		}else{
			K2hdkcComGetDirect*	pComObj = GetOtSlaveK2hdkcComGetDirect(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return false;
			}
			const unsigned char*	pconstval = NULL;
			if(false != (result = pComObj->CommandSend(pkey, keylen, offset, length, &pconstval, &vallen, &rescode))){
				if(pconstval && 0 < vallen){
					*ppval	= k2hbindup(pconstval, vallen);
				}else{
					*ppval	= NULL;
					vallen	= 0;
				}
			}
			DKC_DELETE(pComObj);
		}

	}else{
		// use permanent chmpx object
		if(isModeCAPI){
			result		= k2hdkc_pm_da_get_value(chmpxhandle, pkey, keylen, offset, length, ppval, &vallen);
			rescode		= k2hdkc_get_res_code(chmpxhandle);

		}else{
			K2hdkcSlave*		pSlave = reinterpret_cast<K2hdkcSlave*>(chmpxhandle);
			K2hdkcComGetDirect*	pComObj= GetPmSlaveK2hdkcComGetDirect(pSlave);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return false;
			}
			const unsigned char*	pconstval = NULL;
			if(false != (result = pComObj->CommandSend(pkey, keylen, offset, length, &pconstval, &vallen, &rescode))){
				if(pconstval && 0 < vallen){
					*ppval	= k2hbindup(pconstval, vallen);
				}else{
					*ppval	= NULL;
					vallen	= 0;
				}
			}
			DKC_DELETE(pComObj);
		}
	}

	// check result
	if(!result){
		ERR("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
		DKC_FREE(*ppval);
		return false;
	}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
		MSG("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
	}

	return true;
}

//
// Command Line: directprint(dp) <key> <length> <offset>
//
static bool DirectPrintCommand(k2hdkc_chmpx_h chmpxhandle, params_t& params)
{
	// check parameter
	string	strKey		= params[0].c_str();
	size_t	length		= static_cast<size_t>(atoi(params[1].c_str()));
	off_t	offset		= static_cast<off_t>(atoi(params[2].c_str()));
	bool	is_dump		= false;
	for(size_t pos = 3; pos < params.size(); ++pos){
		if(0 == strcasecmp(params[pos].c_str(), "dump")){
			is_dump = true;
		}else{
			ERR("Unknown parameter(%s) for directprint(dp) command.", params[pos].c_str());
			return true;	// for continue.
		}
	}
	if(MAX_ONE_VALUE_LENGTH < length){
		ERR("length parameter(%s) for directprint(dp) command is over maximum length(%d).", params[1].c_str(), MAX_ONE_VALUE_LENGTH);
		return true;		// for continue.
	}
	if(offset < 0){
		ERR("offset parameter(%s) for directprint(dp) command must be positive number.", params[2].c_str());
		return true;		// for continue.
	}

	unsigned char*	pval		= NULL;
	size_t			vallength	= 0;
	if(!RawDirectGetCommand(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, offset, length, &pval, vallength)){
		return true;		// for continue.
	}

	// print result
	if(pval){
		if(!is_dump){
			char*	pValue = GetPrintableString(pval, vallength);
			PRN("\"%s (offset:%zd byte, request %zu byte)\" => \"%s\"", strKey.c_str(), offset, length, pValue);
			DKC_FREE(pValue);

		}else{
			string	prefix	=  strKey;
			prefix			+= " (offset:";
			prefix			+= to_string(offset);
			prefix			+= " byte, request ";
			prefix			+= to_string(length);
			prefix			+= " byte)";
			BinaryDumpUtility(prefix.c_str(), pval, vallength, 0);
		}
	}else{
		PRN("\"%s\" => value is not found", strKey.c_str());
	}
	DKC_FREE(pval);

	return true;
}


// [NOTE]
// This declared function is presented by C API.
//
extern bool K2hdkcCvtAttrsToPack(K2HAttrs* pAttrs, PK2HDKCATTRPCK* ppattrspck, int* pattrspckcnt);

//
// Command Line: printattr(pa) <key> [name=<attr name>] [dump]
//
static bool PrintAttrCommand(k2hdkc_chmpx_h chmpxhandle, params_t& params)
{
	// check parameter
	string	strKey	= params[0].c_str();
	bool	is_dump	= false;
	string	strAttr("");
	for(size_t pos = 1; pos < params.size(); ++pos){
		if(0 == strcasecmp(params[pos].c_str(), "dump")){
			is_dump = true;
		}else if(0 == strncasecmp(params[pos].c_str(), "NAME=", 5)){
			strAttr = params[pos].substr(5);
		}else{
			ERR("Unknown parameter(%s) for print attributes command.", params[pos].c_str());
			return true;	// for continue.
		}
	}

	// do command
	bool			result;
	dkcres_type_t	rescode		= DKC_NORESTYPE;
	PK2HDKCATTRPCK	pattrspck	= NULL;
	int				attrspckcnt	= 0;
	if(K2HDKC_INVALID_HANDLE == chmpxhandle){
		// use one-time chmpx object
		if(isModeCAPI){
			result = k2hdkc_full_get_attrs(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, &pattrspck, &attrspckcnt);
			rescode= k2hdkc_get_lastres_code();

		}else{
			K2hdkcComGetAttrs*	pComObj		= GetOtSlaveK2hdkcComGetAttrs(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin);
			K2HAttrs*			pAttrsObj	= NULL;
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return false;
			}
			result = pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, &pAttrsObj, &rescode);
			DKC_DELETE(pComObj);

			if(pAttrsObj && !K2hdkcCvtAttrsToPack(pAttrsObj, &pattrspck, &attrspckcnt)){
				ERR("Something internal error occurred. could not convert attributes data.");
				DKC_DELETE(pAttrsObj);
				return false;
			}
			DKC_DELETE(pAttrsObj);
		}

	}else{
		// use permanent chmpx object
		if(isModeCAPI){
			result = k2hdkc_pm_get_attrs(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, &pattrspck, &attrspckcnt);
			rescode= k2hdkc_get_res_code(chmpxhandle);

		}else{
			K2hdkcSlave*		pSlave		= reinterpret_cast<K2hdkcSlave*>(chmpxhandle);
			K2hdkcComGetAttrs*	pComObj		= GetPmSlaveK2hdkcComGetAttrs(pSlave);
			K2HAttrs*			pAttrsObj	= NULL;
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return false;
			}
			result = pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, &pAttrsObj, &rescode);
			DKC_DELETE(pComObj);

			if(pAttrsObj && !K2hdkcCvtAttrsToPack(pAttrsObj, &pattrspck, &attrspckcnt)){
				ERR("Something internal error occurred. could not convert attributes data.");
				DKC_DELETE(pAttrsObj);
				return false;
			}
			DKC_DELETE(pAttrsObj);
		}
	}

	// check result
	if(!result){
		ERR("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
		DKC_FREE_ATTRPACK(pattrspck, attrspckcnt);
		return false;
	}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
		MSG("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
	}

	// print result
	if(pattrspck && 0 < attrspckcnt){
		PRN("KEY => \"%s\"", strKey.c_str());

		bool	is_print = false;
		for(int cnt = 0; cnt < attrspckcnt; ++cnt){
			if(strAttr.empty() || ((pattrspck[cnt].keylength == (strAttr.length() + 1)) && 0 == memcmp(pattrspck[cnt].pkey, reinterpret_cast<const unsigned char*>(strAttr.c_str()), pattrspck[cnt].keylength))){
				if(!is_dump){
					char*	pAttrName	= GetPrintableString(pattrspck[cnt].pkey, pattrspck[cnt].keylength);
					char*	pAttrValue	= GetPrintableString(pattrspck[cnt].pval, pattrspck[cnt].vallength);
					PRN("  +\"%s\" => \"%s\"", (pAttrName ? pAttrName : ""), (pAttrValue ? pAttrValue : ""));
					DKC_FREE(pAttrName);
					DKC_FREE(pAttrValue);
				}else{
					BinaryDumpUtility("+ NAME =", pattrspck[cnt].pkey, pattrspck[cnt].keylength, 1);
					BinaryDumpUtility("  VALUE=", pattrspck[cnt].pval, pattrspck[cnt].vallength, 1);
				}
				is_print = true;
			}
		}
		if(!is_print){
			PRN("  + attribute is not found");
		}
	}else{
		PRN("KEY => \"%s\" (attribute is not found)", strKey.c_str());
	}
	DKC_FREE_ATTRPACK(pattrspck, attrspckcnt);

	return true;
}

//
// Command Line: copyf(cf) <key> <length> <offset> <file>
//
static bool CopyFileCommand(k2hdkc_chmpx_h chmpxhandle, params_t& params)
{
	// check parameter
	string	strKey		= params[0].c_str();
	size_t	length		= static_cast<size_t>(atoi(params[1].c_str()));
	off_t	offset		= static_cast<off_t>(atoi(params[2].c_str()));
	string	strFile		= params[3].c_str();
	if(offset < 0){
		ERR("offset parameter(%s) for copyfile(cf) command must be positive number.", params[2].c_str());
		return true;		// for continue.
	}

	// loop
	for(size_t totalvallen = 0L, vallength = 0L; totalvallen < length; totalvallen += vallength){
		size_t			reqsize;
		off_t			reqoffset;
		unsigned char*	pval;

		// get value
		reqsize		= min(static_cast<size_t>(MAX_ONE_VALUE_LENGTH), (length - totalvallen));
		reqoffset	= offset + totalvallen;
		pval		= NULL;
		vallength	= 0;
		if(!RawDirectGetCommand(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, reqoffset, reqsize, &pval, vallength)){
			return true;		// for continue.
		}

		// write to file
		if(!AppendContentsToFile(strFile.c_str(), pval, vallength)){
			ERR("Something error occurred to write to file(%s).", strFile.c_str());
			DKC_FREE(pval);
			return true;		// for continue.
		}
		DKC_FREE(pval);

		if(vallength < reqsize){
			// reach end of value
			break;
		}
	}
	return true;
}

// [NOTE]
// For backward reference
//
static bool RemoveCommand(k2hdkc_chmpx_h chmpxhandle, const unsigned char* pkey, size_t keylen, bool is_subkeys);

//
// Common utility function for SetCommand() and FillCommand()
//
static bool RawSetCommand(k2hdkc_chmpx_h chmpxhandle, const unsigned char* pKey, size_t KeyLen, const unsigned char* pVal, size_t ValLen, bool is_rmsub, bool is_rmsublist, bool is_noattr, const char* pPassPhrase, const time_t* pExpire)
{
	if(!pKey || 0 == KeyLen){
		ERR("Parameter is wrong.");
		return false;
	}

	// [0] Get subkeys list
	PK2HDKCKEYPCK	pskeypck	= NULL;
	int				skeypckcnt	= 0;
	if(is_rmsub){
		if(!GetSubkeysCommand(chmpxhandle, pKey, KeyLen, is_noattr, &pskeypck, &skeypckcnt)){
			ERR("Something error occurred during getting subkeys.");
			return false;
		}
	}

	// [1] do command
	bool			result;
	dkcres_type_t	rescode		= DKC_NORESTYPE;
	if(K2HDKC_INVALID_HANDLE == chmpxhandle){
		// use one-time chmpx object
		if(isModeCAPI){
			if(!pExpire && !pPassPhrase && !is_rmsublist){
				// normal
				result = k2hdkc_full_set_value(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pKey, KeyLen, pVal, ValLen);
			}else{
				// with attributes
				result = k2hdkc_full_set_value_wa(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pKey, KeyLen, pVal, ValLen, is_rmsublist, pPassPhrase, pExpire);
			}
			rescode = k2hdkc_get_lastres_code();

		}else{
			K2hdkcComSet*	pComObj = GetOtSlaveK2hdkcComSet(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin);
			if(pComObj){
				result = pComObj->CommandSend(pKey, KeyLen, pVal, ValLen, is_rmsublist, pPassPhrase, pExpire, &rescode);
			}else{
				ERR("Something internal error occurred. could not make command object.");
				DKC_FREE_KEYPACK(pskeypck, skeypckcnt);
				return false;
			}
			DKC_DELETE(pComObj);
		}

	}else{
		// use permanent chmpx object
		if(isModeCAPI){
			if(!pExpire && !pPassPhrase && !is_rmsublist){
				// normal
				result = k2hdkc_pm_set_value(chmpxhandle, pKey, KeyLen, pVal, ValLen);
			}else{
				// with attributes
				result = k2hdkc_pm_set_value_wa(chmpxhandle, pKey, KeyLen, pVal, ValLen, is_rmsublist, pPassPhrase, pExpire);
			}
			rescode = k2hdkc_get_res_code(chmpxhandle);

		}else{
			K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(chmpxhandle);
			K2hdkcComSet*	pComObj = GetPmSlaveK2hdkcComSet(pSlave);
			if(pComObj){
				result = pComObj->CommandSend(pKey, KeyLen, pVal, ValLen, is_rmsublist, pPassPhrase, pExpire, &rescode);
			}else{
				ERR("Something internal error occurred. could not make command object.");
				DKC_FREE_KEYPACK(pskeypck, skeypckcnt);
				return false;
			}
			DKC_DELETE(pComObj);
		}
	}
	// check result
	if(!result){
		ERR("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
		DKC_FREE_KEYPACK(pskeypck, skeypckcnt);
		return false;
	}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
		MSG("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
	}

	// [2] Remove all subkeys
	if(is_rmsub){
		for(int cnt = 0; pskeypck && cnt < skeypckcnt; ++cnt){
			if(!RemoveCommand(chmpxhandle, pskeypck[cnt].pkey, pskeypck[cnt].length, is_rmsub)){
				WAN("Something wrong occurred during removing subkey, but continue...");
			}
		}
	}
	DKC_FREE_KEYPACK(pskeypck, skeypckcnt);

	return true;
}

//
// Command Line: set(s) <key> <value> [rmsublist | rmsub] [noattrcheck] [pass=....] [expire=sec]
//
static bool SetCommand(k2hdkc_chmpx_h chmpxhandle, params_t& params)
{
	// check parameter
	string	strKey		= params[0].c_str();
	string	strValue	= params[1].c_str();
	bool	is_noattr	= false;
	bool	is_rmsublist= false;
	bool	is_rmsub	= false;
	int		rmsub_pcnt	= 0;
	time_t	Expire		= -1;
	string	PassPhrase("");
	for(size_t pos = 2; pos < params.size(); ++pos){
		if(0 == strcasecmp(params[pos].c_str(), "noattrcheck")){
			is_noattr = true;
		}else if(0 == strcasecmp(params[pos].c_str(), "rmsub")){
			is_rmsub	= true;
			is_rmsublist= true;
			rmsub_pcnt++;
		}else if(0 == strcasecmp(params[pos].c_str(), "rmsublist")){
			is_rmsub	= false;
			is_rmsublist= true;
			rmsub_pcnt++;
		}else if(0 == strncasecmp(params[pos].c_str(), "PASS=", 5)){
			PassPhrase = params[pos].substr(5);
		}else if(0 == strncasecmp(params[pos].c_str(), "EXPIRE=", 7)){
			Expire = static_cast<time_t>(atoi(params[pos].substr(7).c_str()));
		}else{
			ERR("Unknown parameter(%s) for set command.", params[pos].c_str());
			return true;	// for continue.
		}
	}
	if(1 < rmsub_pcnt){
		ERR("Parameter \"rmsub\" and \"rmsublist\" not be specified together.");
		return true;		// for continue.
	}
	if(strValue == "null"){
		strValue = "";
	}

	// do command
	if(!RawSetCommand(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, (strValue.empty() ? NULL : reinterpret_cast<const unsigned char*>(strValue.c_str())), (strValue.empty() ? 0 : (strValue.length() + 1)), is_rmsub, is_rmsublist, is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire))){
		ERR("Something internal error occurred during setting key and value.");
		return true;		// for continue.
	}

	return true;
}

//
// Common utility function for DirectSetCommand() and SetFileCommand()
//
static bool RawDirectSetCommand(k2hdkc_chmpx_h chmpxhandle, const unsigned char* pkey, size_t keylen, const unsigned char* pval, size_t vallen, off_t offset)
{
	if(!pkey || 0 == keylen || !pval || 0 == vallen || offset < 0){
		ERR("parameters are wrong.");
		return false;
	}

	// do command
	bool			result;
	dkcres_type_t	rescode		= DKC_NORESTYPE;
	if(K2HDKC_INVALID_HANDLE == chmpxhandle){
		// use one-time chmpx object
		if(isModeCAPI){
			result	= k2hdkc_full_da_set_value(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pkey, keylen, pval, vallen, offset);
			rescode	= k2hdkc_get_lastres_code();

		}else{
			K2hdkcComSetDirect*	pComObj = GetOtSlaveK2hdkcComSetDirect(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin);
			if(pComObj){
				result = pComObj->CommandSend(pkey, keylen, pval, vallen, offset, &rescode);
			}else{
				ERR("Something internal error occurred. could not make command object.");
				return true;		// for continue.
			}
			DKC_DELETE(pComObj);
		}

	}else{
		// use permanent chmpx object
		if(isModeCAPI){
			result	= k2hdkc_pm_da_set_value(chmpxhandle, pkey, keylen, pval, vallen, offset);
			rescode	= k2hdkc_get_res_code(chmpxhandle);

		}else{
			K2hdkcSlave*		pSlave	= reinterpret_cast<K2hdkcSlave*>(chmpxhandle);
			K2hdkcComSetDirect*	pComObj	= GetPmSlaveK2hdkcComSetDirect(pSlave);
			if(pComObj){
				result = pComObj->CommandSend(pkey, keylen, pval, vallen, offset, &rescode);
			}else{
				ERR("Something internal error occurred. could not make command object.");
				return true;		// for continue.
			}
			DKC_DELETE(pComObj);
		}
	}

	// check result
	if(!result){
		ERR("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
		return true;		// for continue.
	}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
		MSG("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
	}

	return true;
}

//
// Command Line: directset(dset) <key> <value> <offset>
//
static bool DirectSetCommand(k2hdkc_chmpx_h chmpxhandle, params_t& params)
{
	// check parameter
	string	strKey		= params[0].c_str();
	string	strValue	= params[1].c_str();
	off_t	offset		= static_cast<off_t>(atoi(params[2].c_str()));
	if(offset < 0){
		ERR("offset parameter(%s) for directset(dset) command must be positive number.", params[2].c_str());
		return true;	// for continue.
	}

	// do command
	if(!RawDirectSetCommand(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, (strValue.empty() ? NULL : reinterpret_cast<const unsigned char*>(strValue.c_str())), (strValue.empty() ? 0 : (strValue.length() + 1)), offset)){
		ERR("Failed to set direct value.");
		return true;		// for continue.
	}
	return true;
}

//
// Command Line: setf(sf) <key> <offset> <file>
//
static bool SetFileCommand(k2hdkc_chmpx_h chmpxhandle, params_t& params)
{
	// check parameter
	string	strKey	= params[0].c_str();
	off_t	offset	= static_cast<off_t>(atoi(params[1].c_str()));
	string	strFile	= params[2].c_str();
	if(offset < 0){
		ERR("offset parameter(%s) for directset(dset) command must be positive number.", params[1].c_str());
		return true;	// for continue.
	}
	size_t			flen	= 0UL;
	if(!GetFileSize(strFile.c_str(), flen)){
		ERR("Could not file(%s) size for set file command.", strFile.c_str(), params[1].c_str());
		return true;	// for continue.
	}

	// loop
	for(size_t totallen = 0L, vallen = 0L; totallen < flen; totallen += vallen){
		unsigned char*	pval;
		off_t			reqoffset;
		size_t			reqsize;

		// read
		reqsize		= min(static_cast<size_t>(10), (flen - totallen));
		reqoffset	= static_cast<off_t>(totallen);
		pval		= NULL;
		vallen		= 0;
		if(!ReadContentsFromFile(strFile.c_str(), reqoffset, reqsize, &pval, vallen)){
			ERR("Something error occurred during reading value from file(%s).", strFile.c_str());
			return true;	// for continue.
		}

		// do command
		if(!RawDirectSetCommand(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, pval, vallen, (offset + reqoffset))){
			ERR("Failed to set value from file.");
			DKC_FREE(pval);
			return true;		// for continue.
		}
		DKC_FREE(pval);

		if(vallen < reqsize){
			break;
		}
	}
	return true;
}

//
// Command Line: fill(f) <prefix> <value> <count> [rmsublist | rmsub] [noattrcheck] [pass=....] [expire=sec]
//
static bool FillCommand(k2hdkc_chmpx_h chmpxhandle, params_t& params)
{
	// check parameter
	string	strKey		= params[0].c_str();
	string	strValue	= params[1].c_str();
	int		FillCount	= atoi(params[2].c_str());
	bool	is_noattr	= false;
	bool	is_rmsublist= false;
	bool	is_rmsub	= false;
	int		rmsub_pcnt	= 0;
	time_t	Expire		= -1;
	string	PassPhrase("");
	for(size_t pos = 3; pos < params.size(); ++pos){
		if(0 == strcasecmp(params[pos].c_str(), "noattrcheck")){
			is_noattr = true;
		}else if(0 == strcasecmp(params[pos].c_str(), "rmsub")){
			is_rmsub	= true;
			is_rmsublist= true;
			rmsub_pcnt++;
		}else if(0 == strcasecmp(params[pos].c_str(), "rmsublist")){
			is_rmsub	= false;
			is_rmsublist= true;
			rmsub_pcnt++;
		}else if(0 == strncasecmp(params[pos].c_str(), "PASS=", 5)){
			PassPhrase = params[pos].substr(5);
		}else if(0 == strncasecmp(params[pos].c_str(), "EXPIRE=", 7)){
			Expire = static_cast<time_t>(atoi(params[pos].substr(7).c_str()));
		}else{
			ERR("Unknown parameter(%s) for fill command.", params[pos].c_str());
			return true;	// for continue.
		}
	}
	if(FillCount <= 0){
		ERR("Parameter \"count\" must be positive value.");
		return true;		// for continue.
	}
	if(1 < rmsub_pcnt){
		ERR("Parameter \"rmsub\" and \"rmsublist\" not be specified together.");
		return true;		// for continue.
	}
	if(strValue == "null"){
		strValue = "";
	}

	// do command
	string	strFillKey;
	for(int cnt = 0; cnt < FillCount; ++cnt){
		strFillKey = strKey + string("-") + to_string(cnt);
		if(!RawSetCommand(chmpxhandle, reinterpret_cast<const unsigned char*>(strFillKey.c_str()), strFillKey.length() + 1, (strValue.empty() ? NULL : reinterpret_cast<const unsigned char*>(strValue.c_str())), (strValue.empty() ? 0 : (strValue.length() + 1)), is_rmsub, is_rmsublist, is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire))){
			ERR("Something internal error occurred during setting key and value.");
			return true;		// for continue.
		}
	}
	return true;
}

// [NOTE]
// This declared function is presented by C API.
//
static bool RawSetSubkeyCommand(k2hdkc_chmpx_h chmpxhandle, const unsigned char* pParentKey, size_t ParentKeyLen, const unsigned char* pSubKey, size_t SubKeyLen, const unsigned char* pVal, size_t ValLen, bool is_noattr, const char* pPassPhrase, const time_t* pExpire);

//
// Command Line: fillsub(fs) <parent key> <prefix> <value> <count> [noattrcheck] [pass=....] [expire=sec]
//
static bool FillSubCommand(k2hdkc_chmpx_h chmpxhandle, params_t& params)
{
	// check parameter
	string	strParentKey= params[0].c_str();
	string	strSubKey	= params[1].c_str();
	string	strValue	= params[2].c_str();
	int		FillCount	= atoi(params[3].c_str());
	bool	is_noattr	= false;
	time_t	Expire		= -1;
	string	PassPhrase("");
	for(size_t pos = 4; pos < params.size(); ++pos){
		if(0 == strcasecmp(params[pos].c_str(), "noattrcheck")){
			is_noattr = true;
		}else if(0 == strncasecmp(params[pos].c_str(), "PASS=", 5)){
			PassPhrase = params[pos].substr(5);
		}else if(0 == strncasecmp(params[pos].c_str(), "EXPIRE=", 7)){
			Expire = static_cast<time_t>(atoi(params[pos].substr(7).c_str()));
		}else{
			ERR("Unknown parameter(%s) for fill subkey command.", params[pos].c_str());
			return true;	// for continue.
		}
	}
	if(FillCount <= 0){
		ERR("Parameter \"count\" must be positive value.");
		return true;		// for continue.
	}
	if(strValue == "null"){
		strValue = "";
	}

	// do command
	string	strFillKey;
	for(int cnt = 0; cnt < FillCount; ++cnt){
		strFillKey = strSubKey + string("-") + to_string(cnt);
		if(!RawSetSubkeyCommand(chmpxhandle, reinterpret_cast<const unsigned char*>(strParentKey.c_str()), strParentKey.length() + 1, reinterpret_cast<const unsigned char*>(strFillKey.c_str()), strFillKey.length() + 1, (strValue.empty() ? NULL : reinterpret_cast<const unsigned char*>(strValue.c_str())), (strValue.empty() ? 0 : (strValue.length() + 1)), is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire))){
			ERR("Something internal error occurred during setting subkey and value into parent key.");
			return true;		// for continue.
		}
	}
	return true;
}

//
// Common utility function for RemoveCommand() and SetCommand() and RemoveSubkeyCommand()
//
static bool RemoveCommand(k2hdkc_chmpx_h chmpxhandle, const unsigned char* pkey, size_t keylen, bool is_subkeys)
{
	if(!pkey || 0 == keylen){
		ERR("parameters are wrong.");
		return false;
	}

	// do command
	bool			result;
	dkcres_type_t	rescode		= DKC_NORESTYPE;
	if(K2HDKC_INVALID_HANDLE == chmpxhandle){
		// use one-time chmpx object
		if(isModeCAPI){
			if(is_subkeys){
				// with subkeys
				result = k2hdkc_full_remove_all(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pkey, keylen);
			}else{
				// without subkeys
				result = k2hdkc_full_remove(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pkey, keylen);
			}
			rescode = k2hdkc_get_lastres_code();

		}else{
			K2hdkcComDel*	pComObj = GetOtSlaveK2hdkcComDel(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin);
			if(pComObj){
				result = pComObj->CommandSend(pkey, keylen, is_subkeys, true, &rescode);
			}else{
				ERR("Something internal error occurred. could not make command object.");
				return false;
			}
			DKC_DELETE(pComObj);
		}

	}else{
		// use permanent chmpx object
		if(isModeCAPI){
			if(is_subkeys){
				// with subkeys
				result = k2hdkc_pm_remove_all(chmpxhandle, pkey, keylen);
			}else{
				// without subkeys
				result = k2hdkc_pm_remove(chmpxhandle, pkey, keylen);
			}
			rescode = k2hdkc_get_res_code(chmpxhandle);

		}else{
			K2hdkcSlave*	pSlave = reinterpret_cast<K2hdkcSlave*>(chmpxhandle);
			K2hdkcComDel*	pComObj = GetPmSlaveK2hdkcComDel(pSlave);
			if(pComObj){
				result = pComObj->CommandSend(pkey, keylen, is_subkeys, true, &rescode);
			}else{
				ERR("Something internal error occurred. could not make command object.");
				return false;
			}
			DKC_DELETE(pComObj);
		}
	}

	// check result
	if(!result){
		ERR("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
		return false;
	}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
		MSG("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
	}

	return true;
}

//
// Command Line: del(rm) <key> [all]
//
static bool RemoveCommand(k2hdkc_chmpx_h chmpxhandle, params_t& params)
{
	// check parameter
	string	strKey		= params[0].c_str();
	bool	is_subkeys	= false;
	for(size_t pos = 1; pos < params.size(); ++pos){
		if(0 == strcasecmp(params[pos].c_str(), "all")){
			is_subkeys = true;
		}else{
			ERR("Unknown parameter(%s) for del(rm) command.", params[pos].c_str());
			return true;	// for continue.
		}
	}

	// do command
	if(!RemoveCommand(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, is_subkeys)){
		ERR("Something internal error occurred during removing key.");
		return true;		// for continue.
	}

	return true;
}

//
// Common utility function for SetSubkeyCommand()
//
bool AddSubkeyIntoSubkeysPack(const unsigned char* psubkey, size_t subkeylen, const PK2HDKCKEYPCK psrcskeypck, int srcskeypckcnt, PK2HDKCKEYPCK *ppdstskeypck, int* pdstskeypckcnt)
{
	if(!psubkey || 0 == subkeylen || (!psrcskeypck && 0 < srcskeypckcnt) || (psrcskeypck && 0 == srcskeypckcnt) || !ppdstskeypck || !pdstskeypckcnt){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}

	*pdstskeypckcnt = srcskeypckcnt + 1;
	if(NULL == ((*ppdstskeypck) = reinterpret_cast<PK2HDKCKEYPCK>(calloc(*pdstskeypckcnt, sizeof(K2HDKCKEYPCK))))){
		ERR_DKCPRN("Could not allocate memory.");
		return false;
	}

	// copy
	for(int cnt = 0; cnt < srcskeypckcnt; ++cnt){
		(*ppdstskeypck)[cnt].pkey	= k2hbindup(psrcskeypck[cnt].pkey, psrcskeypck[cnt].length);
		(*ppdstskeypck)[cnt].length	= psrcskeypck[cnt].length;
	}
	// add
	(*ppdstskeypck)[srcskeypckcnt].pkey		= k2hbindup(psubkey, subkeylen);
	(*ppdstskeypck)[srcskeypckcnt].length	= subkeylen;

	return true;
}

//
// Common utility function for RemoveSubkeyCommand()
//
bool RemoveSubkeyFromSubkeysPack(const unsigned char* psubkey, size_t subkeylen, const PK2HDKCKEYPCK psrcskeypck, int srcskeypckcnt, PK2HDKCKEYPCK *ppdstskeypck, int* pdstskeypckcnt)
{
	if(!psubkey || 0 == subkeylen || !psrcskeypck || !ppdstskeypck || !pdstskeypckcnt){
		ERR_DKCPRN("Parameters are wrong.");
		return false;
	}

	// check key is in list
	bool	found = false;
	for(int cnt = 0; cnt < srcskeypckcnt; ++cnt){
		if(psrcskeypck[cnt].length == subkeylen && 0 == memcmp(psrcskeypck[cnt].pkey, psubkey, subkeylen)){
			found = true;
			break;
		}
	}
	if(!found){
		WAN("The subkey is not in subkey list.");
		return false;
	}

	// allocate
	*pdstskeypckcnt = srcskeypckcnt - 1;
	if(*pdstskeypckcnt <= 0){
		*ppdstskeypck	= NULL;
		*pdstskeypckcnt	= 0;
		return true;
	}
	if(NULL == ((*ppdstskeypck) = reinterpret_cast<PK2HDKCKEYPCK>(calloc(*pdstskeypckcnt, sizeof(K2HDKCKEYPCK))))){
		ERR_DKCPRN("Could not allocate memory.");
		return false;
	}

	// copy
	for(int cnt = 0, setpos = 0; cnt < srcskeypckcnt; ++cnt){
		if(psrcskeypck[cnt].length != subkeylen || 0 != memcmp(psrcskeypck[cnt].pkey, psubkey, subkeylen)){
			(*ppdstskeypck)[setpos].pkey	= k2hbindup(psrcskeypck[cnt].pkey, psrcskeypck[cnt].length);
			(*ppdstskeypck)[setpos].length	= psrcskeypck[cnt].length;
			++setpos;
		}
	}
	return true;
}

//
// Common utility function for SetSubkeyCommand() and FillSubCommand()
//
static bool RawSetSubkeyCommand(k2hdkc_chmpx_h chmpxhandle, const unsigned char* pParentKey, size_t ParentKeyLen, const unsigned char* pSubKey, size_t SubKeyLen, const unsigned char* pVal, size_t ValLen, bool is_noattr, const char* pPassPhrase, const time_t* pExpire)
{
	if(!pParentKey || 0 == ParentKeyLen || !pSubKey || 0 == SubKeyLen){
		ERR("Parameters are wrong.");
		return false;
	}

	// do command
	bool			result;
	dkcres_type_t	rescode		= DKC_NORESTYPE;
	if(K2HDKC_INVALID_HANDLE == chmpxhandle){
		// use one-time chmpx object
		if(isModeCAPI){
			// add subkey with value and set into parent key
			if(!pExpire && pPassPhrase){
				// without attributes
				result	= k2hdkc_full_set_subkey(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pParentKey, ParentKeyLen, pSubKey, SubKeyLen, pVal, ValLen);
			}else{
				// with attributes
				result	= k2hdkc_full_set_subkey_wa(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pParentKey, ParentKeyLen, pSubKey, SubKeyLen, pVal, ValLen, !is_noattr, pPassPhrase, pExpire);
			}
			rescode = k2hdkc_get_lastres_code();

		}else{
			// add subkey with value and set into parent key
			K2hdkcComAddSubkey*	pComObj = GetOtSlaveK2hdkcComAddSubkey(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return false;
			}
			result = pComObj->CommandSend(pParentKey, ParentKeyLen, pSubKey, SubKeyLen, pVal, ValLen, !is_noattr, pPassPhrase, pExpire, &rescode);

			DKC_DELETE(pComObj);
		}
	}else{
		// use permanent chmpx object
		if(isModeCAPI){
			// add subkey with value and set into parent key
			if(!pExpire && pPassPhrase){
				// without attributes
				result	= k2hdkc_pm_set_subkey(chmpxhandle, pParentKey, ParentKeyLen, pSubKey, SubKeyLen, pVal, ValLen);
			}else{
				// with attributes
				result	= k2hdkc_pm_set_subkey_wa(chmpxhandle, pParentKey, ParentKeyLen, pSubKey, SubKeyLen, pVal, ValLen, !is_noattr, pPassPhrase, pExpire);
			}
			rescode	= k2hdkc_get_res_code(chmpxhandle);

		}else{
			// add subkey with value and set into parent key
			K2hdkcSlave*		pSlave	= reinterpret_cast<K2hdkcSlave*>(chmpxhandle);
			K2hdkcComAddSubkey*	pComObj	= GetPmSlaveK2hdkcComAddSubkey(pSlave);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return false;
			}
			result = pComObj->CommandSend(pParentKey, ParentKeyLen, pSubKey, SubKeyLen, pVal, ValLen, !is_noattr, pPassPhrase, pExpire, &rescode);

			DKC_DELETE(pComObj);
		}
	}

	// check result
	if(!result){
		ERR("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
		return false;
	}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
		MSG("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
	}

	return true;
}

// [NOTE]
// This declared function is presented by C API.
//
extern bool K2hdkcCvtPackToSubkeys(const PK2HDKCKEYPCK pskeypck, int skeypckcnt, unsigned char** ppsubkeys, size_t* psubkeyslength);

//
// Command Line: setsub(ss) <parent key> <key> [noattrcheck]                                 
//				 setsub(ss) <parent key> <key> <value> [noattrcheck] [pass=....] [expire=sec]
//
static bool SetSubkeyCommand(k2hdkc_chmpx_h chmpxhandle, params_t& params)
{
	// check parameter
	string	strParentKey= params[0].c_str();
	string	strKey		= params[1].c_str();
	bool	is_noattr	= false;
	time_t	Expire		= -1;
	string	strValue("");
	string	PassPhrase("");
	for(size_t pos = 2; pos < params.size(); ++pos){
		if(0 == strncasecmp(params[pos].c_str(), "PASS=", 5)){
			PassPhrase = params[pos].substr(5);
		}else if(0 == strncasecmp(params[pos].c_str(), "EXPIRE=", 7)){
			Expire = static_cast<time_t>(atoi(params[pos].substr(7).c_str()));
		}else if(0 == strcasecmp(params[pos].c_str(), "noattrcheck")){
			is_noattr = true;
		}else{
			if(!strValue.empty()){
				ERR("Unknown parameter(%s) for set subkey command.", params[pos].c_str());
				return true;	// for continue.
			}
			strValue = params[pos].c_str();
		}
	}
	bool	is_only_key	= strValue.empty();
	if(strValue == "null"){
		strValue = "";
	}
	if(is_only_key && (!PassPhrase.empty() || -1 != Expire)){
		ERR("Passphrase or expire parameter is specified without value parameter.");
		return true;			// for continue.
	}

	// For simplify and facilitate visualization.
	const unsigned char*	pPKey		= reinterpret_cast<const unsigned char*>(strParentKey.c_str());
	size_t					PKeyLen		= strParentKey.length() + 1;
	const unsigned char*	pSKey		= reinterpret_cast<const unsigned char*>(strKey.c_str());
	size_t					SkeyLen		= strKey.length() + 1;
	const unsigned char*	pVal		= strValue.empty() ? NULL : reinterpret_cast<const unsigned char*>(strValue.c_str());
	size_t					ValLen		= strValue.empty() ? 0 : (strValue.length() + 1);

	// do command
	if(is_only_key){
		// only adding subkey into parent key
		bool			result;
		dkcres_type_t	rescode	= DKC_NORESTYPE;

		if(K2HDKC_INVALID_HANDLE == chmpxhandle){
			// use one-time chmpx object
			if(isModeCAPI){
				// get subkeys
				PK2HDKCKEYPCK	psrcskeypck		= NULL;
				int				srcskeypckcnt	= 0;
				if(!GetSubkeysCommand(chmpxhandle, pPKey, PKeyLen, is_noattr, &psrcskeypck, &srcskeypckcnt)){
					ERR("Something error occurred during getting subkeys.");
					return true;		// for continue.
				}
				// add new subkey
				PK2HDKCKEYPCK	pskeypck	= NULL;
				int				skeypckcnt	= 0;
				if(!AddSubkeyIntoSubkeysPack(pSKey, SkeyLen, psrcskeypck, srcskeypckcnt, &pskeypck, &skeypckcnt)){
					ERR("Something error occurred adding new subkey into subkeys array.");
					DKC_FREE_KEYPACK(psrcskeypck, srcskeypckcnt);
					return true;		// for continue.
				}
				DKC_FREE_KEYPACK(psrcskeypck, srcskeypckcnt);

				// set subkey name into parent key
				result	= k2hdkc_full_set_subkeys(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pPKey, PKeyLen, pskeypck, skeypckcnt);
				rescode = k2hdkc_get_lastres_code();

				DKC_FREE_KEYPACK(pskeypck, skeypckcnt);

			}else{
				// get subkeys
				PK2HDKCKEYPCK	psrcskeypck		= NULL;
				int				srcskeypckcnt	= 0;
				if(!GetSubkeysCommand(chmpxhandle, pPKey, PKeyLen, is_noattr, &psrcskeypck, &srcskeypckcnt)){
					ERR("Something error occurred during getting subkeys.");
					return true;		// for continue.
				}
				// add new subkey
				PK2HDKCKEYPCK	pskeypck	= NULL;
				int				skeypckcnt	= 0;
				if(!AddSubkeyIntoSubkeysPack(pSKey, SkeyLen, psrcskeypck, srcskeypckcnt, &pskeypck, &skeypckcnt)){
					ERR("Something error occurred adding new subkey into subkeys array.");
					DKC_FREE_KEYPACK(psrcskeypck, srcskeypckcnt);
					return true;		// for continue.
				}
				DKC_FREE_KEYPACK(psrcskeypck, srcskeypckcnt);
				// convert
				unsigned char*	psubkeys	= NULL;
				size_t			subkeyslen	= 0;
				if(!K2hdkcCvtPackToSubkeys(pskeypck, skeypckcnt, &psubkeys, &subkeyslen)){
					ERR("Something error occurred adding new subkey into subkeys array.");
					DKC_FREE_KEYPACK(pskeypck, skeypckcnt);
					return true;		// for continue.
				}
				DKC_FREE_KEYPACK(pskeypck, skeypckcnt);

				// set subkey name into parent key
				K2hdkcComSetSubkeys*	pComObj = GetOtSlaveK2hdkcComSetSubkeys(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin);
				if(!pComObj){
					ERR("Something internal error occurred. could not make command object.");
					DKC_FREE(psubkeys);
					return true;		// for continue.
				}
				result = pComObj->CommandSend(pPKey, PKeyLen, psubkeys, subkeyslen, &rescode);

				DKC_FREE(psubkeys);
				DKC_DELETE(pComObj);
			}
		}else{
			// use permanent chmpx object
			if(isModeCAPI){
				// get subkeys
				PK2HDKCKEYPCK	psrcskeypck		= NULL;
				int				srcskeypckcnt	= 0;
				if(!GetSubkeysCommand(chmpxhandle, pPKey, PKeyLen, is_noattr, &psrcskeypck, &srcskeypckcnt)){
					ERR("Something error occurred during getting subkeys.");
					return true;		// for continue.
				}
				// add new subkey
				PK2HDKCKEYPCK	pskeypck	= NULL;
				int				skeypckcnt	= 0;
				if(!AddSubkeyIntoSubkeysPack(pSKey, SkeyLen, psrcskeypck, srcskeypckcnt, &pskeypck, &skeypckcnt)){
					ERR("Something error occurred adding new subkey into subkeys array.");
					DKC_FREE_KEYPACK(psrcskeypck, srcskeypckcnt);
					return true;		// for continue.
				}
				DKC_FREE_KEYPACK(psrcskeypck, srcskeypckcnt);

				// set subkey name into parent key
				result	= k2hdkc_pm_set_subkeys(chmpxhandle, pPKey, PKeyLen, pskeypck, skeypckcnt);
				rescode	= k2hdkc_get_res_code(chmpxhandle);

				DKC_FREE_KEYPACK(pskeypck, skeypckcnt);

			}else{
				// get subkeys
				PK2HDKCKEYPCK	psrcskeypck		= NULL;
				int				srcskeypckcnt	= 0;
				if(!GetSubkeysCommand(chmpxhandle, pPKey, PKeyLen, is_noattr, &psrcskeypck, &srcskeypckcnt)){
					ERR("Something error occurred during getting subkeys.");
					return true;		// for continue.
				}
				// add new subkey
				PK2HDKCKEYPCK	pskeypck	= NULL;
				int				skeypckcnt	= 0;
				if(!AddSubkeyIntoSubkeysPack(pSKey, SkeyLen, psrcskeypck, srcskeypckcnt, &pskeypck, &skeypckcnt)){
					ERR("Something error occurred adding new subkey into subkeys array.");
					DKC_FREE_KEYPACK(psrcskeypck, srcskeypckcnt);
					return true;		// for continue.
				}
				DKC_FREE_KEYPACK(psrcskeypck, srcskeypckcnt);
				// convert
				unsigned char*	psubkeys	= NULL;
				size_t			subkeyslen	= 0;
				if(!K2hdkcCvtPackToSubkeys(pskeypck, skeypckcnt, &psubkeys, &subkeyslen)){
					ERR("Something error occurred adding new subkey into subkeys array.");
					DKC_FREE_KEYPACK(pskeypck, skeypckcnt);
					return true;		// for continue.
				}
				DKC_FREE_KEYPACK(pskeypck, skeypckcnt);

				// set subkey name into parent key
				K2hdkcSlave*			pSlave	= reinterpret_cast<K2hdkcSlave*>(chmpxhandle);
				K2hdkcComSetSubkeys*	pComObj	= GetPmSlaveK2hdkcComSetSubkeys(pSlave);
				if(!pComObj){
					ERR("Something internal error occurred. could not make command object.");
					DKC_FREE(psubkeys);
					return true;		// for continue.
				}
				result = pComObj->CommandSend(pPKey, PKeyLen, psubkeys, subkeyslen, &rescode);

				DKC_FREE(psubkeys);
				DKC_DELETE(pComObj);
			}
		}

		// check result
		if(!result){
			ERR("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
			return true;		// for continue.
		}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
			MSG("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
		}
	}else{
		// add subkey with value and set into parent key
		if(!RawSetSubkeyCommand(chmpxhandle, pPKey, PKeyLen, pSKey, SkeyLen, pVal, ValLen, !is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire))){
			ERR("Something error occurred during set subkey and value into parent key.");
			return true;		// for continue.
		}
	}
	return true;
}

//
// Command Line: rmsub(delsub) <parent key> [<key> | all] [rmsub] [noattrcheck]
//
static bool RemoveSubkeyCommand(k2hdkc_chmpx_h chmpxhandle, params_t& params)
{
	// check parameter
	string	strParentKey= params[0].c_str();
	string	strKey("");
	bool	is_noattr	= false;
	bool	is_rmsub	= false;
	bool	is_all		= false;
	for(size_t pos = 1; pos < params.size(); ++pos){
		if(0 == strcasecmp(params[pos].c_str(), "noattrcheck")){
			is_noattr	= true;
		}else if(0 == strcasecmp(params[pos].c_str(), "rmsub")){
			is_rmsub	= true;
		}else if(0 == strcasecmp(params[pos].c_str(), "all")){
			is_all		= true;
		}else{
			if(1 == pos){
				strKey	= params[pos].c_str();
			}else{
				ERR("Unknown parameter(%s) for remove subkey command.", params[pos].c_str());
				return true;	// for continue.
			}
		}
	}
	if(is_all && !strKey.empty()){
		ERR("Parameter subkey name and \"all\" not be specified together.");
		return true;	// for continue.
	}else if(!is_all && strKey.empty()){
		ERR("Parameter subkey name or \"all\" must be specified.");
		return true;	// for continue.
	}

	// [0] Get subkeys list
	PK2HDKCKEYPCK	pskeypck	= NULL;
	int				skeypckcnt	= 0;
	if(!GetSubkeysCommand(chmpxhandle, reinterpret_cast<const unsigned char*>(strParentKey.c_str()), strParentKey.length() + 1, is_noattr, &pskeypck, &skeypckcnt)){
		ERR("Something error occurred during getting subkeys.");
		return true;	// for continue.
	}

	// [1] do command
	bool			result;
	dkcres_type_t	rescode	= DKC_NORESTYPE;
	if(K2HDKC_INVALID_HANDLE == chmpxhandle){
		// use one-time chmpx object
		if(isModeCAPI){
			if(is_all){
				// remove all subkey list
				result = k2hdkc_full_clear_subkeys(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strParentKey.c_str()), strParentKey.length() + 1);
			}else{
				// remove target key
				PK2HDKCKEYPCK	pnewskeypck		= NULL;
				int				newskeypckcnt	= 0;
				if(!RemoveSubkeyFromSubkeysPack(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, pskeypck, skeypckcnt, &pnewskeypck, &newskeypckcnt)){
					ERR("Something error occurred during removing subkey.");
					DKC_FREE_KEYPACK(pskeypck, skeypckcnt);
					return true;	// for continue.
				}
				result = k2hdkc_full_set_subkeys(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strParentKey.c_str()), strParentKey.length() + 1, pnewskeypck, newskeypckcnt);

				DKC_FREE_KEYPACK(pnewskeypck, newskeypckcnt);
			}
			rescode = k2hdkc_get_lastres_code();

		}else{
			unsigned char*	psubkeys	= NULL;			// NULL = remove all subkey list
			size_t			subkeyslen	= 0;
			if(!is_all){
				// remove target key
				PK2HDKCKEYPCK	pnewskeypck		= NULL;
				int				newskeypckcnt	= 0;
				if(!RemoveSubkeyFromSubkeysPack(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, pskeypck, skeypckcnt, &pnewskeypck, &newskeypckcnt)){
					ERR("Something error occurred during removing subkey.");
					DKC_FREE_KEYPACK(pskeypck, skeypckcnt);
					return true;	// for continue.
				}

				// convert
				if(!K2hdkcCvtPackToSubkeys(pnewskeypck, newskeypckcnt, &psubkeys, &subkeyslen)){
					ERR("Something error occurred adding new subkey into subkeys array.");
					DKC_FREE_KEYPACK(pnewskeypck, newskeypckcnt);
					DKC_FREE_KEYPACK(pskeypck, skeypckcnt);
					return true;		// for continue.
				}
				DKC_FREE_KEYPACK(pnewskeypck, newskeypckcnt);
			}

			// set new subkey list into parent key
			K2hdkcComSetSubkeys*	pComObj = GetOtSlaveK2hdkcComSetSubkeys(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				DKC_FREE(psubkeys);
				DKC_FREE_KEYPACK(pskeypck, skeypckcnt);
				return true;		// for continue.
			}
			result = pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strParentKey.c_str()), strParentKey.length() + 1, psubkeys, subkeyslen, &rescode);

			DKC_FREE(psubkeys);
			DKC_DELETE(pComObj);
		}

	}else{
		// use permanent chmpx object
		if(isModeCAPI){
			if(is_all){
				// remove all subkey list
				result = k2hdkc_pm_clear_subkeys(chmpxhandle, reinterpret_cast<const unsigned char*>(strParentKey.c_str()), strParentKey.length() + 1);

			}else{
				// remove target key
				PK2HDKCKEYPCK	pnewskeypck		= NULL;
				int				newskeypckcnt	= 0;
				if(!RemoveSubkeyFromSubkeysPack(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, pskeypck, skeypckcnt, &pnewskeypck, &newskeypckcnt)){
					ERR("Something error occurred during removing subkey.");
					DKC_FREE_KEYPACK(pskeypck, skeypckcnt);
					return true;	// for continue.
				}
				result = k2hdkc_pm_set_subkeys(chmpxhandle, reinterpret_cast<const unsigned char*>(strParentKey.c_str()), strParentKey.length() + 1, pnewskeypck, newskeypckcnt);

				DKC_FREE_KEYPACK(pnewskeypck, newskeypckcnt);
			}
			rescode	= k2hdkc_get_res_code(chmpxhandle);

		}else{
			unsigned char*	psubkeys	= NULL;			// NULL = remove all subkey list
			size_t			subkeyslen	= 0;
			if(!is_all){
				// remove target key
				PK2HDKCKEYPCK	pnewskeypck		= NULL;
				int				newskeypckcnt	= 0;
				if(!RemoveSubkeyFromSubkeysPack(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, pskeypck, skeypckcnt, &pnewskeypck, &newskeypckcnt)){
					ERR("Something error occurred during removing subkey.");
					DKC_FREE_KEYPACK(pskeypck, skeypckcnt);
					return true;	// for continue.
				}

				// convert
				if(!K2hdkcCvtPackToSubkeys(pnewskeypck, newskeypckcnt, &psubkeys, &subkeyslen)){
					ERR("Something error occurred adding new subkey into subkeys array.");
					DKC_FREE_KEYPACK(pnewskeypck, newskeypckcnt);
					DKC_FREE_KEYPACK(pskeypck, skeypckcnt);
					return true;		// for continue.
				}
				DKC_FREE_KEYPACK(pnewskeypck, newskeypckcnt);
			}

			// set new subkey list into parent key
			K2hdkcSlave*			pSlave	= reinterpret_cast<K2hdkcSlave*>(chmpxhandle);
			K2hdkcComSetSubkeys*	pComObj = GetPmSlaveK2hdkcComSetSubkeys(pSlave);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				DKC_FREE(psubkeys);
				DKC_FREE_KEYPACK(pskeypck, skeypckcnt);
				return true;		// for continue.
			}
			result = pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strParentKey.c_str()), strParentKey.length() + 1, psubkeys, subkeyslen, &rescode);

			DKC_FREE(psubkeys);
			DKC_DELETE(pComObj);
		}
	}
	// check result
	if(!result){
		ERR("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
		DKC_FREE_KEYPACK(pskeypck, skeypckcnt);
		return true;		// for continue.
	}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
		MSG("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
	}

	// [2] Remove target(all) subkeys
	if(is_rmsub){
		if(is_all){
			// remove all subkey
			for(int cnt = 0; cnt < skeypckcnt; ++cnt){
				if(!RemoveCommand(chmpxhandle, pskeypck[cnt].pkey, pskeypck[cnt].length, true)){
					WAN("Something internal error occurred during removing subkeys. but continue...");
				}
			}
		}else{
			// remove target subkey
			if(!RemoveCommand(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, true)){
				WAN("Something internal error occurred during removing target subkey.");
			}
		}
	}
	DKC_FREE_KEYPACK(pskeypck, skeypckcnt);

	return true;
}

//
// Command Line: rename(ren) <key> <new key> [parent=<key>] [noattrcheck] [pass=....] [expire=sec]
//
static bool RenameCommand(k2hdkc_chmpx_h chmpxhandle, params_t& params)
{
	// check parameter
	string	strOldKey	= params[0].c_str();
	string	strNewKey	= params[1].c_str();
	bool	is_noattr	= false;
	time_t	Expire		= -1;
	string	strParentKey("");
	string	PassPhrase("");
	for(size_t pos = 2; pos < params.size(); ++pos){
		if(0 == strncasecmp(params[pos].c_str(), "PARENT=", 7)){
			strParentKey = params[pos].substr(7);
		}else if(0 == strncasecmp(params[pos].c_str(), "PASS=", 5)){
			PassPhrase = params[pos].substr(5);
		}else if(0 == strncasecmp(params[pos].c_str(), "EXPIRE=", 7)){
			Expire = static_cast<time_t>(atoi(params[pos].substr(7).c_str()));
		}else if(0 == strcasecmp(params[pos].c_str(), "noattrcheck")){
			is_noattr = true;
		}else{
			ERR("Unknown parameter(%s) for rename command.", params[pos].c_str());
			return true;	// for continue.
		}
	}

	// For simplify and facilitate visualization.
	const unsigned char*	pOldKey			= reinterpret_cast<const unsigned char*>(strOldKey.c_str());
	size_t					OldKeyLen		= strOldKey.length() + 1;
	const unsigned char*	pNewKey			= reinterpret_cast<const unsigned char*>(strNewKey.c_str());
	size_t					NewkeyLen		= strNewKey.length() + 1;
	const unsigned char*	pParentKey		= strParentKey.empty() ? NULL : reinterpret_cast<const unsigned char*>(strParentKey.c_str());
	size_t					pParentKeyLen	= strParentKey.empty() ? 0 : (strParentKey.length() + 1);

	// do command
	bool			result;
	dkcres_type_t	rescode	= DKC_NORESTYPE;
	if(K2HDKC_INVALID_HANDLE == chmpxhandle){
		// use one-time chmpx object
		if(isModeCAPI){
			if(strParentKey.empty()){
				// rename only key
				if(-1 == Expire && PassPhrase.empty()){
					result = k2hdkc_full_rename(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pOldKey, OldKeyLen, pNewKey, NewkeyLen);
				}else{
					result = k2hdkc_full_rename_wa(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pOldKey, OldKeyLen, pNewKey, NewkeyLen, !is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
				}
			}else{
				// rename key and change subkey list in parent key
				if(-1 == Expire && PassPhrase.empty()){
					result = k2hdkc_full_rename_with_parent(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pOldKey, OldKeyLen, pNewKey, NewkeyLen, pParentKey, pParentKeyLen);
				}else{
					result = k2hdkc_full_rename_with_parent_wa(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pOldKey, OldKeyLen, pNewKey, NewkeyLen, pParentKey, pParentKeyLen, !is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
				}
			}
			rescode = k2hdkc_get_lastres_code();

		}else{
			// rename only key (and change subkey list in parent key)
			K2hdkcComRen*	pComObj = GetOtSlaveK2hdkcComRen(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return true;		// for continue.
			}
			result = pComObj->CommandSend(pOldKey, OldKeyLen, pNewKey, NewkeyLen, pParentKey, pParentKeyLen, !is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);

			DKC_DELETE(pComObj);
		}

	}else{
		// use permanent chmpx object
		if(isModeCAPI){
			if(strParentKey.empty()){
				// rename only key
				if(-1 == Expire && PassPhrase.empty()){
					result = k2hdkc_pm_rename(chmpxhandle, pOldKey, OldKeyLen, pNewKey, NewkeyLen);
				}else{
					result = k2hdkc_pm_rename_wa(chmpxhandle, pOldKey, OldKeyLen, pNewKey, NewkeyLen, !is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
				}
			}else{
				// rename key and change subkey list in parent key
				if(-1 == Expire && PassPhrase.empty()){
					result = k2hdkc_pm_rename_with_parent(chmpxhandle, pOldKey, OldKeyLen, pNewKey, NewkeyLen, pParentKey, pParentKeyLen);
				}else{
					result = k2hdkc_pm_rename_with_parent_wa(chmpxhandle, pOldKey, OldKeyLen, pNewKey, NewkeyLen, pParentKey, pParentKeyLen, !is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
				}
			}
			rescode	= k2hdkc_get_res_code(chmpxhandle);

		}else{
			// rename only key (and change subkey list in parent key)
			K2hdkcSlave*	pSlave	= reinterpret_cast<K2hdkcSlave*>(chmpxhandle);
			K2hdkcComRen*	pComObj = GetPmSlaveK2hdkcComRen(pSlave);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return true;		// for continue.
			}
			result = pComObj->CommandSend(pOldKey, OldKeyLen, pNewKey, NewkeyLen, pParentKey, pParentKeyLen, !is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);

			DKC_DELETE(pComObj);
		}
	}

	// check result
	if(!result){
		ERR("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
		return true;		// for continue.
	}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
		MSG("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
	}

	return true;
}

//
// Sub push command for queue: params => <value> [noattrcheck] [pass=....] [expire=sec]
//
static bool QueuePushCommand(k2hdkc_chmpx_h chmpxhandle, const unsigned char* pName, size_t NameLen, bool is_Fifo, params_t& params)
{
	// check parameter
	if(params.size() < 1){
		ERR("Too few parameter for queue push command.");
		return true;	// for continue.
	}
	string	strValue	= params[0].c_str();
	bool	is_noattr	= false;
	time_t	Expire		= -1;
	string	PassPhrase("");
	for(size_t pos = 1; pos < params.size(); ++pos){
		if(0 == strncasecmp(params[pos].c_str(), "PASS=", 5)){
			PassPhrase = params[pos].substr(5);
		}else if(0 == strncasecmp(params[pos].c_str(), "EXPIRE=", 7)){
			Expire = static_cast<time_t>(atoi(params[pos].substr(7).c_str()));
		}else if(0 == strcasecmp(params[pos].c_str(), "noattrcheck")){
			is_noattr = true;
		}else{
			ERR("Unknown parameter(%s) for queue push command.", params[pos].c_str());
			return true;	// for continue.
		}
	}

	// do command
	bool			result;
	dkcres_type_t	rescode	= DKC_NORESTYPE;
	if(K2HDKC_INVALID_HANDLE == chmpxhandle){
		// use one-time chmpx object
		if(isModeCAPI){
			if(-1 == Expire && PassPhrase.empty()){
				result = k2hdkc_full_q_push(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pName, NameLen, reinterpret_cast<const unsigned char*>(strValue.c_str()), strValue.length() + 1, is_Fifo);
			}else{
				result = k2hdkc_full_q_push_wa(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pName, NameLen, reinterpret_cast<const unsigned char*>(strValue.c_str()), strValue.length() + 1, is_Fifo, !is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
			}
			rescode = k2hdkc_get_lastres_code();

		}else{
			K2hdkcComQPush*	pComObj = GetOtSlaveK2hdkcComQPush(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return true;		// for continue.
			}
			// [NOTE] not set attributes
			result = pComObj->QueueCommandSend(pName, NameLen, reinterpret_cast<const unsigned char*>(strValue.c_str()), strValue.length() + 1, is_Fifo, NULL, 0UL, !is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);

			DKC_DELETE(pComObj);
		}

	}else{
		// use permanent chmpx object
		if(isModeCAPI){
			if(-1 == Expire && PassPhrase.empty()){
				result = k2hdkc_pm_q_push(chmpxhandle, pName, NameLen, reinterpret_cast<const unsigned char*>(strValue.c_str()), strValue.length() + 1, is_Fifo);
			}else{
				result = k2hdkc_pm_q_push_wa(chmpxhandle, pName, NameLen, reinterpret_cast<const unsigned char*>(strValue.c_str()), strValue.length() + 1, is_Fifo, !is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
			}
			rescode	= k2hdkc_get_res_code(chmpxhandle);

		}else{
			K2hdkcSlave*	pSlave	= reinterpret_cast<K2hdkcSlave*>(chmpxhandle);
			K2hdkcComQPush*	pComObj = GetPmSlaveK2hdkcComQPush(pSlave);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return true;		// for continue.
			}
			// [NOTE] not set attributes
			result = pComObj->QueueCommandSend(pName, NameLen, reinterpret_cast<const unsigned char*>(strValue.c_str()), strValue.length() + 1, is_Fifo, NULL, 0UL, !is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);

			DKC_DELETE(pComObj);
		}
	}

	// check result
	if(!result){
		ERR("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
		return true;		// for continue.
	}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
		MSG("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
	}

	return true;
}

//
// Sub pop command for queue: params => [noattrcheck] [pass=....] [dump]
//
static bool QueuePopCommand(k2hdkc_chmpx_h chmpxhandle, const unsigned char* pName, size_t NameLen, bool is_Fifo, params_t& params)
{
	// check parameter
	string	PassPhrase("");
	bool	is_noattr	= false;
	bool	is_dump		= false;
	for(size_t pos = 0; pos < params.size(); ++pos){
		if(0 == strncasecmp(params[pos].c_str(), "PASS=", 5)){
			PassPhrase = params[pos].substr(5);
		}else if(0 == strcasecmp(params[pos].c_str(), "noattrcheck")){
			is_noattr = true;
		}else if(0 == strcasecmp(params[pos].c_str(), "dump")){
			is_dump = true;
		}else{
			ERR("Unknown parameter(%s) for queue push command.", params[pos].c_str());
			return true;	// for continue.
		}
	}

	// do command
	bool			result;
	unsigned char*	pval		= NULL;
	size_t			vallength	= 0;
	dkcres_type_t	rescode		= DKC_NORESTYPE;
	if(K2HDKC_INVALID_HANDLE == chmpxhandle){
		// use one-time chmpx object
		if(isModeCAPI){
			// [NOTE] always check attributes
			if(PassPhrase.empty()){
				result = k2hdkc_full_q_pop(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pName, NameLen, is_Fifo, &pval, &vallength);
			}else{
				result = k2hdkc_full_q_pop_wp(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pName, NameLen, is_Fifo, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &pval, &vallength);
			}
			rescode = k2hdkc_get_lastres_code();

		}else{
			K2hdkcComQPop*	pComObj = GetOtSlaveK2hdkcComQPop(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return true;		// for continue.
			}
			const unsigned char*	pvaltmp		= NULL;
			size_t					valtmplen	= 0L;
			result = pComObj->QueueCommandSend(pName, NameLen, is_Fifo, !is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &pvaltmp, &valtmplen, &rescode);

			if(result && pvaltmp && 0 < valtmplen){
				pval		= k2hbindup(pvaltmp, valtmplen);
				vallength	= valtmplen;
			}
			DKC_DELETE(pComObj);
		}

	}else{
		// use permanent chmpx object
		if(isModeCAPI){
			// [NOTE] always check attributes
			if(PassPhrase.empty()){
				result = k2hdkc_pm_q_pop(chmpxhandle, pName, NameLen, is_Fifo, &pval, &vallength);
			}else{
				result = k2hdkc_pm_q_pop_wp(chmpxhandle, pName, NameLen, is_Fifo, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &pval, &vallength);
			}
			rescode	= k2hdkc_get_res_code(chmpxhandle);

		}else{
			K2hdkcSlave*	pSlave	= reinterpret_cast<K2hdkcSlave*>(chmpxhandle);
			K2hdkcComQPop*	pComObj = GetPmSlaveK2hdkcComQPop(pSlave);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return true;		// for continue.
			}
			const unsigned char*	pvaltmp		= NULL;
			size_t					valtmplen	= 0L;
			result = pComObj->QueueCommandSend(pName, NameLen, is_Fifo, !is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &pvaltmp, &valtmplen, &rescode);

			if(result && pvaltmp && 0 < valtmplen){
				pval		= k2hbindup(pvaltmp, valtmplen);
				vallength	= valtmplen;
			}
			DKC_DELETE(pComObj);
		}
	}

	// check result
	if(!result){
		ERR("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
		return true;		// for continue.
	}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
		MSG("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
	}

	// print result
	if(pval && 0 < vallength){
		if(!is_dump){
			char*	pQueueName	= GetPrintableString(pName, NameLen);
			char*	pQueueValue	= GetPrintableString(pval, vallength);
			PRN("popped \"%s\" queue => \"%s\"", (pQueueName ? pQueueName : ""), (pQueueValue ? pQueueValue : ""));
			DKC_FREE(pQueueName);
			DKC_FREE(pQueueValue);
		}else{
			BinaryDumpUtility("queue name  =", pName, NameLen, 0);
			BinaryDumpUtility("popped value=", pval, vallength, 0);
		}
	}else{
		char*	pQueueName	= GetPrintableString(pName, NameLen);
		PRN("popped \"%s\" queue => could not pop", (pQueueName ? pQueueName : ""));
		DKC_FREE(pQueueName);
	}
	DKC_FREE(pval);

	return true;

}

//
// Sub remove command for queue: params => <count> [pass=...]
//
static bool QueueRemoveCommand(k2hdkc_chmpx_h chmpxhandle, const unsigned char* pName, size_t NameLen, bool is_Fifo, params_t& params)
{
	// check parameter
	string	PassPhrase("");
	if(params.size() < 1){
		ERR("Too few parameter for queue remove command.");
		return true;		// for continue.
	}else if(2 < params.size()){
		ERR("Unknown parameter(%s) for queue remove command.", params[2].c_str());
		return true;		// for continue.
	}else if(2 == params.size()){
		if(0 == strncasecmp(params[1].c_str(), "PASS=", 5)){
			PassPhrase = params[1].substr(5);
		}else{
			ERR("Unknown parameter(%s) for queue push command.", params[1].c_str());
			return true;	// for continue.
		}
	}
	int		RmCount = atoi(params[0].c_str());
	if(RmCount <= 0){
		ERR("Parameter \"count\" must be positive number.");
		return true;		// for continue.
	}

	// do command
	bool			result;
	dkcres_type_t	rescode	= DKC_NORESTYPE;
	if(K2HDKC_INVALID_HANDLE == chmpxhandle){
		// use one-time chmpx object
		if(isModeCAPI){
			if(PassPhrase.empty()){
				result = k2hdkc_full_q_remove(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pName, NameLen, RmCount, is_Fifo);
			}else{
				result = k2hdkc_full_q_remove_wp(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pName, NameLen, RmCount, is_Fifo, (PassPhrase.empty() ? NULL : PassPhrase.c_str()));
			}
			rescode = k2hdkc_get_lastres_code();

		}else{
			K2hdkcComQDel*	pComObj = GetOtSlaveK2hdkcComQDel(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return true;		// for continue.
			}
			// [NOTE] always check attributes
			result = pComObj->QueueCommandSend(pName, NameLen, RmCount, is_Fifo, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &rescode);

			DKC_DELETE(pComObj);
		}

	}else{
		// use permanent chmpx object
		if(isModeCAPI){
			if(PassPhrase.empty()){
				result = k2hdkc_pm_q_remove(chmpxhandle, pName, NameLen, RmCount, is_Fifo);
			}else{
				result = k2hdkc_pm_q_remove_wp(chmpxhandle, pName, NameLen, RmCount, is_Fifo, (PassPhrase.empty() ? NULL : PassPhrase.c_str()));
			}
			rescode	= k2hdkc_get_res_code(chmpxhandle);

		}else{
			K2hdkcSlave*	pSlave	= reinterpret_cast<K2hdkcSlave*>(chmpxhandle);
			K2hdkcComQDel*	pComObj = GetPmSlaveK2hdkcComQDel(pSlave);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return true;		// for continue.
			}
			// [NOTE] always check attributes
			result = pComObj->QueueCommandSend(pName, NameLen, RmCount, is_Fifo, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &rescode);

			DKC_DELETE(pComObj);
		}
	}

	// check result
	if(!result){
		ERR("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
		return true;		// for continue.
	}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
		MSG("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
	}

	return true;
}

//
// Command Line:	queue(que) <name> push <fifo | lifo> <value> [noattrcheck] [pass=....] [expire=sec]
//					queue(que) <name> pop <fifo | lifo> [noattrcheck] [pass=...] [dump]
//					queue(que) <name> remove <fifo | lifo> <count> [pass=...]
//
static bool QueueCommand(k2hdkc_chmpx_h chmpxhandle, params_t& params)
{
	// check parameter
	string	strName		= params[0].c_str();
	string	strSubCmd	= params[1].c_str();
	bool	is_Fifo		= false;
	if(0 == strcasecmp(params[2].c_str(), "FIFO")){
		is_Fifo	= true;
	}else if(0 == strcasecmp(params[2].c_str(), "LIFO")){
		is_Fifo	= false;
	}else{
		ERR("Unknown parameter(%s) for queue command.", params[2].c_str());
		return true;	// for continue.
	}
	params.erase(params.begin(), params.begin() + 3);

	// dispatch sub command
	//
	// [NOTICE]
	// Name does not include null character, because this tool should be as same as
	// k2hlinetool. It does not include null character for marker.
	//
	bool	result;
	if(0 == strcasecmp(strSubCmd.c_str(), "push")){
		result = QueuePushCommand(chmpxhandle, reinterpret_cast<const unsigned char*>(strName.c_str()), strName.length(), is_Fifo, params);
	}else if(0 == strcasecmp(strSubCmd.c_str(), "pop")){
		result = QueuePopCommand(chmpxhandle, reinterpret_cast<const unsigned char*>(strName.c_str()), strName.length(), is_Fifo, params);
	}else if(0 == strcasecmp(strSubCmd.c_str(), "remove") || 0 == strcasecmp(strSubCmd.c_str(), "rm") || 0 == strcasecmp(strSubCmd.c_str(), "del")){
		result = QueueRemoveCommand(chmpxhandle, reinterpret_cast<const unsigned char*>(strName.c_str()), strName.length(), is_Fifo, params);
	}else{
		ERR("Unknown sub command(%s) for queue command.", strSubCmd.c_str());
		return true;	// for continue.
	}

	if(!result){
		ERR("Something error occurred during queue %s command.", strSubCmd.c_str());
		return true;	// for continue.
	}

	return true;
}

//
// Sub push command for queue: params => <key> <value> [noattrcheck] [pass=....] [expire=sec]
//
static bool KeyQueuePushCommand(k2hdkc_chmpx_h chmpxhandle, const unsigned char* pName, size_t NameLen, bool is_Fifo, params_t& params)
{
	// check parameter
	if(params.size() < 2){
		ERR("Too few parameter for key queue push command.");
		return true;	// for continue.
	}
	string	strKey		= params[0].c_str();
	string	strValue	= params[1].c_str();
	bool	is_noattr	= false;
	time_t	Expire		= -1;
	string	PassPhrase("");
	for(size_t pos = 2; pos < params.size(); ++pos){
		if(0 == strncasecmp(params[pos].c_str(), "PASS=", 5)){
			PassPhrase = params[pos].substr(5);
		}else if(0 == strncasecmp(params[pos].c_str(), "EXPIRE=", 7)){
			Expire = static_cast<time_t>(atoi(params[pos].substr(7).c_str()));
		}else if(0 == strcasecmp(params[pos].c_str(), "noattrcheck")){
			is_noattr = true;
		}else{
			ERR("Unknown parameter(%s) for key queue push command.", params[pos].c_str());
			return true;	// for continue.
		}
	}

	// do command
	bool			result;
	dkcres_type_t	rescode	= DKC_NORESTYPE;
	if(K2HDKC_INVALID_HANDLE == chmpxhandle){
		// use one-time chmpx object
		if(isModeCAPI){
			if(-1 == Expire && PassPhrase.empty()){
				result = k2hdkc_full_keyq_push(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pName, NameLen, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, reinterpret_cast<const unsigned char*>(strValue.c_str()), strValue.length() + 1, is_Fifo);
			}else{
				result = k2hdkc_full_keyq_push_wa(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pName, NameLen, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, reinterpret_cast<const unsigned char*>(strValue.c_str()), strValue.length() + 1, is_Fifo, !is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
			}
			rescode = k2hdkc_get_lastres_code();

		}else{
			K2hdkcComQPush*	pComObj = GetOtSlaveK2hdkcComQPush(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return true;		// for continue.
			}
			// [NOTE] not set attributes
			result = pComObj->KeyQueueCommandSend(pName, NameLen, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, reinterpret_cast<const unsigned char*>(strValue.c_str()), strValue.length() + 1, is_Fifo, NULL, 0UL, !is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);

			DKC_DELETE(pComObj);
		}

	}else{
		// use permanent chmpx object
		if(isModeCAPI){
			if(-1 == Expire && PassPhrase.empty()){
				result = k2hdkc_pm_keyq_push(chmpxhandle, pName, NameLen, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, reinterpret_cast<const unsigned char*>(strValue.c_str()), strValue.length() + 1, is_Fifo);
			}else{
				result = k2hdkc_pm_keyq_push_wa(chmpxhandle, pName, NameLen, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, reinterpret_cast<const unsigned char*>(strValue.c_str()), strValue.length() + 1, is_Fifo, !is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
			}
			rescode	= k2hdkc_get_res_code(chmpxhandle);

		}else{
			K2hdkcSlave*	pSlave	= reinterpret_cast<K2hdkcSlave*>(chmpxhandle);
			K2hdkcComQPush*	pComObj = GetPmSlaveK2hdkcComQPush(pSlave);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return true;		// for continue.
			}
			// [NOTE] not set attributes
			result = pComObj->KeyQueueCommandSend(pName, NameLen, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, reinterpret_cast<const unsigned char*>(strValue.c_str()), strValue.length() + 1, is_Fifo, NULL, 0UL, !is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);

			DKC_DELETE(pComObj);
		}
	}

	// check result
	if(!result){
		ERR("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
		return true;		// for continue.
	}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
		MSG("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
	}

	return true;
}

//
// Sub pop command for queue: params => [noattrcheck] [pass=....] [dump]
//
static bool KeyQueuePopCommand(k2hdkc_chmpx_h chmpxhandle, const unsigned char* pName, size_t NameLen, bool is_Fifo, params_t& params)
{
	// check parameter
	string	PassPhrase("");
	bool	is_noattr	= false;
	bool	is_dump		= false;
	for(size_t pos = 0; pos < params.size(); ++pos){
		if(0 == strncasecmp(params[pos].c_str(), "PASS=", 5)){
			PassPhrase = params[pos].substr(5);
		}else if(0 == strcasecmp(params[pos].c_str(), "noattrcheck")){
			is_noattr = true;
		}else if(0 == strcasecmp(params[pos].c_str(), "dump")){
			is_dump = true;
		}else{
			ERR("Unknown parameter(%s) for key queue push command.", params[pos].c_str());
			return true;	// for continue.
		}
	}

	// do command
	bool			result;
	unsigned char*	pkey		= NULL;
	size_t			keylength	= 0;
	unsigned char*	pval		= NULL;
	size_t			vallength	= 0;
	dkcres_type_t	rescode		= DKC_NORESTYPE;
	if(K2HDKC_INVALID_HANDLE == chmpxhandle){
		// use one-time chmpx object
		if(isModeCAPI){
			// [NOTE] always check attributes
			if(PassPhrase.empty()){
				result = k2hdkc_full_keyq_pop(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pName, NameLen, is_Fifo, &pkey, &keylength, &pval, &vallength);
			}else{
				result = k2hdkc_full_keyq_pop_wp(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pName, NameLen, is_Fifo, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &pkey, &keylength, &pval, &vallength);
			}
			rescode = k2hdkc_get_lastres_code();

		}else{
			K2hdkcComQPop*	pComObj = GetOtSlaveK2hdkcComQPop(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return true;		// for continue.
			}
			const unsigned char*	pkeytmp		= NULL;
			size_t					keytmplen	= 0L;
			const unsigned char*	pvaltmp		= NULL;
			size_t					valtmplen	= 0L;
			result = pComObj->KeyQueueCommandSend(pName, NameLen, is_Fifo, !is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &pkeytmp, &keytmplen, &pvaltmp, &valtmplen, &rescode);

			if(result && pkeytmp && 0 < keytmplen){
				pkey			= k2hbindup(pkeytmp, keytmplen);
				keylength		= keytmplen;

				if(pvaltmp && 0 < valtmplen){
					pval		= k2hbindup(pvaltmp, valtmplen);
					vallength	= valtmplen;
				}
			}
			DKC_DELETE(pComObj);
		}

	}else{
		// use permanent chmpx object
		if(isModeCAPI){
			// [NOTE] always check attributes
			if(PassPhrase.empty()){
				result = k2hdkc_pm_keyq_pop(chmpxhandle, pName, NameLen, is_Fifo, &pkey, &keylength, &pval, &vallength);
			}else{
				result = k2hdkc_pm_keyq_pop_wp(chmpxhandle, pName, NameLen, is_Fifo, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &pkey, &keylength, &pval, &vallength);
			}
			rescode	= k2hdkc_get_res_code(chmpxhandle);

		}else{
			K2hdkcSlave*	pSlave	= reinterpret_cast<K2hdkcSlave*>(chmpxhandle);
			K2hdkcComQPop*	pComObj = GetPmSlaveK2hdkcComQPop(pSlave);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return true;		// for continue.
			}
			const unsigned char*	pkeytmp		= NULL;
			size_t					keytmplen	= 0L;
			const unsigned char*	pvaltmp		= NULL;
			size_t					valtmplen	= 0L;
			result = pComObj->KeyQueueCommandSend(pName, NameLen, is_Fifo, !is_noattr, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &pkeytmp, &keytmplen, &pvaltmp, &valtmplen, &rescode);

			if(result && pkeytmp && 0 < keytmplen){
				pkey			= k2hbindup(pkeytmp, keytmplen);
				keylength		= keytmplen;

				if(pvaltmp && 0 < valtmplen){
					pval		= k2hbindup(pvaltmp, valtmplen);
					vallength	= valtmplen;
				}
			}
			DKC_DELETE(pComObj);
		}
	}

	// check result
	if(!result){
		ERR("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
		return true;		// for continue.
	}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
		MSG("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
	}

	// print result
	if(pval && 0 < vallength){
		if(!is_dump){
			char*	pKeyQueueName	= GetPrintableString(pName, NameLen);
			char*	pKeyQueueKey	= GetPrintableString(pkey, keylength);
			char*	pKeyQueueValue	= GetPrintableString(pval, vallength);
			PRN("popped \"%s\" queue => \"%s\" : \"%s\"", (pKeyQueueName ? pKeyQueueName : ""), (pKeyQueueKey ? pKeyQueueKey : ""), (pKeyQueueValue ? pKeyQueueValue : ""));
			DKC_FREE(pKeyQueueName);
			DKC_FREE(pKeyQueueKey);
			DKC_FREE(pKeyQueueValue);
		}else{
			BinaryDumpUtility("queue name  =", pName, NameLen, 0);
			BinaryDumpUtility("queue key   =", pkey, keylength, 0);
			BinaryDumpUtility("popped value=", pval, vallength, 0);
		}
	}else{
		char*	pKeyQueueName	= GetPrintableString(pName, NameLen);
		PRN("popped \"%s\" queue => could not pop", (pKeyQueueName ? pKeyQueueName : ""));
		DKC_FREE(pKeyQueueName);
	}
	DKC_FREE(pval);

	return true;

}

//
// Sub remove command for queue: params => <count> [pass=...]
//
static bool KeyQueueRemoveCommand(k2hdkc_chmpx_h chmpxhandle, const unsigned char* pName, size_t NameLen, bool is_Fifo, params_t& params)
{
	// check parameter
	string	PassPhrase("");
	if(params.size() < 1){
		ERR("Too few parameter for key queue remove command.");
		return true;		// for continue.
	}else if(2 < params.size()){
		ERR("Unknown parameter(%s) for key queue remove command.", params[2].c_str());
		return true;		// for continue.
	}else if(2 == params.size()){
		if(0 == strncasecmp(params[1].c_str(), "PASS=", 5)){
			PassPhrase = params[1].substr(5);
		}else{
			ERR("Unknown parameter(%s) for key queue push command.", params[1].c_str());
			return true;	// for continue.
		}
	}
	int		RmCount = atoi(params[0].c_str());
	if(RmCount <= 0){
		ERR("Parameter \"count\" must be positive number.");
		return true;		// for continue.
	}

	// do command
	bool			result;
	dkcres_type_t	rescode	= DKC_NORESTYPE;
	if(K2HDKC_INVALID_HANDLE == chmpxhandle){
		// use one-time chmpx object
		if(isModeCAPI){
			if(PassPhrase.empty()){
				result = k2hdkc_full_keyq_remove(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pName, NameLen, RmCount, is_Fifo);
			}else{
				result = k2hdkc_full_keyq_remove_wp(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, pName, NameLen, RmCount, is_Fifo, (PassPhrase.empty() ? NULL : PassPhrase.c_str()));
			}
			rescode = k2hdkc_get_lastres_code();

		}else{
			K2hdkcComQDel*	pComObj = GetOtSlaveK2hdkcComQDel(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return true;		// for continue.
			}
			// [NOTE] always check attributes
			result = pComObj->KeyQueueCommandSend(pName, NameLen, RmCount, is_Fifo, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &rescode);

			DKC_DELETE(pComObj);
		}

	}else{
		// use permanent chmpx object
		if(isModeCAPI){
			if(PassPhrase.empty()){
				result = k2hdkc_pm_keyq_remove(chmpxhandle, pName, NameLen, RmCount, is_Fifo);
			}else{
				result = k2hdkc_pm_keyq_remove_wp(chmpxhandle, pName, NameLen, RmCount, is_Fifo, (PassPhrase.empty() ? NULL : PassPhrase.c_str()));
			}
			rescode	= k2hdkc_get_res_code(chmpxhandle);

		}else{
			K2hdkcSlave*	pSlave	= reinterpret_cast<K2hdkcSlave*>(chmpxhandle);
			K2hdkcComQDel*	pComObj = GetPmSlaveK2hdkcComQDel(pSlave);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return true;		// for continue.
			}
			// [NOTE] always check attributes
			result = pComObj->KeyQueueCommandSend(pName, NameLen, RmCount, is_Fifo, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &rescode);

			DKC_DELETE(pComObj);
		}
	}

	// check result
	if(!result){
		ERR("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
		return true;		// for continue.
	}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
		MSG("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
	}

	return true;
}

//
// Command Line:	keyqueue(kque) <name> push <fifo | lifo> <key> <value> [noattrcheck] [pass=....] [expire=sec]
//					keyqueue(kque) <name> pop <fifo | lifo> [noattrcheck] [pass=...] [dump]
//					keyqueue(kque) <name> remove <fifo | lifo> <count> [pass=...]
//
static bool KeyQueueCommand(k2hdkc_chmpx_h chmpxhandle, params_t& params)
{
	// check parameter
	string	strName		= params[0].c_str();
	string	strSubCmd	= params[1].c_str();
	bool	is_Fifo		= false;
	if(0 == strcasecmp(params[2].c_str(), "FIFO")){
		is_Fifo	= true;
	}else if(0 == strcasecmp(params[2].c_str(), "LIFO")){
		is_Fifo	= false;
	}else{
		ERR("Unknown parameter(%s) for key queue command.", params[2].c_str());
		return true;	// for continue.
	}
	params.erase(params.begin(), params.begin() + 3);

	// dispatch sub command
	//
	// [NOTICE]
	// Name does not include null character, because this tool should be as same as
	// k2hlinetool. It does not include null character for marker.
	//
	bool	result;
	if(0 == strcasecmp(strSubCmd.c_str(), "push")){
		result = KeyQueuePushCommand(chmpxhandle, reinterpret_cast<const unsigned char*>(strName.c_str()), strName.length(), is_Fifo, params);
	}else if(0 == strcasecmp(strSubCmd.c_str(), "pop")){
		result = KeyQueuePopCommand(chmpxhandle, reinterpret_cast<const unsigned char*>(strName.c_str()), strName.length(), is_Fifo, params);
	}else if(0 == strcasecmp(strSubCmd.c_str(), "remove") || 0 == strcasecmp(strSubCmd.c_str(), "rm") || 0 == strcasecmp(strSubCmd.c_str(), "del")){
		result = KeyQueueRemoveCommand(chmpxhandle, reinterpret_cast<const unsigned char*>(strName.c_str()), strName.length(), is_Fifo, params);
	}else{
		ERR("Unknown sub command(%s) for queue command.", strSubCmd.c_str());
		return true;	// for continue.
	}

	if(!result){
		ERR("Something error occurred during key queue %s command.", strSubCmd.c_str());
		return true;	// for continue.
	}

	return true;
}

//
// Sub init command for cas: params	=> <key> <value> [pass=....] [expire=sec]
//
static bool CasInitCommand(k2hdkc_chmpx_h chmpxhandle, CASTYPE type, params_t& params)
{
	// check parameter
	if(params.size() < 2){
		ERR("Too few parameter for cas init command.");
		return true;	// for continue.
	}
	string	strKey		= params[0].c_str();
	string	strValue	= params[1].c_str();
	time_t	Expire		= -1;
	string	PassPhrase("");
	for(size_t pos = 2; pos < params.size(); ++pos){
		if(0 == strncasecmp(params[pos].c_str(), "PASS=", 5)){
			PassPhrase = params[pos].substr(5);
		}else if(0 == strncasecmp(params[pos].c_str(), "EXPIRE=", 7)){
			Expire = static_cast<time_t>(atoi(params[pos].substr(7).c_str()));
		}else{
			ERR("Unknown parameter(%s) for cas init command.", params[pos].c_str());
			return true;	// for continue.
		}
	}

	// do command
	bool			result;
	dkcres_type_t	rescode	= DKC_NORESTYPE;
	if(K2HDKC_INVALID_HANDLE == chmpxhandle){
		// use one-time chmpx object
		if(isModeCAPI){
			if(-1 == Expire && PassPhrase.empty()){
				if(CAS_TYPE_8 == type){
					uint8_t		val	= static_cast<uint8_t>(atoi(strValue.c_str()));
					result			= k2hdkc_full_cas8_init(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, val);
				}else if(CAS_TYPE_16 == type){
					uint16_t	val	= static_cast<uint16_t>(atoi(strValue.c_str()));
					result			= k2hdkc_full_cas16_init(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, val);
				}else if(CAS_TYPE_32 == type){
					uint32_t	val	= static_cast<uint32_t>(atoi(strValue.c_str()));
					result			= k2hdkc_full_cas32_init(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, val);
				}else{	// CAS_TYPE_64 == type
					uint64_t	val	= static_cast<uint64_t>(atoi(strValue.c_str()));
					result			= k2hdkc_full_cas64_init(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, val);
				}
			}else{
				if(CAS_TYPE_8 == type){
					uint8_t		val	= static_cast<uint8_t>(atoi(strValue.c_str()));
					result			= k2hdkc_full_cas8_init_wa(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, val, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
				}else if(CAS_TYPE_16 == type){
					uint16_t	val	= static_cast<uint16_t>(atoi(strValue.c_str()));
					result			= k2hdkc_full_cas16_init_wa(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, val, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
				}else if(CAS_TYPE_32 == type){
					uint32_t	val	= static_cast<uint32_t>(atoi(strValue.c_str()));
					result			= k2hdkc_full_cas32_init_wa(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, val, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
				}else{	// CAS_TYPE_64 == type
					uint64_t	val	= static_cast<uint64_t>(atoi(strValue.c_str()));
					result			= k2hdkc_full_cas64_init_wa(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, val, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
				}
			}
			rescode = k2hdkc_get_lastres_code();

		}else{
			K2hdkcComCasInit*	pComObj = GetOtSlaveK2hdkcComCasInit(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return true;		// for continue.
			}
			// [NOTE] not set attributes
			if(CAS_TYPE_8 == type){
				uint8_t		val	= static_cast<uint8_t>(atoi(strValue.c_str()));
				result			= pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, val, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
			}else if(CAS_TYPE_16 == type){
				uint16_t	val	= static_cast<uint16_t>(atoi(strValue.c_str()));
				result			= pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, val, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
			}else if(CAS_TYPE_32 == type){
				uint32_t	val	= static_cast<uint32_t>(atoi(strValue.c_str()));
				result			= pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, val, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
			}else{	// CAS_TYPE_64 == type
				uint64_t	val	= static_cast<uint64_t>(atoi(strValue.c_str()));
				result			= pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, val, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
			}
			DKC_DELETE(pComObj);
		}

	}else{
		// use permanent chmpx object
		if(isModeCAPI){
			if(-1 == Expire && PassPhrase.empty()){
				if(CAS_TYPE_8 == type){
					uint8_t		val	= static_cast<uint8_t>(atoi(strValue.c_str()));
					result			= k2hdkc_pm_cas8_init(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, val);
				}else if(CAS_TYPE_16 == type){
					uint16_t	val	= static_cast<uint16_t>(atoi(strValue.c_str()));
					result			= k2hdkc_pm_cas16_init(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, val);
				}else if(CAS_TYPE_32 == type){
					uint32_t	val	= static_cast<uint32_t>(atoi(strValue.c_str()));
					result			= k2hdkc_pm_cas32_init(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, val);
				}else{	// CAS_TYPE_64 == type
					uint64_t	val	= static_cast<uint64_t>(atoi(strValue.c_str()));
					result			= k2hdkc_pm_cas64_init(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, val);
				}
			}else{
				if(CAS_TYPE_8 == type){
					uint8_t		val	= static_cast<uint8_t>(atoi(strValue.c_str()));
					result			= k2hdkc_pm_cas8_init_wa(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, val, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
				}else if(CAS_TYPE_16 == type){
					uint16_t	val	= static_cast<uint16_t>(atoi(strValue.c_str()));
					result			= k2hdkc_pm_cas16_init_wa(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, val, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
				}else if(CAS_TYPE_32 == type){
					uint32_t	val	= static_cast<uint32_t>(atoi(strValue.c_str()));
					result			= k2hdkc_pm_cas32_init_wa(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, val, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
				}else{	// CAS_TYPE_64 == type
					uint64_t	val	= static_cast<uint64_t>(atoi(strValue.c_str()));
					result			= k2hdkc_pm_cas64_init_wa(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, val, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
				}
			}
			rescode	= k2hdkc_get_res_code(chmpxhandle);

		}else{
			K2hdkcSlave*		pSlave	= reinterpret_cast<K2hdkcSlave*>(chmpxhandle);
			K2hdkcComCasInit*	pComObj = GetPmSlaveK2hdkcComCasInit(pSlave);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return true;		// for continue.
			}
			// [NOTE] not set attributes
			if(CAS_TYPE_8 == type){
				uint8_t		val	= static_cast<uint8_t>(atoi(strValue.c_str()));
				result			= pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, val, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
			}else if(CAS_TYPE_16 == type){
				uint16_t	val	= static_cast<uint16_t>(atoi(strValue.c_str()));
				result			= pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, val, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
			}else if(CAS_TYPE_32 == type){
				uint32_t	val	= static_cast<uint32_t>(atoi(strValue.c_str()));
				result			= pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, val, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
			}else{	// CAS_TYPE_64 == type
				uint64_t	val	= static_cast<uint64_t>(atoi(strValue.c_str()));
				result			= pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, val, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
			}
			DKC_DELETE(pComObj);
		}
	}

	// check result
	if(!result){
		ERR("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
		return true;		// for continue.
	}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
		MSG("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
	}

	return true;
}

//
// Sub get command for cas: params	=> <key> [pass=....]
//
static bool CasGetCommand(k2hdkc_chmpx_h chmpxhandle, CASTYPE type, params_t& params)
{
	// check parameter
	if(params.size() < 1){
		ERR("Too few parameter for cas init command.");
		return true;	// for continue.
	}
	string	strKey		= params[0].c_str();
	string	PassPhrase("");
	for(size_t pos = 1; pos < params.size(); ++pos){
		if(0 == strncasecmp(params[pos].c_str(), "PASS=", 5)){
			PassPhrase = params[pos].substr(5);
		}else{
			ERR("Unknown parameter(%s) for cas get command.", params[pos].c_str());
			return true;	// for continue.
		}
	}

	// do command
	bool			result;
	dkcres_type_t	rescode	= DKC_NORESTYPE;
	uint8_t			val8	= 0;
	uint16_t		val16	= 0;
	uint32_t		val32	= 0;
	uint64_t		val64	= 0;
	if(K2HDKC_INVALID_HANDLE == chmpxhandle){
		// use one-time chmpx object
		if(isModeCAPI){
			if(PassPhrase.empty()){
				if(CAS_TYPE_8 == type){
					result	= k2hdkc_full_cas8_get(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, &val8);
				}else if(CAS_TYPE_16 == type){
					result	= k2hdkc_full_cas16_get(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, &val16);
				}else if(CAS_TYPE_32 == type){
					result	= k2hdkc_full_cas32_get(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, &val32);
				}else{	// CAS_TYPE_64 == type
					result	= k2hdkc_full_cas64_get(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, &val64);
				}
			}else{
				if(CAS_TYPE_8 == type){
					result	= k2hdkc_full_cas8_get_wa(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &val8);
				}else if(CAS_TYPE_16 == type){
					result	= k2hdkc_full_cas16_get_wa(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &val16);
				}else if(CAS_TYPE_32 == type){
					result	= k2hdkc_full_cas32_get_wa(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &val32);
				}else{	// CAS_TYPE_64 == type
					result	= k2hdkc_full_cas64_get_wa(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &val64);
				}
			}
			rescode = k2hdkc_get_lastres_code();

		}else{
			K2hdkcComCasGet*	pComObj = GetOtSlaveK2hdkcComCasGet(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return true;		// for continue.
			}
			// [NOTE] not set attributes
			if(CAS_TYPE_8 == type){
				const uint8_t*	pval8 = NULL;
				if(false != (result = pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &pval8, &rescode)) && pval8){
					val8 = *pval8;
				}
			}else if(CAS_TYPE_16 == type){
				const uint16_t*	pval16 = NULL;
				if(false != (result = pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &pval16, &rescode)) && pval16){
					val16 = *pval16;
				}
			}else if(CAS_TYPE_32 == type){
				const uint32_t*	pval32 = NULL;
				if(false != (result = pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &pval32, &rescode)) && pval32){
					val32 = *pval32;
				}
			}else{	// CAS_TYPE_64 == type
				const uint64_t*	pval64 = NULL;
				if(false != (result = pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &pval64, &rescode)) && pval64){
					val64 = *pval64;
				}
			}
			DKC_DELETE(pComObj);
		}

	}else{
		// use permanent chmpx object
		if(isModeCAPI){
			if(PassPhrase.empty()){
				if(CAS_TYPE_8 == type){
					result	= k2hdkc_pm_cas8_get(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, &val8);
				}else if(CAS_TYPE_16 == type){
					result	= k2hdkc_pm_cas16_get(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, &val16);
				}else if(CAS_TYPE_32 == type){
					result	= k2hdkc_pm_cas32_get(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, &val32);
				}else{	// CAS_TYPE_64 == type
					result	= k2hdkc_pm_cas64_get(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, &val64);
				}
			}else{
				if(CAS_TYPE_8 == type){
					result	= k2hdkc_pm_cas8_get_wa(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &val8);
				}else if(CAS_TYPE_16 == type){
					result	= k2hdkc_pm_cas16_get_wa(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &val16);
				}else if(CAS_TYPE_32 == type){
					result	= k2hdkc_pm_cas32_get_wa(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &val32);
				}else{	// CAS_TYPE_64 == type
					result	= k2hdkc_pm_cas64_get_wa(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &val64);
				}
			}
			rescode	= k2hdkc_get_res_code(chmpxhandle);

		}else{
			K2hdkcSlave*		pSlave	= reinterpret_cast<K2hdkcSlave*>(chmpxhandle);
			K2hdkcComCasGet*	pComObj = GetPmSlaveK2hdkcComCasGet(pSlave);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return true;		// for continue.
			}
			// [NOTE] not set attributes
			if(CAS_TYPE_8 == type){
				const uint8_t*	pval8 = NULL;
				if(false != (result = pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &pval8, &rescode)) && pval8){
					val8 = *pval8;
				}
			}else if(CAS_TYPE_16 == type){
				const uint16_t*	pval16 = NULL;
				if(false != (result = pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &pval16, &rescode)) && pval16){
					val16 = *pval16;
				}
			}else if(CAS_TYPE_32 == type){
				const uint32_t*	pval32 = NULL;
				if(false != (result = pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &pval32, &rescode)) && pval32){
					val32 = *pval32;
				}
			}else{	// CAS_TYPE_64 == type
				const uint64_t*	pval64 = NULL;
				if(false != (result = pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), &pval64, &rescode)) && pval64){
					val64 = *pval64;
				}
			}
			DKC_DELETE(pComObj);
		}
	}

	// check result
	if(!result){
		ERR("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
		return true;		// for continue.
	}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
		MSG("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
	}

	// print result
	if(CAS_TYPE_8 == type){
		PRN("\"%s\" => %u (0x%02X)", strKey.c_str(), val8, val8);
	}else if(CAS_TYPE_16 == type){
		PRN("\"%s\" => %u (0x%04X)", strKey.c_str(), val16, val16);
	}else if(CAS_TYPE_32 == type){
		PRN("\"%s\" => %lu (0x%08lX)", strKey.c_str(), val32, val32);
	}else{	// CAS_TYPE_64 == type
		PRN("\"%s\" => %" PRIu64 " (0x%016" PRIx64 ")", strKey.c_str(), val64, val64);
	}
	return true;
}

//
// Sub set command for cas: params	=> <key> <old value> <new value> [pass=....] [expire=sec]
//
static bool CasSetCommand(k2hdkc_chmpx_h chmpxhandle, CASTYPE type, params_t& params)
{
	// check parameter
	if(params.size() < 3){
		ERR("Too few parameter for cas init command.");
		return true;	// for continue.
	}
	string	strKey		= params[0].c_str();
	string	strOldVal	= params[1].c_str();
	string	strNewVal	= params[2].c_str();
	time_t	Expire		= -1;
	string	PassPhrase("");
	for(size_t pos = 3; pos < params.size(); ++pos){
		if(0 == strncasecmp(params[pos].c_str(), "PASS=", 5)){
			PassPhrase = params[pos].substr(5);
		}else if(0 == strncasecmp(params[pos].c_str(), "EXPIRE=", 7)){
			Expire = static_cast<time_t>(atoi(params[pos].substr(7).c_str()));
		}else{
			ERR("Unknown parameter(%s) for cas set command.", params[pos].c_str());
			return true;	// for continue.
		}
	}

	// do command
	bool			result;
	dkcres_type_t	rescode	= DKC_NORESTYPE;
	if(K2HDKC_INVALID_HANDLE == chmpxhandle){
		// use one-time chmpx object
		if(isModeCAPI){
			if(-1 == Expire && PassPhrase.empty()){
				if(CAS_TYPE_8 == type){
					uint8_t		oldval	= static_cast<uint8_t>(atoi(strOldVal.c_str()));
					uint8_t		newval	= static_cast<uint8_t>(atoi(strNewVal.c_str()));
					result				= k2hdkc_full_cas8_set(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, oldval, newval);
				}else if(CAS_TYPE_16 == type){
					uint16_t	oldval	= static_cast<uint16_t>(atoi(strOldVal.c_str()));
					uint16_t	newval	= static_cast<uint16_t>(atoi(strNewVal.c_str()));
					result				= k2hdkc_full_cas16_set(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, oldval, newval);
				}else if(CAS_TYPE_32 == type){
					uint32_t	oldval	= static_cast<uint32_t>(atoi(strOldVal.c_str()));
					uint32_t	newval	= static_cast<uint32_t>(atoi(strNewVal.c_str()));
					result				= k2hdkc_full_cas32_set(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, oldval, newval);
				}else{	// CAS_TYPE_64 == type
					uint64_t	oldval	= static_cast<uint64_t>(atoi(strOldVal.c_str()));
					uint64_t	newval	= static_cast<uint64_t>(atoi(strNewVal.c_str()));
					result				= k2hdkc_full_cas64_set(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, oldval, newval);
				}
			}else{
				if(CAS_TYPE_8 == type){
					uint8_t		oldval	= static_cast<uint8_t>(atoi(strOldVal.c_str()));
					uint8_t		newval	= static_cast<uint8_t>(atoi(strNewVal.c_str()));
					result				= k2hdkc_full_cas8_set_wa(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, oldval, newval, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
				}else if(CAS_TYPE_16 == type){
					uint16_t	oldval	= static_cast<uint16_t>(atoi(strOldVal.c_str()));
					uint16_t	newval	= static_cast<uint16_t>(atoi(strNewVal.c_str()));
					result				= k2hdkc_full_cas16_set_wa(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, oldval, newval, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
				}else if(CAS_TYPE_32 == type){
					uint32_t	oldval	= static_cast<uint32_t>(atoi(strOldVal.c_str()));
					uint32_t	newval	= static_cast<uint32_t>(atoi(strNewVal.c_str()));
					result				= k2hdkc_full_cas32_set_wa(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, oldval, newval, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
				}else{	// CAS_TYPE_64 == type
					uint64_t	oldval	= static_cast<uint64_t>(atoi(strOldVal.c_str()));
					uint64_t	newval	= static_cast<uint64_t>(atoi(strNewVal.c_str()));
					result				= k2hdkc_full_cas64_set_wa(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, oldval, newval, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
				}
			}
			rescode = k2hdkc_get_lastres_code();

		}else{
			K2hdkcComCasSet*	pComObj = GetOtSlaveK2hdkcComCasSet(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return true;		// for continue.
			}
			// [NOTE] not set attributes
			if(CAS_TYPE_8 == type){
				uint8_t		oldval	= static_cast<uint8_t>(atoi(strOldVal.c_str()));
				uint8_t		newval	= static_cast<uint8_t>(atoi(strNewVal.c_str()));
				result				= pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, oldval, newval, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
			}else if(CAS_TYPE_16 == type){
				uint16_t	oldval	= static_cast<uint16_t>(atoi(strOldVal.c_str()));
				uint16_t	newval	= static_cast<uint16_t>(atoi(strNewVal.c_str()));
				result				= pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, oldval, newval, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
			}else if(CAS_TYPE_32 == type){
				uint32_t	oldval	= static_cast<uint32_t>(atoi(strOldVal.c_str()));
				uint32_t	newval	= static_cast<uint32_t>(atoi(strNewVal.c_str()));
				result				= pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, oldval, newval, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
			}else{	// CAS_TYPE_64 == type
				uint64_t	oldval	= static_cast<uint64_t>(atoi(strOldVal.c_str()));
				uint64_t	newval	= static_cast<uint64_t>(atoi(strNewVal.c_str()));
				result				= pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, oldval, newval, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
			}
			DKC_DELETE(pComObj);
		}

	}else{
		// use permanent chmpx object
		if(isModeCAPI){
			if(-1 == Expire && PassPhrase.empty()){
				if(CAS_TYPE_8 == type){
					uint8_t		oldval	= static_cast<uint8_t>(atoi(strOldVal.c_str()));
					uint8_t		newval	= static_cast<uint8_t>(atoi(strNewVal.c_str()));
					result				= k2hdkc_pm_cas8_set(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, oldval, newval);
				}else if(CAS_TYPE_16 == type){
					uint16_t	oldval	= static_cast<uint16_t>(atoi(strOldVal.c_str()));
					uint16_t	newval	= static_cast<uint16_t>(atoi(strNewVal.c_str()));
					result				= k2hdkc_pm_cas16_set(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, oldval, newval);
				}else if(CAS_TYPE_32 == type){
					uint32_t	oldval	= static_cast<uint32_t>(atoi(strOldVal.c_str()));
					uint32_t	newval	= static_cast<uint32_t>(atoi(strNewVal.c_str()));
					result				= k2hdkc_pm_cas32_set(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, oldval, newval);
				}else{	// CAS_TYPE_64 == type
					uint64_t	oldval	= static_cast<uint64_t>(atoi(strOldVal.c_str()));
					uint64_t	newval	= static_cast<uint64_t>(atoi(strNewVal.c_str()));
					result				= k2hdkc_pm_cas64_set(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, oldval, newval);
				}
			}else{
				if(CAS_TYPE_8 == type){
					uint8_t		oldval	= static_cast<uint8_t>(atoi(strOldVal.c_str()));
					uint8_t		newval	= static_cast<uint8_t>(atoi(strNewVal.c_str()));
					result				= k2hdkc_pm_cas8_set_wa(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, oldval, newval, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
				}else if(CAS_TYPE_16 == type){
					uint16_t	oldval	= static_cast<uint16_t>(atoi(strOldVal.c_str()));
					uint16_t	newval	= static_cast<uint16_t>(atoi(strNewVal.c_str()));
					result				= k2hdkc_pm_cas16_set_wa(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, oldval, newval, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
				}else if(CAS_TYPE_32 == type){
					uint32_t	oldval	= static_cast<uint32_t>(atoi(strOldVal.c_str()));
					uint32_t	newval	= static_cast<uint32_t>(atoi(strNewVal.c_str()));
					result				= k2hdkc_pm_cas32_set_wa(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, oldval, newval, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
				}else{	// CAS_TYPE_64 == type
					uint64_t	oldval	= static_cast<uint64_t>(atoi(strOldVal.c_str()));
					uint64_t	newval	= static_cast<uint64_t>(atoi(strNewVal.c_str()));
					result				= k2hdkc_pm_cas64_set_wa(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, oldval, newval, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
				}
			}
			rescode	= k2hdkc_get_res_code(chmpxhandle);

		}else{
			K2hdkcSlave*		pSlave	= reinterpret_cast<K2hdkcSlave*>(chmpxhandle);
			K2hdkcComCasSet*	pComObj = GetPmSlaveK2hdkcComCasSet(pSlave);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return true;		// for continue.
			}
			// [NOTE] not set attributes
			if(CAS_TYPE_8 == type){
				uint8_t		oldval	= static_cast<uint8_t>(atoi(strOldVal.c_str()));
				uint8_t		newval	= static_cast<uint8_t>(atoi(strNewVal.c_str()));
				result				= pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, oldval, newval, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
			}else if(CAS_TYPE_16 == type){
				uint16_t	oldval	= static_cast<uint16_t>(atoi(strOldVal.c_str()));
				uint16_t	newval	= static_cast<uint16_t>(atoi(strNewVal.c_str()));
				result				= pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, oldval, newval, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
			}else if(CAS_TYPE_32 == type){
				uint32_t	oldval	= static_cast<uint32_t>(atoi(strOldVal.c_str()));
				uint32_t	newval	= static_cast<uint32_t>(atoi(strNewVal.c_str()));
				result				= pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, oldval, newval, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
			}else{	// CAS_TYPE_64 == type
				uint64_t	oldval	= static_cast<uint64_t>(atoi(strOldVal.c_str()));
				uint64_t	newval	= static_cast<uint64_t>(atoi(strNewVal.c_str()));
				result				= pComObj->CommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, oldval, newval, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
			}
			DKC_DELETE(pComObj);
		}
	}

	// check result
	if(!result){
		ERR("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
		return true;		// for continue.
	}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
		MSG("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
	}

	return true;
}

//
// Sub increment/decrement command for cas: params	=> <key> [pass=....] [expire=sec]
//
static bool CasIncDecCommand(k2hdkc_chmpx_h chmpxhandle, bool is_increment, params_t& params)
{
	// check parameter
	if(params.size() < 1){
		ERR("Too few parameter for cas init command.");
		return true;	// for continue.
	}
	string	strKey		= params[0].c_str();
	time_t	Expire		= -1;
	string	PassPhrase("");
	for(size_t pos = 1; pos < params.size(); ++pos){
		if(0 == strncasecmp(params[pos].c_str(), "PASS=", 5)){
			PassPhrase = params[pos].substr(5);
		}else if(0 == strncasecmp(params[pos].c_str(), "EXPIRE=", 7)){
			Expire = static_cast<time_t>(atoi(params[pos].substr(7).c_str()));
		}else{
			ERR("Unknown parameter(%s) for cas increment/decrement command.", params[pos].c_str());
			return true;	// for continue.
		}
	}

	// do command
	bool			result;
	dkcres_type_t	rescode	= DKC_NORESTYPE;
	if(K2HDKC_INVALID_HANDLE == chmpxhandle){
		// use one-time chmpx object
		if(isModeCAPI){
			if(-1 == Expire && PassPhrase.empty()){
				if(is_increment){
					result = k2hdkc_full_cas_increment(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1);
				}else{
					result = k2hdkc_full_cas_decrement(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1);
				}
			}else{
				if(is_increment){
					result = k2hdkc_full_cas_increment_wa(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
				}else{
					result = k2hdkc_full_cas_decrement_wa(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
				}
			}
			rescode = k2hdkc_get_lastres_code();

		}else{
			K2hdkcComCasIncDec*	pComObj = GetOtSlaveK2hdkcComCasIncDec(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return true;		// for continue.
			}
			// [NOTE] not set attributes
			if(is_increment){
				result = pComObj->IncrementCommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
			}else{
				result = pComObj->DecrementCommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
			}
			DKC_DELETE(pComObj);
		}

	}else{
		// use permanent chmpx object
		if(isModeCAPI){
			if(-1 == Expire && PassPhrase.empty()){
				if(is_increment){
					result = k2hdkc_pm_cas_increment(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1);
				}else{
					result = k2hdkc_pm_cas_decrement(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1);
				}
			}else{
				if(is_increment){
					result = k2hdkc_pm_cas_increment_wa(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
				}else{
					result = k2hdkc_pm_cas_decrement_wa(chmpxhandle, reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire));
				}
			}
			rescode	= k2hdkc_get_res_code(chmpxhandle);

		}else{
			K2hdkcSlave*		pSlave	= reinterpret_cast<K2hdkcSlave*>(chmpxhandle);
			K2hdkcComCasIncDec*	pComObj = GetPmSlaveK2hdkcComCasIncDec(pSlave);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return true;		// for continue.
			}
			// [NOTE] not set attributes
			if(is_increment){
				result = pComObj->IncrementCommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
			}else{
				result = pComObj->DecrementCommandSend(reinterpret_cast<const unsigned char*>(strKey.c_str()), strKey.length() + 1, true, (PassPhrase.empty() ? NULL : PassPhrase.c_str()), (-1 == Expire ? NULL : &Expire), &rescode);
			}
			DKC_DELETE(pComObj);
		}
	}

	// check result
	if(!result){
		ERR("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
		return true;		// for continue.
	}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
		MSG("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
	}

	return true;
}

//
// Command Line:	cas[8 | 16 | 32 | 64] init <key> <value> [pass=....] [expire=sec]
//					cas[8 | 16 | 32 | 64] get <key> [pass=....]
//					cas[8 | 16 | 32 | 64] set <key> <old value> <new value> [pass=....] [expire=sec]
//					cas [increment(inc) | decrement(dec)] <key> [pass=....] [expire=sec]
//
static bool CasCommand(k2hdkc_chmpx_h chmpxhandle, CASTYPE type, params_t& params)
{
	// check parameter
	string	strSubCmd = params[0].c_str();
	params.erase(params.begin(), params.begin() + 1);
	if(CAS_TYPE_NO == type){
		type = CAS_TYPE_8;						// default 8 bit
	}

	// dispatch sub command
	//
	// [NOTICE]
	// Name does not include null character, because this tool should be as same as
	// k2hlinetool. It does not include null character for marker.
	//
	bool	result;
	if(0 == strcasecmp(strSubCmd.c_str(), "init")){
		result = CasInitCommand(chmpxhandle, type, params);
	}else if(0 == strcasecmp(strSubCmd.c_str(), "get")){
		result = CasGetCommand(chmpxhandle, type, params);
	}else if(0 == strcasecmp(strSubCmd.c_str(), "set")){
		result = CasSetCommand(chmpxhandle, type, params);
	}else if(0 == strcasecmp(strSubCmd.c_str(), "increment") || 0 == strcasecmp(strSubCmd.c_str(), "inc")){
		result = CasIncDecCommand(chmpxhandle, true, params);
	}else if(0 == strcasecmp(strSubCmd.c_str(), "decrement") || 0 == strcasecmp(strSubCmd.c_str(), "dec")){
		result = CasIncDecCommand(chmpxhandle, false, params);
	}else{
		ERR("Unknown parameter(%s) for cas command.", strSubCmd.c_str());
		return true;	// for continue.
	}

	if(!result){
		ERR("Something error occurred during cas %s command.", strSubCmd.c_str());
		return true;	// for continue.
	}
	return true;
}

//
// Sub chmpx command for status: params	=> [self] [full]
//
static bool StatusChmpxCommand(k2hdkc_chmpx_h chmpxhandle, params_t& params)
{
	// check parameter
	bool	dsp_self = false;
	bool	dsp_full = false;
	for(size_t pos = 0; pos < params.size(); ++pos){
		if(0 == strcasecmp(params[pos].c_str(), "self")){
			dsp_self = true;
		}else if(0 == strcasecmp(params[pos].c_str(), "full")){
			dsp_full = true;
		}else{
			ERR("Unknown parameter(%s) for status chmpx command.", params[pos].c_str());
			return true;	// for continue.
		}
	}

	// Initialize
	ChmCntrl	chmobj;
	if(!chmobj.OnlyAttachInitialize(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()))){
		ERR("Could not initialize(attach) chmpx shared memory.");
		return true;		// for continue.
	}

	// do(not command, but direct access api for chmpx)
	if(dsp_self){
		// display only self chmpx

		// get information
		PCHMPX	pInfo = chmobj.DupSelfChmpxInfo();
		if(!pInfo){
			ERR("Something error occurred in getting information.");
			chmobj.Clean();
			return true;	// for continue.
		}

		// print
		if(dsp_full){
			// full
			PRN("CHMPX SELF STATUS(FULL)");
			PRN("");
			PRN("chmpxid                    = 0x%016" PRIx64 ,	pInfo->chmpxid);
			PRN("hostname                   = %s",				pInfo->name);
			PRN("mode                       = %s",				STRCHMPXMODE(pInfo->mode));
			PRN("base hash                  = 0x%016" PRIx64 ,	pInfo->base_hash);
			PRN("pending hash               = 0x%016" PRIx64 ,	pInfo->pending_hash);
			PRN("port                       = %d",				pInfo->port);
			PRN("control port               = %d",				pInfo->ctlport);
			PRN("cuk                        = %s",				pInfo->cuk);
			PRN("custom id seed             = %s",				pInfo->custom_seed);
			PRN("endpoints                  = %s",				get_hostport_pairs_string(pInfo->endpoints, EXTERNAL_EP_MAX).c_str());
			PRN("control endpoints          = %s",				get_hostport_pairs_string(pInfo->ctlendpoints, EXTERNAL_EP_MAX).c_str());
			PRN("forward peers              = %s",				get_hostport_pairs_string(pInfo->forward_peers, FORWARD_PEER_MAX).c_str());
			PRN("reverse peers              = %s",				get_hostport_pairs_string(pInfo->reverse_peers, REVERSE_PEER_MAX).c_str());
			PRN("ssl                        = %s",				pInfo->is_ssl ? "yes" : "no");

			if(pInfo->is_ssl){
				PRN("{");
				PRN("  verify client peer       = %s",			pInfo->verify_peer ? "yes" : "no");
				PRN("  CA path is               = %s",			pInfo->is_ca_file ? "file" : "directory");
				PRN("  CA path                  = %s",			pInfo->capath);
				PRN("  server cert path         = %s",			pInfo->server_cert);
				PRN("  server private key path  = %s",			pInfo->server_prikey);
				PRN("  slave cert path          = %s",			pInfo->slave_cert);
				PRN("  slave private key path   = %s",			pInfo->slave_prikey);
				PRN("}");
			}

			string	socks;
			for(PCHMSOCKLIST psocklist = pInfo->socklist; psocklist; psocklist = psocklist->next){
				char	szBuff[32];
				sprintf(szBuff, "%d", psocklist->sock);
				if(!socks.empty()){
					socks += ",";
				}
				socks += szBuff;
			}
			PRN("sockets                    = %s",				socks.c_str());
			PRN("listen socket              = %d",				pInfo->selfsock);
			PRN("control socket             = %d",				pInfo->ctlsock);
			PRN("listen control socket      = %d",				pInfo->selfctlsock);
			PRN("last status update time    = %zu (unix time)",	pInfo->last_status_time);
			PRN("status                     = %s",				STR_CHMPXSTS_FULL(pInfo->status).c_str());
			PRN("");
		}else{
			// simple
			PRN("CHMPX SELF STATUS(SIMPLE)");
			PRN("");
			PRN("chmpxid                    = 0x%016" PRIx64 ,	pInfo->chmpxid);
			PRN("hostname                   = %s",				pInfo->name);
			PRN("mode                       = %s",				STRCHMPXMODE(pInfo->mode));
			PRN("port                       = %d",				pInfo->port);
			PRN("control port               = %d",				pInfo->ctlport);
			PRN("cuk                        = %s",				pInfo->cuk);
			PRN("custom id seed             = %s",				pInfo->custom_seed);
			PRN("ssl                        = %s",				pInfo->is_ssl ? "yes" : "no");
			PRN("last status update time    = %zu (unix time)",	pInfo->last_status_time);
			PRN("status                     = %s",				STR_CHMPXSTS_FULL(pInfo->status).c_str());
			PRN("");
		}
		ChmCntrl::FreeDupSelfChmpxInfo(pInfo);

	}else{
		// display all chmpx

		// Get information
		PCHMINFOEX	pInfo = chmobj.DupAllChmInfo();
		if(!pInfo){
			ERR("Something error occurred in getting information.");
			chmobj.Clean();
			return false;
		}
		if(!pInfo->pchminfo){
			ERR("Something error occurred in getting information.");
			ChmCntrl::FreeDupAllChmInfo(pInfo);
			chmobj.Clean();
			return false;
		}

		// print
		int				counter;
		PCHMPXLIST		pchmpxlist;
		PMQMSGHEADLIST	pmsglisttmp;
		PCLTPROCLIST	pproclist;
		if(dsp_full){
			// full
			PRN("ALL CHMPX STATUS(FULL)");
			PRN("");

			// chmpx info & info ex
			PRN("chmpx process id                             = %d",				pInfo->pchminfo->pid);
			PRN("process start time                           = %zu (unix time)",	pInfo->pchminfo->start_time);
			PRN("chmpx shared memory file path                = %s",				pInfo->shmpath);
			PRN("chmpx shared memory file size                = %zu (byte)",		pInfo->shmsize);
			PRN("k2hash file path                             = %s",				pInfo->k2hashpath);
			PRN("k2hash full mapping                          = %s",				pInfo->pchminfo->k2h_fullmap ? "yes" : "no");
			PRN("k2hash mask bit count                        = %d",				pInfo->pchminfo->k2h_mask_bitcnt);
			PRN("k2hash collision mask bit count              = %d",				pInfo->pchminfo->k2h_cmask_bitcnt);
			PRN("k2hash maximum element count                 = %d",				pInfo->pchminfo->k2h_max_element);
			PRN("random mode                                  = %s",				pInfo->pchminfo->is_random_deliver ? "yes" : "no");
			PRN("communication history count                  = %ld",				pInfo->pchminfo->histlog_count);
			PRN("auto merge                                   = %s",				pInfo->pchminfo->is_auto_merge ? "yes" : "no");
			PRN("merge processing(do merge)                   = %s",				pInfo->pchminfo->is_do_merge ? "yes" :"no");
			PRN("timeout for merge                            = %zd (s)",			pInfo->pchminfo->timeout_merge);
			PRN("thread count for socket                      = %d",				pInfo->pchminfo->evsock_thread_cnt);
			PRN("thread count for MQ                          = %d",				pInfo->pchminfo->evmq_thread_cnt);
			PRN("maximum socket count per chmpx(socket pool)  = %d",				pInfo->pchminfo->max_sock_pool);
			PRN("timeout for socket pool                      = %zd (s)",			pInfo->pchminfo->sock_pool_timeout);
			PRN("retry count on socket                        = %d",				pInfo->pchminfo->sock_retrycnt);
			PRN("timeout for send/receive on socket           = %ld (us)",			pInfo->pchminfo->timeout_wait_socket);
			PRN("timeout for connect on socket                = %ld (us)",			pInfo->pchminfo->timeout_wait_connect);
			PRN("timeout for send/receive on socket           = %ld (us)",			pInfo->pchminfo->timeout_wait_mq);
			PRN("maximum MQ count                             = %ld",				pInfo->pchminfo->max_mqueue);
			PRN("chmpx process MQ count                       = %ld",				pInfo->pchminfo->chmpx_mqueue);
			PRN("maximum queue per chmpx process MQ           = %ld",				pInfo->pchminfo->max_q_per_chmpxmq);
			PRN("maximum queue per client process MQ          = %ld",				pInfo->pchminfo->max_q_per_cltmq);
			PRN("maximum MQ per client process                = %ld",				pInfo->pchminfo->max_mq_per_client);
			PRN("maximum MQ per attach                        = %ld",				pInfo->pchminfo->mq_per_attach);
			PRN("using MQ ACK                                 = %s",				pInfo->pchminfo->mq_ack ? "yes" : "no");
			PRN("retry count on MQ                            = %d",				pInfo->pchminfo->mq_retrycnt);
			PRN("base msgid                                   = 0x%016" PRIx64 ,	pInfo->pchminfo->base_msgid);
			PRN("");

			// chmpx info & info ex --> chmpx manager
			PRN("chmpx name                                   = %s",				pInfo->pchminfo->chmpx_man.group);
			PRN("configuration file version                   = %ld",				pInfo->pchminfo->chmpx_man.cfg_revision);
			PRN("configuration file date                      = %zu (unix time)",	pInfo->pchminfo->chmpx_man.cfg_date);
			PRN("replication count                            = %ld",				pInfo->pchminfo->chmpx_man.replica_count);
			PRN("maximum chmpx count                          = %ld",				pInfo->pchminfo->chmpx_man.chmpx_count);
			PRN("base hash count                              = %ld",				pInfo->pchminfo->chmpx_man.chmpx_bhash_count);
			PRN("now operating                                = %s",				pInfo->pchminfo->chmpx_man.is_operating ? "yes" : "no");
			PRN("last using random chmpxid                    = 0x%016" PRIx64 ,	pInfo->pchminfo->chmpx_man.last_chmpxid);

			// chmpx info & info ex --> chmpx manager --> self chmpx
			if(pInfo->pchminfo->chmpx_man.chmpx_self){
				PRN("self chmpx = {");
				PRN("  chmpxid                                    = 0x%016" PRIx64 ,pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.chmpxid);
				PRN("  hostname                                   = %s",			pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.name);
				PRN("  mode                                       = %s",			STRCHMPXMODE(pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.mode));
				PRN("  base hash                                  = 0x%016" PRIx64 ,pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.base_hash);
				PRN("  pending hash                               = 0x%016" PRIx64 ,pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.pending_hash);
				PRN("  port                                       = %d",			pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.port);
				PRN("  control port                               = %d",			pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.ctlport);
				PRN("  cuk                                        = %s",			pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.cuk);
				PRN("  custom id seed                             = %s",			pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.custom_seed);
				PRN("  endpoints                                  = %s",			get_hostport_pairs_string(pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.endpoints, EXTERNAL_EP_MAX).c_str());
				PRN("  control endpoints                          = %s",			get_hostport_pairs_string(pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.ctlendpoints, EXTERNAL_EP_MAX).c_str());
				PRN("  forward peers                              = %s",			get_hostport_pairs_string(pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.forward_peers, FORWARD_PEER_MAX).c_str());
				PRN("  reverse peers                              = %s",			get_hostport_pairs_string(pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.reverse_peers, REVERSE_PEER_MAX).c_str());
				PRN("  ssl                                        = %s",			pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.is_ssl ? "yes" : "no");

				if(pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.is_ssl){
					PRN("  verify client peer                         = %s",		pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.verify_peer ? "yes" : "no");
					PRN("  CA path is                                 = %s",		pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.is_ca_file ? "file" : "directory");
					PRN("  CA path                                    = %s",		pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.capath);
					PRN("  server cert path                           = %s",		pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.server_cert);
					PRN("  server private key path                    = %s",		pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.server_prikey);
					PRN("  slave cert path                            = %s",		pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.slave_cert);
					PRN("  slave private key path                     = %s",		pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.slave_prikey);
				}

				string	socks;
				for(PCHMSOCKLIST psocklist = pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.socklist; psocklist; psocklist = psocklist->next){
					char	szBuff[32];
					sprintf(szBuff, "%d", psocklist->sock);
					if(!socks.empty()){
						socks += ",";
					}
					socks += szBuff;
				}
				PRN("  sockets                                    = %s",			socks.c_str());
				PRN("  listen socket                              = %d",			pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.selfsock);
				PRN("  control socket                             = %d",			pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.ctlsock);
				PRN("  listen control socket                      = %d",			pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.selfctlsock);
				PRN("  last status update time                    = %zu (unix time)",	pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.last_status_time);
				PRN("  status                                     = %s",			STR_CHMPXSTS_FULL(pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.status).c_str());
				PRN("}");
			}

			// chmpx info & info ex --> chmpx manager --> server chmpxs
			PRN("server chmpxs [ %ld ] = {",										pInfo->pchminfo->chmpx_man.chmpx_server_count);
			for(counter = 0, pchmpxlist = pInfo->pchminfo->chmpx_man.chmpx_servers; pchmpxlist; pchmpxlist = pchmpxlist->next, ++counter){
				PRN("  [%d] = {",													counter);
				PRN("    chmpxid                                  = 0x%016" PRIx64 ,pchmpxlist->chmpx.chmpxid);
				PRN("    hostname                                 = %s",			pchmpxlist->chmpx.name);
				PRN("    mode                                     = %s",			STRCHMPXMODE(pchmpxlist->chmpx.mode));
				PRN("    base hash                                = 0x%016" PRIx64 ,pchmpxlist->chmpx.base_hash);
				PRN("    pending hash                             = 0x%016" PRIx64 ,pchmpxlist->chmpx.pending_hash);
				PRN("    port                                     = %d",			pchmpxlist->chmpx.port);
				PRN("    control port                             = %d",			pchmpxlist->chmpx.ctlport);
				PRN("    cuk                                      = %s",			pchmpxlist->chmpx.cuk);
				PRN("    custom id seed                           = %s",			pchmpxlist->chmpx.custom_seed);
				PRN("    endpoints                                = %s",			get_hostport_pairs_string(pchmpxlist->chmpx.endpoints, EXTERNAL_EP_MAX).c_str());
				PRN("    control endpoints                        = %s",			get_hostport_pairs_string(pchmpxlist->chmpx.ctlendpoints, EXTERNAL_EP_MAX).c_str());
				PRN("    forward peers                            = %s",			get_hostport_pairs_string(pchmpxlist->chmpx.forward_peers, FORWARD_PEER_MAX).c_str());
				PRN("    reverse peers                            = %s",			get_hostport_pairs_string(pchmpxlist->chmpx.reverse_peers, REVERSE_PEER_MAX).c_str());
				PRN("    ssl                                      = %s",			pchmpxlist->chmpx.is_ssl ? "yes" : "no");

				if(pchmpxlist->chmpx.is_ssl){
					PRN("    verify client peer                       = %s",		pchmpxlist->chmpx.verify_peer ? "yes" : "no");
					PRN("    CA path is                               = %s",		pchmpxlist->chmpx.is_ca_file ? "file" : "directory");
					PRN("    CA path                                  = %s",		pchmpxlist->chmpx.capath);
					PRN("    server cert path                         = %s",		pchmpxlist->chmpx.server_cert);
					PRN("    server private key path                  = %s",		pchmpxlist->chmpx.server_prikey);
					PRN("    slave cert path                          = %s",		pchmpxlist->chmpx.slave_cert);
					PRN("    slave private key path                   = %s",		pchmpxlist->chmpx.slave_prikey);
				}

				string	socks;
				for(PCHMSOCKLIST psocklist = pchmpxlist->chmpx.socklist; psocklist; psocklist = psocklist->next){
					char	szBuff[32];
					sprintf(szBuff, "%d", psocklist->sock);
					if(!socks.empty()){
						socks += ",";
					}
					socks += szBuff;
				}
				PRN("    sockets                                  = %s",			socks.c_str());
				PRN("    listen socket                            = %d",			pchmpxlist->chmpx.selfsock);
				PRN("    control socket                           = %d",			pchmpxlist->chmpx.ctlsock);
				PRN("    listen control socket                    = %d",			pchmpxlist->chmpx.selfctlsock);
				PRN("    last status update time                  = %zu (unix time)",	pchmpxlist->chmpx.last_status_time);
				PRN("    status                                   = %s",			STR_CHMPXSTS_FULL(pchmpxlist->chmpx.status).c_str());
				PRN("  }");
			}
			PRN("}");

			// chmpx info & info ex --> chmpx manager --> slave chmpxs
			PRN("slave chmpxs [ %ld ] = {",											pInfo->pchminfo->chmpx_man.chmpx_slave_count);
			for(counter = 0, pchmpxlist = pInfo->pchminfo->chmpx_man.chmpx_slaves; pchmpxlist; pchmpxlist = pchmpxlist->next, ++counter){
				PRN("  [%d] = {",													counter);
				PRN("    chmpxid                                  = 0x%016" PRIx64 ,pchmpxlist->chmpx.chmpxid);
				PRN("    hostname                                 = %s",			pchmpxlist->chmpx.name);
				PRN("    mode                                     = %s",			STRCHMPXMODE(pchmpxlist->chmpx.mode));
				PRN("    base hash                                = 0x%016" PRIx64 ,pchmpxlist->chmpx.base_hash);
				PRN("    pending hash                             = 0x%016" PRIx64 ,pchmpxlist->chmpx.pending_hash);
				PRN("    port                                     = %d",			pchmpxlist->chmpx.port);
				PRN("    control port                             = %d",			pchmpxlist->chmpx.ctlport);
				PRN("    cuk                                      = %s",			pchmpxlist->chmpx.cuk);
				PRN("    custom id seed                           = %s",			pchmpxlist->chmpx.custom_seed);
				PRN("    endpoints                                = %s",			get_hostport_pairs_string(pchmpxlist->chmpx.endpoints, EXTERNAL_EP_MAX).c_str());
				PRN("    control endpoints                        = %s",			get_hostport_pairs_string(pchmpxlist->chmpx.ctlendpoints, EXTERNAL_EP_MAX).c_str());
				PRN("    forward peers                            = %s",			get_hostport_pairs_string(pchmpxlist->chmpx.forward_peers, FORWARD_PEER_MAX).c_str());
				PRN("    reverse peers                            = %s",			get_hostport_pairs_string(pchmpxlist->chmpx.reverse_peers, REVERSE_PEER_MAX).c_str());
				PRN("    ssl                                      = %s",			pchmpxlist->chmpx.is_ssl ? "yes" : "no");

				if(pchmpxlist->chmpx.is_ssl){
					PRN("    verify client peer                       = %s",		pchmpxlist->chmpx.verify_peer ? "yes" : "no");
					PRN("    CA path is                               = %s",		pchmpxlist->chmpx.is_ca_file ? "file" : "directory");
					PRN("    CA path                                  = %s",		pchmpxlist->chmpx.capath);
					PRN("    server cert path                         = %s",		pchmpxlist->chmpx.server_cert);
					PRN("    server private key path                  = %s",		pchmpxlist->chmpx.server_prikey);
					PRN("    slave cert path                          = %s",		pchmpxlist->chmpx.slave_cert);
					PRN("    slave private key path                   = %s",		pchmpxlist->chmpx.slave_prikey);
				}

				string	socks;
				for(PCHMSOCKLIST psocklist = pchmpxlist->chmpx.socklist; psocklist; psocklist = psocklist->next){
					char	szBuff[32];
					sprintf(szBuff, "%d", psocklist->sock);
					if(!socks.empty()){
						socks += ",";
					}
					socks += szBuff;
				}
				PRN("    sockets                                  = %s",			socks.c_str());
				PRN("    listen socket                            = %d",			pchmpxlist->chmpx.selfsock);
				PRN("    control socket                           = %d",			pchmpxlist->chmpx.ctlsock);
				PRN("    listen control socket                    = %d",			pchmpxlist->chmpx.selfctlsock);
				PRN("    last status update time                  = %zu (unix time)",	pchmpxlist->chmpx.last_status_time);
				PRN("    status                                   = %s",			STR_CHMPXSTS_FULL(pchmpxlist->chmpx.status).c_str());
				PRN("  }");
			}
			PRN("}");

			// chmpx info & info ex --> chmpx manager --> stat
			PRN("server stat = {");
			PRN("    total send message count                 = %ld",				pInfo->pchminfo->chmpx_man.server_stat.total_sent_count);
			PRN("    total receive message count              = %ld",				pInfo->pchminfo->chmpx_man.server_stat.total_received_count);
			PRN("    total body size                          = %zu (bytes)",		pInfo->pchminfo->chmpx_man.server_stat.total_body_bytes);
			PRN("    minimum body size                        = %zu (bytes)",		pInfo->pchminfo->chmpx_man.server_stat.min_body_bytes);
			PRN("    maximum body size                        = %zu (bytes)",		pInfo->pchminfo->chmpx_man.server_stat.max_body_bytes);
			PRN("    total elapsed time                       = %s",				PRN_TIMESPEC(pInfo->pchminfo->chmpx_man.server_stat.total_elapsed_time).c_str());
			PRN("    minimum elapsed time                     = %s",				PRN_TIMESPEC(pInfo->pchminfo->chmpx_man.server_stat.min_elapsed_time).c_str());
			PRN("    maximum elapsed time                     = %s",				PRN_TIMESPEC(pInfo->pchminfo->chmpx_man.server_stat.max_elapsed_time).c_str());
			PRN("}");

			PRN("slave stat = {");
			PRN("    total send message count                 = %ld",				pInfo->pchminfo->chmpx_man.slave_stat.total_sent_count);
			PRN("    total receive message count              = %ld",				pInfo->pchminfo->chmpx_man.slave_stat.total_received_count);
			PRN("    total body size                          = %zu (bytes)",		pInfo->pchminfo->chmpx_man.slave_stat.total_body_bytes);
			PRN("    minimum body size                        = %zu (bytes)",		pInfo->pchminfo->chmpx_man.slave_stat.min_body_bytes);
			PRN("    maximum body size                        = %zu (bytes)",		pInfo->pchminfo->chmpx_man.slave_stat.max_body_bytes);
			PRN("    total elapsed time                       = %s",				PRN_TIMESPEC(pInfo->pchminfo->chmpx_man.slave_stat.total_elapsed_time).c_str());
			PRN("    minimum elapsed time                     = %s",				PRN_TIMESPEC(pInfo->pchminfo->chmpx_man.slave_stat.min_elapsed_time).c_str());
			PRN("    maximum elapsed time                     = %s",				PRN_TIMESPEC(pInfo->pchminfo->chmpx_man.slave_stat.max_elapsed_time).c_str());
			PRN("}");

			// chmpx info & info ex --> chmpx manager --> free chmpx/sock
			PRN("free chmpx area count                        = %ld",				pInfo->pchminfo->chmpx_man.chmpx_free_count);
			PRN("free sock area count                         = %ld",				pInfo->pchminfo->chmpx_man.sock_free_count);
			PRN("");

			// chmpx info & info ex --> mq list
			PRN("chmpx using MQ [ %ld ] = {",										pInfo->pchminfo->chmpx_msg_count);
			for(counter = 0, pmsglisttmp = pInfo->pchminfo->chmpx_msgs; pmsglisttmp; pmsglisttmp = pmsglisttmp->next, ++counter){
				PRN("  [%d] = {",													counter);
				PRN("    msgid                                    = 0x%016" PRIx64 ,pmsglisttmp->msghead.msgid);
				PRN("    flag                                     = %s%s%s",		STR_MQFLAG_ASSIGNED(pmsglisttmp->msghead.flag), STR_MQFLAG_KIND(pmsglisttmp->msghead.flag), STR_MQFLAG_ACTIVATED(pmsglisttmp->msghead.flag));
				PRN("    pid                                      = %d",			pmsglisttmp->msghead.pid);
				PRN("  }");
			}
			PRN("}");

			// chmpx info & info ex --> mq list
			PRN("client process using MQ [ %ld ] = {",								pInfo->pchminfo->activated_msg_count);
			for(counter = 0, pmsglisttmp = pInfo->pchminfo->activated_msgs; pmsglisttmp; pmsglisttmp = pmsglisttmp->next, ++counter){
				PRN("  [%d] = {",													counter);
				PRN("    msgid                                    = 0x%016" PRIx64 ,pmsglisttmp->msghead.msgid);
				PRN("    flag                                     = %s%s%s",		STR_MQFLAG_ASSIGNED(pmsglisttmp->msghead.flag), STR_MQFLAG_KIND(pmsglisttmp->msghead.flag), STR_MQFLAG_ACTIVATED(pmsglisttmp->msghead.flag));
				PRN("    pid                                      = %d",			pmsglisttmp->msghead.pid);
				PRN("  }");
			}
			PRN("}");

			// chmpx info & info ex --> mq list
			PRN("assigned MQ [ %ld ] = {",											pInfo->pchminfo->assigned_msg_count);
			for(counter = 0, pmsglisttmp = pInfo->pchminfo->assigned_msgs; pmsglisttmp; pmsglisttmp = pmsglisttmp->next, ++counter){
				PRN("  [%d] = {",													counter);
				PRN("    msgid                                    = 0x%016" PRIx64 ,pmsglisttmp->msghead.msgid);
				PRN("    flag                                     = %s%s%s",		STR_MQFLAG_ASSIGNED(pmsglisttmp->msghead.flag), STR_MQFLAG_KIND(pmsglisttmp->msghead.flag), STR_MQFLAG_ACTIVATED(pmsglisttmp->msghead.flag));
				PRN("    pid                                      = %d",			pmsglisttmp->msghead.pid);
				PRN("  }");
			}
			PRN("}");

			// chmpx info & info ex
			PRN("free MQ                                      = %ld",				pInfo->pchminfo->free_msg_count);
			PRN("last random msgid used by chmpx process      = 0x%016" PRIx64 ,	pInfo->pchminfo->last_msgid_chmpx);
			PRN("last activated msgid                         = 0x%016" PRIx64 ,	pInfo->pchminfo->last_msgid_activated);
			PRN("last assigned msgid                          = 0x%016" PRIx64 ,	pInfo->pchminfo->last_msgid_assigned);

			// chmpx info & info ex --> client process
			PRN("joining client process = {");
			for(counter = 0, pproclist = pInfo->pchminfo->client_pids; pproclist; pproclist = pproclist->next, ++counter){
				PRN("  [%d] pid                                   = %d",			counter, pproclist->pid);
			}
			PRN("}");
			PRN("");

		}else{
			// simple
			PRN("ALL CHMPX STATUS(SIMPLE)");
			PRN("");

			// chmpx info & info ex
			PRN("chmpx process id                             = %d",				pInfo->pchminfo->pid);
			PRN("random mode                                  = %s",				pInfo->pchminfo->is_random_deliver ? "yes" : "no");
			PRN("chmpx name                                   = %s",				pInfo->pchminfo->chmpx_man.group);

			// chmpx info & info ex --> chmpx manager --> self chmpx
			if(pInfo->pchminfo->chmpx_man.chmpx_self){
				PRN("self chmpx = {");
				PRN("  chmpxid                                    = 0x%016" PRIx64 ,pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.chmpxid);
				PRN("  hostname                                   = %s",			pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.name);
				PRN("  mode                                       = %s",			STRCHMPXMODE(pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.mode));
				PRN("  port                                       = %d",			pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.port);
				PRN("  control port                               = %d",			pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.ctlport);
				PRN("  cuk                                        = %s",			pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.cuk);
				PRN("  custom id seed                             = %s",			pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.custom_seed);
				PRN("  endpoints                                  = %s",			get_hostport_pairs_string(pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.endpoints, EXTERNAL_EP_MAX).c_str());
				PRN("  control endpoints                          = %s",			get_hostport_pairs_string(pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.ctlendpoints, EXTERNAL_EP_MAX).c_str());
				PRN("  forward peers                              = %s",			get_hostport_pairs_string(pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.forward_peers, FORWARD_PEER_MAX).c_str());
				PRN("  reverse peers                              = %s",			get_hostport_pairs_string(pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.reverse_peers, REVERSE_PEER_MAX).c_str());
				PRN("  ssl                                        = %s",			pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.is_ssl ? "yes" : "no");
				PRN("  last status update time                    = %zu (unix time)",	pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.last_status_time);
				PRN("  status                                     = %s",			STR_CHMPXSTS_FULL(pInfo->pchminfo->chmpx_man.chmpx_self->chmpx.status).c_str());
				PRN("}");
			}

			// chmpx info & info ex --> chmpx manager --> server chmpxs
			PRN("server chmpxs [ %ld ] = {",										pInfo->pchminfo->chmpx_man.chmpx_server_count);
			for(counter = 0, pchmpxlist = pInfo->pchminfo->chmpx_man.chmpx_servers; pchmpxlist; pchmpxlist = pchmpxlist->next, ++counter){
				PRN("  [%d] = {",													counter);
				PRN("    chmpxid                                  = 0x%016" PRIx64 ,pchmpxlist->chmpx.chmpxid);
				PRN("    hostname                                 = %s",			pchmpxlist->chmpx.name);
				PRN("    mode                                     = %s",			STRCHMPXMODE(pchmpxlist->chmpx.mode));
				PRN("    port                                     = %d",			pchmpxlist->chmpx.port);
				PRN("    control port                             = %d",			pchmpxlist->chmpx.ctlport);
				PRN("    cuk                                      = %s",			pchmpxlist->chmpx.cuk);
				PRN("    custom id seed                           = %s",			pchmpxlist->chmpx.custom_seed);
				PRN("    endpoints                                = %s",			get_hostport_pairs_string(pchmpxlist->chmpx.endpoints, EXTERNAL_EP_MAX).c_str());
				PRN("    control endpoints                        = %s",			get_hostport_pairs_string(pchmpxlist->chmpx.ctlendpoints, EXTERNAL_EP_MAX).c_str());
				PRN("    forward peers                            = %s",			get_hostport_pairs_string(pchmpxlist->chmpx.forward_peers, FORWARD_PEER_MAX).c_str());
				PRN("    reverse peers                            = %s",			get_hostport_pairs_string(pchmpxlist->chmpx.reverse_peers, REVERSE_PEER_MAX).c_str());
				PRN("    ssl                                      = %s",			pchmpxlist->chmpx.is_ssl ? "yes" : "no");
				PRN("    last status update time                  = %zu (unix time)",	pchmpxlist->chmpx.last_status_time);
				PRN("    status                                   = %s",			STR_CHMPXSTS_FULL(pchmpxlist->chmpx.status).c_str());
				PRN("  }");
			}
			PRN("}");

			// chmpx info & info ex --> chmpx manager --> slave chmpxs
			PRN("slave chmpxs [ %ld ] = {",											pInfo->pchminfo->chmpx_man.chmpx_slave_count);
			for(counter = 0, pchmpxlist = pInfo->pchminfo->chmpx_man.chmpx_slaves; pchmpxlist; pchmpxlist = pchmpxlist->next, ++counter){
				PRN("  [%d] = {",													counter);
				PRN("    chmpxid                                  = 0x%016" PRIx64 ,pchmpxlist->chmpx.chmpxid);
				PRN("    hostname                                 = %s",			pchmpxlist->chmpx.name);
				PRN("    mode                                     = %s",			STRCHMPXMODE(pchmpxlist->chmpx.mode));
				PRN("    control port                             = %d",			pchmpxlist->chmpx.ctlport);
				PRN("    cuk                                      = %s",			pchmpxlist->chmpx.cuk);
				PRN("    custom id seed                           = %s",			pchmpxlist->chmpx.custom_seed);
				PRN("    endpoints                                = %s",			get_hostport_pairs_string(pchmpxlist->chmpx.endpoints, EXTERNAL_EP_MAX).c_str());
				PRN("    control endpoints                        = %s",			get_hostport_pairs_string(pchmpxlist->chmpx.ctlendpoints, EXTERNAL_EP_MAX).c_str());
				PRN("    forward peers                            = %s",			get_hostport_pairs_string(pchmpxlist->chmpx.forward_peers, FORWARD_PEER_MAX).c_str());
				PRN("    reverse peers                            = %s",			get_hostport_pairs_string(pchmpxlist->chmpx.reverse_peers, REVERSE_PEER_MAX).c_str());
				PRN("    ssl                                      = %s",			pchmpxlist->chmpx.is_ssl ? "yes" : "no");
				PRN("    last status update time                  = %zu (unix time)",	pchmpxlist->chmpx.last_status_time);
				PRN("    status                                   = %s",			STR_CHMPXSTS_FULL(pchmpxlist->chmpx.status).c_str());
				PRN("  }");
			}
			PRN("}");

			PRN("chmpx using MQ                               = %ld",				pInfo->pchminfo->chmpx_msg_count);
			PRN("client process using MQ                      = %ld",				pInfo->pchminfo->activated_msg_count);
			PRN("assigned MQ                                  = %ld",				pInfo->pchminfo->assigned_msg_count);
			PRN("");
		}
		ChmCntrl::FreeDupAllChmInfo(pInfo);
	}
	chmobj.Clean();

	return true;
}

//
// Sub chmpx command for status: params	=> [full]
//
static bool StatusNodeCommand(k2hdkc_chmpx_h chmpxhandle, params_t& params)
{
	// check parameter
	bool	dsp_full = false;
	for(size_t pos = 0; pos < params.size(); ++pos){
		if(0 == strcasecmp(params[pos].c_str(), "full")){
			dsp_full = true;
		}else{
			ERR("Unknown parameter(%s) for status node command.", params[pos].c_str());
			return true;	// for continue.
		}
	}

	// do command
	bool			result;
	dkcres_type_t	rescode		= DKC_NORESTYPE;
	PDKC_NODESTATE	pstates		= NULL;
	size_t			statecount	= 0;
	if(K2HDKC_INVALID_HANDLE == chmpxhandle){
		// use one-time chmpx object
		if(isModeCAPI){
			result	= k2hdkc_full_get_state(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, &pstates, &statecount);
			rescode	= k2hdkc_get_lastres_code();

		}else{
			K2hdkcComState*	pComObj = GetOtSlaveK2hdkcComState(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return true;		// for continue.
			}
			const DKC_NODESTATE*	ptmpstates		= NULL;
			size_t					tmpstatecount	= 0UL;
			if(true == (result = pComObj->CommandSend(&ptmpstates, &tmpstatecount, &rescode)) && ptmpstates && 0 < tmpstatecount){
				// copy to new allocated area
				if(NULL == (pstates = reinterpret_cast<PDKC_NODESTATE>(calloc(tmpstatecount, sizeof(DKC_NODESTATE))))){
					ERR_DKCPRN("Could not allocate memory.");
					DKC_DELETE(pComObj);
					return true;	// for continue.
				}
				for(size_t cnt = 0; cnt < tmpstatecount; ++cnt){
					memcpy(&(pstates[cnt]), &(ptmpstates[cnt]), sizeof(DKC_NODESTATE));
				}
				statecount = tmpstatecount;
			}
			DKC_DELETE(pComObj);
		}

	}else{
		// use permanent chmpx object
		if(isModeCAPI){
			result	= k2hdkc_pm_get_state(chmpxhandle, &pstates, &statecount);
			rescode	= k2hdkc_get_res_code(chmpxhandle);

		}else{
			K2hdkcSlave*	pSlave	= reinterpret_cast<K2hdkcSlave*>(chmpxhandle);
			K2hdkcComState*	pComObj = GetPmSlaveK2hdkcComState(pSlave);
			if(!pComObj){
				ERR("Something internal error occurred. could not make command object.");
				return true;		// for continue.
			}
			const DKC_NODESTATE*	ptmpstates		= NULL;
			size_t					tmpstatecount	= 0UL;
			if(true == (result = pComObj->CommandSend(&ptmpstates, &tmpstatecount, &rescode)) && ptmpstates && 0 < tmpstatecount){
				// copy to new allocated area
				if(NULL == (pstates = reinterpret_cast<PDKC_NODESTATE>(calloc(tmpstatecount, sizeof(DKC_NODESTATE))))){
					ERR_DKCPRN("Could not allocate memory.");
					DKC_DELETE(pComObj);
					return true;	// for continue.
				}
				for(size_t cnt = 0; cnt < tmpstatecount; ++cnt){
					memcpy(&(pstates[cnt]), &(ptmpstates[cnt]), sizeof(DKC_NODESTATE));
				}
				statecount = tmpstatecount;
			}
			DKC_DELETE(pComObj);
		}
	}
	// check result
	if(!result){
		ERR("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
		DKC_FREE(pstates);
		return true;		// for continue.
	}else if(DKC_RES_SUBCODE_NOTHING != GET_DKC_RES_SUBCODE(rescode)){
		MSG("RESULT CODE(%s - %s)", STR_DKCRES_RESULT_TYPE(rescode), STR_DKCRES_SUBCODE_TYPE(rescode));
	}

	// print result
	PRN("K2HDKC server node count                       = %zu", statecount);
	for(size_t cnt = 0; cnt < statecount; ++cnt){
		if(dsp_full){
			// dump
			PRN("server node [%zu] = {",											cnt);
			PRN("  chmpxid                                      = 0x%016" PRIx64 , 	pstates[cnt].chmpxid);
			PRN("  server name                                  = %s", 				pstates[cnt].name);
			PRN("  base hash value                              = 0x%016" PRIx64 , 	pstates[cnt].base_hash);
			PRN("  pending hash value                           = 0x%016" PRIx64 , 	pstates[cnt].pending_hash);
			PRN("  k2hash state = {");
			PRN("    version                                    = %s", 				pstates[cnt].k2hstate.version);
			PRN("    hash function                              = %s", 				pstates[cnt].k2hstate.hash_version);
			PRN("    transaction function                       = %s", 				pstates[cnt].k2hstate.trans_version);
			PRN("    transaction pool count                     = %d", 				pstates[cnt].k2hstate.trans_pool_count);
			PRN("    maximum mask bit count                     = 0x%016" PRIx64 , 	pstates[cnt].k2hstate.max_mask);
			PRN("    minimum mask bit count                     = 0x%016" PRIx64 , 	pstates[cnt].k2hstate.min_mask);
			PRN("    current mask bit count                     = 0x%016" PRIx64 , 	pstates[cnt].k2hstate.cur_mask);
			PRN("    collision mask bit count                   = 0x%016" PRIx64 , 	pstates[cnt].k2hstate.collision_mask);
			PRN("    maximum element count                      = %ld", 			pstates[cnt].k2hstate.max_element_count);
			PRN("    total k2hash size                          = %zu (byte)", 		pstates[cnt].k2hstate.total_size);
			PRN("    k2hash file size                           = %zu (byte)", 		pstates[cnt].k2hstate.file_size);
			PRN("    paging size                                = %zu (byte)", 		pstates[cnt].k2hstate.page_size);
			PRN("    total used size                            = %zu (byte)", 		pstates[cnt].k2hstate.total_used_size);
			PRN("    total mapping size                         = %zu (byte)", 		pstates[cnt].k2hstate.total_map_size);
			PRN("    total element area size                    = %zu (byte)", 		pstates[cnt].k2hstate.total_element_size);
			PRN("    total page area size                       = %zu (byte)", 		pstates[cnt].k2hstate.total_page_size);
			PRN("    total area count                           = %ld", 			pstates[cnt].k2hstate.total_area_count);
			PRN("    total element count                        = %ld", 			pstates[cnt].k2hstate.total_element_count);
			PRN("    total page count                           = %ld", 			pstates[cnt].k2hstate.total_page_count);
			PRN("    assigned area count (%)                    = %ld (%ld%%)", 	pstates[cnt].k2hstate.assigned_area_count,		((pstates[cnt].k2hstate.assigned_area_count * 100) / pstates[cnt].k2hstate.total_area_count));
			PRN("    assigned key count                         = %ld", 			pstates[cnt].k2hstate.assigned_key_count);
			PRN("    assigned collision key count               = %ld", 			pstates[cnt].k2hstate.assigned_ckey_count);
			PRN("    assigned element count (%)                 = %ld (%ld%%)", 	pstates[cnt].k2hstate.assigned_element_count,	((pstates[cnt].k2hstate.assigned_element_count * 100) / pstates[cnt].k2hstate.total_element_count));
			PRN("    assigned page count (%)                    = %ld (%ld%%)", 	pstates[cnt].k2hstate.assigned_page_count,		((pstates[cnt].k2hstate.assigned_page_count * 100) / pstates[cnt].k2hstate.total_page_count));
			PRN("    free element count (%)                     = %ld (%ld%%)", 	pstates[cnt].k2hstate.unassigned_element_count,	((pstates[cnt].k2hstate.unassigned_element_count * 100) / pstates[cnt].k2hstate.total_element_count));
			PRN("    free page count(%)                         = %ld (%ld%%)", 	pstates[cnt].k2hstate.unassigned_page_count,	((pstates[cnt].k2hstate.unassigned_page_count * 100) / pstates[cnt].k2hstate.total_page_count));
			PRN("    last update                                = %jds %jdusec", 	pstates[cnt].k2hstate.last_update.tv_sec,		pstates[cnt].k2hstate.last_update.tv_usec);
			PRN("    last area update                           = %jds %jdusec", 	pstates[cnt].k2hstate.last_area_update.tv_sec,	pstates[cnt].k2hstate.last_area_update.tv_usec);
			PRN("  }");
			PRN("}");
		}else{
			// make buffer
			char	szName[256];
			char	szAreaRaito[6];
			char	szElemRaito[6];
			char	szPageRaito[6];
			{
				memset(szName,		' ', sizeof(szName));
				memset(szAreaRaito,	' ', sizeof(szAreaRaito));
				memset(szElemRaito,	' ', sizeof(szElemRaito));
				memset(szPageRaito,	' ', sizeof(szPageRaito));
				szName[sizeof(szName) - 1]			= '\0';
				szAreaRaito[sizeof(szAreaRaito) - 1]= '\0';
				szElemRaito[sizeof(szElemRaito) - 1]= '\0';
				szPageRaito[sizeof(szPageRaito) - 1]= '\0';

				memcpy(szName, pstates[cnt].name, min(strlen(pstates[cnt].name), (sizeof(szName) - 1)));

				long	AreaRaito	= ((pstates[cnt].k2hstate.assigned_area_count * 100) / pstates[cnt].k2hstate.total_area_count);
				long	ElemRaito	= ((pstates[cnt].k2hstate.assigned_element_count * 100) / pstates[cnt].k2hstate.total_element_count);
				long	PageRaito	= ((pstates[cnt].k2hstate.assigned_page_count * 100) / pstates[cnt].k2hstate.total_page_count);

				sprintf(&szAreaRaito[((100 <= AreaRaito) ? 0 : ((10 <= AreaRaito) ? 1 : 2))], "%ld%%", AreaRaito);
				sprintf(&szElemRaito[((100 <= ElemRaito) ? 0 : ((10 <= ElemRaito) ? 1 : 2))], "%ld%%", ElemRaito);
				sprintf(&szPageRaito[((100 <= PageRaito) ? 0 : ((10 <= PageRaito) ? 1 : 2))], "%ld%%", PageRaito);
			}
			// print header
			if(0 == cnt){
				// example
				//   "<    chmpxid   >[<  base hash   >](      server name      ) : area element page (k2hash size/ file size )"
				//   "-----------------+-----------------+-------------------------:-----+-------+----+-------------------------"
				//   "0000000000000000[0000000000000000](                       ) :  00%     00%  00% (0000000000 / 0000000000)"
				//   "0000000000000000[0000000000000000](                       ) :  00%     00%  00% (0000000000 / 0000000000)"
				//
				PRN("<    chmpxid   >[<  base hash   >](      server name      ) : area element page (k2hash size/ file size )");
				PRN("----------------+-----------------+-------------------------:-----+-------+----+-------------------------");
			}
			// print line
			PRN("%016" PRIx64 "[%016" PRIx64 "](%s) : %s    %s %s (%zu / %zu)", pstates[cnt].chmpxid, pstates[cnt].base_hash, szName, szAreaRaito, szElemRaito, szPageRaito, pstates[cnt].k2hstate.total_size, pstates[cnt].k2hstate.file_size);
		}
	}

	DKC_FREE(pstates);

	return true;
}

//
// Command Line:	status chmpx [self] [full]
//					status node [full]
//
static bool StatusCommand(k2hdkc_chmpx_h chmpxhandle, params_t& params)
{
	// check parameter
	string	strSubCmd = params[0].c_str();
	params.erase(params.begin(), params.begin() + 1);

	// dispatch sub command
	bool	result;
	if(0 == strcasecmp(strSubCmd.c_str(), "chmpx")){
		result = StatusChmpxCommand(chmpxhandle, params);
	}else if(0 == strcasecmp(strSubCmd.c_str(), "node")){
		result = StatusNodeCommand(chmpxhandle, params);
	}else{
		ERR("Unknown parameter(%s) for status command.", strSubCmd.c_str());
		return true;	// for continue.
	}

	if(!result){
		ERR("Something error occurred during status %s command.", strSubCmd.c_str());
		return true;	// for continue.
	}
	return true;
}

//
// Command Line: history(his)
//
static bool HistoryCommand(const ConsoleInput& InputIF)
{
	const strarr_t&	history = InputIF.GetAllHistory();

	int	nCnt = 1;
	// cppcheck-suppress postfixOperator
	for(strarr_t::const_iterator iter = history.begin(); iter != history.end(); iter++, nCnt++){
		PRN(" %d  %s", nCnt, iter->c_str());
	}
	return true;
}

//
// Command Line: save <file path>
//
static bool SaveCommand(const ConsoleInput& InputIF, params_t& params)
{
	int	fd;
	if(-1 == (fd = open(params[0].c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644))){
		ERR("Could not open file(%s) for writing history. errno(%d)", params[0].c_str(), errno);
		return true;	// for continue
	}

	const strarr_t&	history = InputIF.GetAllHistory();
	// cppcheck-suppress postfixOperator
	for(strarr_t::const_iterator iter = history.begin(); iter != history.end(); iter++){
		// check except command for writing file
		if(	0 == strncasecmp(iter->c_str(), "his",		strlen("his"))		||
			0 == strncasecmp(iter->c_str(), "history",	strlen("history"))	||
			0 == strncasecmp(iter->c_str(), "shell",	strlen("shell"))	||
			0 == strncasecmp(iter->c_str(), "sh",		strlen("sh"))		||
			0 == strncasecmp(iter->c_str(), "!!",		strlen("!!"))		||
			0 == strncasecmp(iter->c_str(), "save",		strlen("save"))		||
			0 == strncasecmp(iter->c_str(), "load",		strlen("load"))		||
			iter->at(0) == '!' )
		{
			continue;
		}
		const char*	pHistory;
		size_t		wrote_byte;
		ssize_t		one_wrote_byte;
		for(pHistory = iter->c_str(), wrote_byte = 0, one_wrote_byte = 0L; wrote_byte < iter->length(); wrote_byte += one_wrote_byte){
			if(-1 == (one_wrote_byte = write(fd, &pHistory[wrote_byte], (iter->length() - wrote_byte)))){
				ERR("Failed writing history to file(%s). errno(%d)", params[0].c_str(), errno);
				DKC_CLOSE(fd);
				return true;	// for continue
			}
		}
		if(-1 == write(fd, "\n", 1)){
			ERR("Failed writing history to file(%s). errno(%d)", params[0].c_str(), errno);
			DKC_CLOSE(fd);
			return true;	// for continue
		}
	}
	DKC_CLOSE(fd);
	return true;
}

//
// Command Line: load <file path>
//
static bool LoadCommand(k2hdkc_chmpx_h chmpxhandle, ConsoleInput& InputIF, params_t& params, bool& is_exit)
{
	int	fd;
	if(-1 == (fd = open(params[0].c_str(), O_RDONLY))){
		ERR("Could not open file(%s) for reading commands. errno(%d)", params[0].c_str(), errno);
		return true;	// for continue
	}

	string	CommandLine;
	for(bool ReadResult = true; ReadResult; ){
		ReadResult = ReadLine(fd, CommandLine);
		if(0 < CommandLine.length()){
			PRN("> %s", CommandLine.c_str());

			// reentrant
			if(!CommandStringHandle(chmpxhandle, InputIF, CommandLine.c_str(), is_exit)){
				ERR("Something error occurred at loading command file(%s) - \"%s\", so stop running.", params[0].c_str(), CommandLine.c_str());
				break;
			}
			if(is_exit){
				break;
			}
		}
	}
	DKC_CLOSE(fd);
	return true;
}

//
// Command Line: shell                                        exit shell(same as "!" command).
//
static bool ShellCommand(void)
{
	static const char*	pDefaultShell = "/bin/sh";

	if(0 == system(NULL)){
		ERR("Could not execute shell.");
		return true;	// for continue
	}

	const char*	pEnvShell = getenv("SHELL");
	if(!pEnvShell){
		pEnvShell = pDefaultShell;
	}
	if(-1 == system(pEnvShell)){
		ERR("Something error occurred by executing shell(%s).", pEnvShell);
		return true;	// for continue
	}
	return true;
}

//
// Command Line: echo <string>...                             echo string
//
static bool EchoCommand(params_t& params)
{
	string	strDisp("");
	for(size_t cnt = 0; cnt < params.size(); ++cnt){
		if(0 < cnt){
			strDisp += ' ';
		}
		strDisp += params[cnt];
	}
	if(!strDisp.empty()){
		PRN("%s", strDisp.c_str());
	}
	return true;
}

//
// Command Line: sleep <second>
//
static bool SleepCommand(params_t& params)
{
	if(1 != params.size()){
		if(1 < params.size()){
			ERR("unknown parameter %s.", params[1].c_str());
		}else{
			ERR("sleep command needs parameter.");
		}
		return true;		// for continue.
	}
	unsigned int	sec = static_cast<unsigned int>(atoi(params[0].c_str()));
	sleep(sec);
	return true;
}

//---------------------------------------------------------
// Command Handling
//---------------------------------------------------------
static bool CommandStringHandle(k2hdkc_chmpx_h chmpxhandle, ConsoleInput& InputIF, const char* pCommand, bool& is_exit)
{
	is_exit = false;

	if(DKCEMPTYSTR(pCommand)){
		return true;
	}

	option_t	opts;
	if(!LineOptionParser(pCommand, opts)){
		return true;	// for continue.
	}
	// cppcheck-suppress unmatchedSuppression
	// cppcheck-suppress stlSize
	if(0 == opts.size()){
		return true;
	}

	// Command switch
	if(opts.end() != opts.find("help")){
		LineHelp();

	}else if(opts.end() != opts.find("quit")){
		PRN("Quit.");
		is_exit = true;

	}else if(opts.end() != opts.find("comlog")){
		if(!ComlogCommand(opts["comlog"])){
			CleanOptionMap(opts);
			return false;
		}

	}else if(opts.end() != opts.find("dbglevel")){
		if(!DbglevelCommand(opts["dbglevel"])){
			CleanOptionMap(opts);
			return false;
		}

	}else if(opts.end() != opts.find("print")){
		if(!PrintCommand(chmpxhandle, opts["print"])){
			CleanOptionMap(opts);
			return false;
		}

	}else if(opts.end() != opts.find("dp")){
		if(!DirectPrintCommand(chmpxhandle, opts["dp"])){
			CleanOptionMap(opts);
			return false;
		}

	}else if(opts.end() != opts.find("pa")){
		if(!PrintAttrCommand(chmpxhandle, opts["pa"])){
			CleanOptionMap(opts);
			return false;
		}

	}else if(opts.end() != opts.find("cf")){
		if(!CopyFileCommand(chmpxhandle, opts["cf"])){
			CleanOptionMap(opts);
			return false;
		}

	}else if(opts.end() != opts.find("set")){
		if(!SetCommand(chmpxhandle, opts["set"])){
			CleanOptionMap(opts);
			return false;
		}

	}else if(opts.end() != opts.find("dset")){
		if(!DirectSetCommand(chmpxhandle, opts["dset"])){
			CleanOptionMap(opts);
			return false;
		}

	}else if(opts.end() != opts.find("sf")){
		if(!SetFileCommand(chmpxhandle, opts["sf"])){
			CleanOptionMap(opts);
			return false;
		}

	}else if(opts.end() != opts.find("f")){
		if(!FillCommand(chmpxhandle, opts["f"])){
			CleanOptionMap(opts);
			return false;
		}

	}else if(opts.end() != opts.find("fs")){
		if(!FillSubCommand(chmpxhandle, opts["fs"])){
			CleanOptionMap(opts);
			return false;
		}

	}else if(opts.end() != opts.find("rm")){
		if(!RemoveCommand(chmpxhandle, opts["rm"])){
			CleanOptionMap(opts);
			return false;
		}

	}else if(opts.end() != opts.find("ss")){
		if(!SetSubkeyCommand(chmpxhandle, opts["ss"])){
			CleanOptionMap(opts);
			return false;
		}

	}else if(opts.end() != opts.find("rs")){
		if(!RemoveSubkeyCommand(chmpxhandle, opts["rs"])){
			CleanOptionMap(opts);
			return false;
		}

	}else if(opts.end() != opts.find("ren")){
		if(!RenameCommand(chmpxhandle, opts["ren"])){
			CleanOptionMap(opts);
			return false;
		}

	}else if(opts.end() != opts.find("q")){
		if(!QueueCommand(chmpxhandle, opts["q"])){
			CleanOptionMap(opts);
			return false;
		}

	}else if(opts.end() != opts.find("kq")){
		if(!KeyQueueCommand(chmpxhandle, opts["kq"])){
			CleanOptionMap(opts);
			return false;
		}

	}else if(opts.end() != opts.find("cas")){
		if(!CasCommand(chmpxhandle, CAS_TYPE_NO, opts["cas"])){
			CleanOptionMap(opts);
			return false;
		}

	}else if(opts.end() != opts.find("cas8")){
		if(!CasCommand(chmpxhandle, CAS_TYPE_8, opts["cas8"])){
			CleanOptionMap(opts);
			return false;
		}

	}else if(opts.end() != opts.find("cas16")){
		if(!CasCommand(chmpxhandle, CAS_TYPE_16, opts["cas16"])){
			CleanOptionMap(opts);
			return false;
		}

	}else if(opts.end() != opts.find("cas32")){
		if(!CasCommand(chmpxhandle, CAS_TYPE_32, opts["cas32"])){
			CleanOptionMap(opts);
			return false;
		}

	}else if(opts.end() != opts.find("cas64")){
		if(!CasCommand(chmpxhandle, CAS_TYPE_64, opts["cas64"])){
			CleanOptionMap(opts);
			return false;
		}

	}else if(opts.end() != opts.find("st")){
		if(!StatusCommand(chmpxhandle, opts["st"])){
			CleanOptionMap(opts);
			return false;
		}

	}else if(opts.end() != opts.find("history")){
		if(!HistoryCommand(InputIF)){
			CleanOptionMap(opts);
			return false;
		}

	}else if(opts.end() != opts.find("save")){
		if(!SaveCommand(InputIF, opts["save"])){
			CleanOptionMap(opts);
			return false;
		}

	}else if(opts.end() != opts.find("load")){
		if(!LoadCommand(chmpxhandle, InputIF, opts["load"], is_exit)){
			CleanOptionMap(opts);
			return false;
		}

	}else if(opts.end() != opts.find("shell")){
		if(!ShellCommand()){
			CleanOptionMap(opts);
			return false;
		}

	}else if(opts.end() != opts.find("echo")){
		if(!EchoCommand(opts["echo"])){
			CleanOptionMap(opts);
			return false;
		}

	}else if(opts.end() != opts.find("sleep")){
		if(!SleepCommand(opts["sleep"])){
			CleanOptionMap(opts);
			return false;
		}

	}else{
		ERR("Unknown command. see \"help\".");
	}
	CleanOptionMap(opts);

	return true;
}

static bool ExecHistoryCommand(k2hdkc_chmpx_h chmpxhandle, ConsoleInput& InputIF, ssize_t history_pos, bool& is_exit)
{
	const strarr_t&	history = InputIF.GetAllHistory();

	if(-1L == history_pos && 0 < history.size()){
		history_pos = static_cast<ssize_t>(history.size() - 1UL);

	}else if(0 < history_pos && static_cast<size_t>(history_pos) < history.size()){		// last history is "!..."
		history_pos--;

	}else{
		ERR("No history number(%zd) is existed.", history_pos);
		return true;																	// for continue.
	}
	InputIF.RemoveLastHistory();														// remove last(this) command from history
	InputIF.PutHistory(history[history_pos].c_str());									// and push this command(replace history)

	// execute
	PRN(" %s", history[history_pos].c_str());
	bool	result	= CommandStringHandle(chmpxhandle, InputIF, history[history_pos].c_str(), is_exit);

	return result;
}

static bool CommandHandle(k2hdkc_chmpx_h chmpxhandle, ConsoleInput& InputIF)
{
	if(!InputIF.GetCommand()){
		ERR("Something error occurred while reading stdin: err(%d).", InputIF.LastErrno());
		return false;
	}
	const string	strLine = InputIF.c_str();
	bool			is_exit = false;
	if(0 < strLine.length() && '!' == strLine[0]){
		// special character("!") command
		const char*	pSpecialCommand = strLine.c_str();
		pSpecialCommand++;

		if('\0' == *pSpecialCommand){
			// exit shell
			InputIF.RemoveLastHistory();	// remove last(this) command from history
			InputIF.PutHistory("shell");	// and push "shell" command(replace history)
			if(!ShellCommand()){
				return false;
			}
		}else{
			// execute history
			ssize_t	history_pos;
			if(1 == strlen(pSpecialCommand) && '!' == *pSpecialCommand){
				// "!!"
				history_pos = -1L;
			}else{
				history_pos = static_cast<ssize_t>(atoi(pSpecialCommand));
			}
			if(!ExecHistoryCommand(chmpxhandle, InputIF, history_pos, is_exit) || is_exit){
				return false;
			}
		}
	}else{
		if(!CommandStringHandle(chmpxhandle, InputIF, strLine.c_str(), is_exit) || is_exit){
			return false;
		}
	}
	return true;
}

//---------------------------------------------------------
// Main
//---------------------------------------------------------
int main(int argc, char** argv)
{
	option_t	opts;
	string		prgname;

	//----------------------
	// Console: default
	//----------------------
	ConsoleInput	InputIF;
	InputIF.SetMax(1000);				// command history 1000
	InputIF.SetPrompt("K2HDKC> ");		// Prompt

	if(!ExecOptionParser(argc, argv, opts, prgname)){
		Help(prgname.c_str());
		exit(EXIT_FAILURE);
	}

	//----------------------
	// Check and Set Options
	//----------------------
	// -capi : API Mode
	if(opts.end() != opts.find("-capi")){
		isModeCAPI	= true;
	}
	// -dfile
	if(opts.end() != opts.find("-dfile")){
		bool result;
		if(isModeCAPI){
			result = k2hdkc_set_debug_file(opts["-dfile"][0].c_str());
		}else{
			result = SetK2hdkcDbgFile(opts["-dfile"][0].c_str());
		}
		if(!result){
			ERR("Failed to set debug message log file(%s).", opts["-dfile"][0].c_str());
			exit(EXIT_FAILURE);
		}
	}
	// -d
	if(opts.end() != opts.find("-d")){
		if(0 == strcasecmp(opts["-d"][0].c_str(), "silent") || 0 == strcasecmp(opts["-d"][0].c_str(), "slt")){
			if(isModeCAPI){
				k2hdkc_set_debug_level_silent();
			}else{
				SetK2hdkcDbgMode(DKCDBG_SILENT);
			}
		}else if(0 == strcasecmp(opts["-d"][0].c_str(), "error") || 0 == strcasecmp(opts["-d"][0].c_str(), "err")){
			if(isModeCAPI){
				k2hdkc_set_debug_level_error();
			}else{
				SetK2hdkcDbgMode(DKCDBG_ERR);
			}
		}else if(0 == strcasecmp(opts["-d"][0].c_str(), "warning") || 0 == strcasecmp(opts["-d"][0].c_str(), "wan")){
			if(isModeCAPI){
				k2hdkc_set_debug_level_warning();
			}else{
				SetK2hdkcDbgMode(DKCDBG_WARN);
			}
			is_print_wan = true;
		}else if(0 == strcasecmp(opts["-d"][0].c_str(), "info") || 0 == strcasecmp(opts["-d"][0].c_str(), "msg")){
			if(isModeCAPI){
				k2hdkc_set_debug_level_message();
			}else{
				SetK2hdkcDbgMode(DKCDBG_MSG);
			}
			is_print_msg = true;
			is_print_wan = true;
		}else if(0 == strcasecmp(opts["-d"][0].c_str(), "dump") || 0 == strcasecmp(opts["-d"][0].c_str(), "dmp")){
			if(isModeCAPI){
				k2hdkc_set_debug_level_dump();
			}else{
				SetK2hdkcDbgMode(DKCDBG_DUMP);
			}
			is_print_msg = true;
			is_print_wan = true;
		}else{
			ERR("Unknown parameter(%s) value for \"-d\" option.", opts["-d"][0].c_str());
			exit(EXIT_FAILURE);
		}
	}
	// -help
	if(opts.end() != opts.find("-help")){
		Help(prgname.c_str());
		exit(EXIT_SUCCESS);
	}
	// -libversion
	if(opts.end() != opts.find("-libversion")){
		k2hdkc_print_version(stdout);
		exit(EXIT_SUCCESS);
	}
	// -conf or -json
	if(opts.end() != opts.find("-conf")){
		strConfig = opts["-conf"][0];
	}else if(opts.end() != opts.find("-json")){
		strConfig = opts["-json"][0];
	}
	// -ctlport
	if(opts.end() != opts.find("-ctlport")){
		string	strtmp	= opts["-ctlport"][0];
		CntlPort		= static_cast<short>(atoi(strtmp.c_str()));
	}
	// -cuk
	if(opts.end() != opts.find("-cuk")){
		strCuk	= opts["-cuk"][0];
	}
	// -permanent
	if(opts.end() != opts.find("-permanent")){
		isPermanent	= true;
	}
	// -rejoin
	if(opts.end() != opts.find("-rejoin")){
		isAutoRejoin = true;
	}
	// -nogiveup
	if(opts.end() != opts.find("-nogiveup")){
		isNoGiveupRejoin = true;
	}
	// -nocleanup
	if(opts.end() != opts.find("-nocleanup")){
		isCleanupBup = false;
	}
	// -comlog
	bool	is_comlog = false;
	if(opts.end() != opts.find("-comlog")){
		if(0 < opts["-comlog"].size()){
			if(0 == strcasecmp(opts["-comlog"][0].c_str(), "on") || 0 == strcasecmp(opts["-comlog"][0].c_str(), "yes") || 0 == strcasecmp(opts["-comlog"][0].c_str(), "y")){
				is_comlog = true;
			}else if(0 == strcasecmp(opts["-comlog"][0].c_str(), "off") || 0 == strcasecmp(opts["-comlog"][0].c_str(), "no") || 0 == strcasecmp(opts["-comlog"][0].c_str(), "n")){
				is_comlog = false;
			}else{
				ERR("Unknown parameter(%s) value for \"-comlog\" option.", opts["-comlog"][0].c_str());
				exit(EXIT_FAILURE);
			}
		}
	}
	if(isModeCAPI){
		if(is_comlog){
			k2hdkc_enable_comlog();
		}else{
			k2hdkc_disable_comlog();
		}
	}else{
		if(is_comlog){
			K2hdkcComNumber::Enable();
		}else{
			K2hdkcComNumber::Disable();
		}
	}
	// -lap
	if(opts.end() != opts.find("-lap")){
		LapTime::Enable();
	}
	// -history
	if(opts.end() != opts.find("-history")){
		int	hiscount = atoi(opts["-history"][0].c_str());
		InputIF.SetMax(static_cast<size_t>(hiscount));
	}
	// -run option
	string	CommandFile("");
	if(opts.end() != opts.find("-run")){
		if(0 == opts["-run"].size()){
			ERR("Option \"-run\" needs parameter as command file path.");
			exit(EXIT_FAILURE);
		}else if(1 < opts["-run"].size()){
			ERR("Unknown parameter(%s) value for \"-run\" option.", opts["-run"][1].c_str());
			exit(EXIT_FAILURE);
		}
		// check file
		struct stat	st;
		if(0 != stat(opts["-run"][0].c_str(), &st)){
			ERR("Parameter command file path(%s) for option \"-run\" does not exist(errno=%d).", opts["-run"][0].c_str(), errno);
			exit(EXIT_FAILURE);
		}
		CommandFile = opts["-run"][0];
	}

	// cleanup for valgrind
	CleanOptionMap(opts);

	//----------------------
	// Check options
	//----------------------
	if(strConfig.empty()){
		// over check
		CHMCONFTYPE	conftype = check_chmconf_type_ex(NULL, K2HDKC_CONFFILE_ENV_NAME, K2HDKC_JSONCONF_ENV_NAME, &strConfig);
		if(CHMCONF_TYPE_UNKNOWN == conftype || CHMCONF_TYPE_NULL == conftype){
			ERR("Must specify \"-conf\" or \"-json\" option.");
			exit(EXIT_FAILURE);
		}
	}

	//----------------------
	// Permanent chmpx
	//----------------------
	K2hdkcSlave*	pSlave		= NULL;
	k2hdkc_chmpx_h	chmpxhandle	= K2HDKC_INVALID_HANDLE;
	if(isPermanent){
		if(isModeCAPI){
			if(K2HDKC_INVALID_HANDLE == (chmpxhandle = k2hdkc_open_chmpx_full(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin, isNoGiveupRejoin, isCleanupBup))){
				ERR("Could not open(join and open msgid) slave node chmpx.");
				exit(EXIT_FAILURE);
			}
		}else{
			pSlave = new K2hdkcSlave();
			if(!pSlave->Initialize(strConfig.c_str(), CntlPort, (strCuk.empty() ? NULL : strCuk.c_str()), isAutoRejoin)){
				ERR("Could not join slave node chmpx.");
				DKC_DELETE(pSlave);
				exit(EXIT_FAILURE);
			}
			if(!pSlave->Open(isNoGiveupRejoin)){
				ERR("Could not open msgid on slave node chmpx.");
				DKC_DELETE(pSlave);
				exit(EXIT_FAILURE);
			}
			chmpxhandle	= reinterpret_cast<k2hdkc_chmpx_h>(pSlave);
		}
	}

	//----------------------
	// Main Loop
	//----------------------
	bool	IsWelcomMsg = true;
	do{
		// check command file at starting
		if(!CommandFile.empty()){
			string	LoadCommandLine("load ");
			LoadCommandLine += CommandFile;

			bool	is_exit = false;
			if(!CommandStringHandle(chmpxhandle, InputIF, LoadCommandLine.c_str(), is_exit) || is_exit){
				break;
			}
			CommandFile.clear();

		// cppcheck-suppress unmatchedSuppression
		// cppcheck-suppress knownConditionTrueFalse
		}else if(IsWelcomMsg){
			// print message
			PRN("-------------------------------------------------------");
			PRN(" K2HDKC LINE TOOL");
			PRN("-------------------------------------------------------");
			PRN("K2HDKC library version          : %s",	VERSION);
			PRN("K2HDKC API                      : %s",	isModeCAPI ? "C" : "C++");
			PRN("Communication log mode          : %s",	IsK2hdkcComLog() ? "yes" : "no");
			PRN("Debug mode                      : %s",	K2hdkcDbgMode_FULLSTR(GetK2hdkcDbgMode()));
			PRN("Debug log file                  : %s",	GetK2hdkcDbgFile() ? GetK2hdkcDbgFile() : "not set");
			PRN("Print command lap time          : %s",	LapTime::IsEnable() ? "yes" : "no");
			PRN("Command line history count      : %zu",	InputIF.GetMax());
			PRN("Chmpx parameters:");
			PRN("    Configuration               : %s",	strConfig.c_str());
			PRN("    Control port                : %d",	(CHM_INVALID_PORT == CntlPort ? 0 : CntlPort));
			PRN("    CUK                         : %s",	strCuk.c_str());
			PRN("    Permanent connect           : %s",	isPermanent ? "yes" : "no");
			PRN("    Auto rejoin                 : %s",	isAutoRejoin ? "yes" : "no");
			PRN("    Join giveup                 : %s",	isNoGiveupRejoin ? "yes" : "no");
			PRN("    Cleanup backup files        : %s",	isCleanupBup ? "yes" : "no");
			PRN("-------------------------------------------------------");
		}
		IsWelcomMsg = false;
		// start interactive until error occurred.

	}while(CommandHandle(chmpxhandle, InputIF));

	InputIF.Clean();

	//----------------------
	// Permanent chmpx(close)
	//----------------------
	if(isPermanent){
		if(isModeCAPI){
			if(!k2hdkc_close_chmpx_ex(chmpxhandle, isCleanupBup)){
				ERR("Could not close(leave and close msgid) slave node chmpx, but continue for cleanup...");
			}
			chmpxhandle	= K2HDKC_INVALID_HANDLE;		// force
		}else{
			if(!pSlave->Close()){
				ERR("Could not close msgid on slave node chmpx, but continue for cleanup...");
			}
			if(!pSlave->Clean(isCleanupBup)){
				ERR("Could not leave slave node chmpx, but continue for cleanup...");
			}
			DKC_DELETE(pSlave);							// force
			chmpxhandle	= K2HDKC_INVALID_HANDLE;		// force
		}
	}

	exit(EXIT_SUCCESS);
}

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */

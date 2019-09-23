#include "log.h"

const char* log_loglevel_str[5] = {
	"---",
	"ERROR",
	"WARNING",
	"NOTE",
	"DEBUG"
};

int log_init_file(char* _file, unsigned int _verbosity)
{
	log_fd = open(_file, O_WRONLY | O_APPEND | O_CREAT | O_DSYNC);

	return log_init_stdout(_verbosity);;
}

int log_init_stdout(unsigned int _verbosity)
{
	log_loglevel = _verbosity;// > _LOG_DEBUG ? _LOG_DEBUG : _verbosity;
	log_fd = STDIN_FILENO;
	
	LOGPRINTF(0, "=== RESTART ===");
	LOGPRINTF(0, "Verbosity: %i", _verbosity);

	return 0;
}

int log_close()
{
	return close(log_fd);
}

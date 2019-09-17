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
	int fd = open(_file, O_WRONLY | O_APPEND | O_CREAT | O_DSYNC);

	if(fd < 0) {
		LOGPRINTF(_LOG_ERROR, "Failed to open LogFile %s", _file);
	} else {
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
	}
	return log_init_stdout(_verbosity);;
}

int log_init_stdout(unsigned int _verbosity)
{
	log_loglevel = _verbosity;// > _LOG_DEBUG ? _LOG_DEBUG : _verbosity;
	LOGPRINTF(0, "=== RESTART ===");
	LOGPRINTF(0, "Verbosity: %i", _verbosity);
	return 0;
}


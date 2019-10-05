#include <string>

#include "utils.hpp"
#include "httpServer.hpp"

extern "C"{
	HttpResponse plugin(PluginArg arg);
}
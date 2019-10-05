#include <string>

#include "utils.hpp"
#include "httpServer.hpp"

extern "C"{
	HttpResponse signup(PluginArg arg);

	void __attribute__ ((constructor)) initPlugin();
	void __attribute__ ((destructor)) clean();
}
#include <string>

#include "utils.hpp"
#include "httpServer.hpp"

extern "C"{
	HttpResponse login(PluginArg arg);
	HttpResponse signup(PluginArg arg);
	HttpResponse postTrack(PluginArg arg);

	bool checkUser(std::string userName, std::string password, Sqlite3DB *db);
	int getUserId(std::string userName, Sqlite3DB *db);

	void __attribute__ ((constructor)) initPlugin();
	void __attribute__ ((destructor)) clean();
}
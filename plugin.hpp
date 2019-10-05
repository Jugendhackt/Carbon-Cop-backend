#include <string>
#include <time.h>

#include "utils.hpp"
#include "httpServer.hpp"

extern "C"{
	HttpResponse login(PluginArg arg);
	HttpResponse signup(PluginArg arg);
	HttpResponse postTrack(PluginArg arg);
	HttpResponse getToplist(PluginArg arg);

	void __attribute__ ((constructor)) initPlugin();
	void __attribute__ ((destructor)) clean();
}


bool checkUser(std::string userName, std::string password, Sqlite3DB *db);
int getUserId(std::string userName, Sqlite3DB *db);

float calcDist(int userId, std::string vehicle, Sqlite3DB *db);
float calcScore(int userId, Sqlite3DB *db);
float calcCO2(float dist, std::string vehicle);
float calcCO2(std::string userName, std::string vehicle, Sqlite3DB *db);
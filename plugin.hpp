#include <string>
#include <time.h>
#include <algorithm>
#include <functional>

#include "utils.hpp"
#include "httpServer.hpp"

class Challenge{
public:
	Challenge(std::function<bool(std::string, Sqlite3DB*)> unlocked, std::string name = "", std::string description = "", std::string icon = "");
	bool isunlocked(std::string name, Sqlite3DB *db);
	cJSON* toJson();

private:
	std::string name;
	std::string description;
	std::string icon;
	std::function<bool(std::string, Sqlite3DB*)> unlocked;
};

int getUserId(std::string userName, Sqlite3DB *db);
bool checkUser(std::string userName, std::string password, Sqlite3DB *db);

float calcDist(int userId, std::string vehicle, Sqlite3DB *db);
float calcScore(int userId, Sqlite3DB *db);
float calcCO2(float dist, std::string vehicle);
float calcCO2(std::string userName, std::string vehicle, Sqlite3DB *db);


extern "C"{
	HttpResponse login(PluginArg arg);
	HttpResponse signup(PluginArg arg);
	HttpResponse postTrack(PluginArg arg);
	HttpResponse getToplist(PluginArg arg);
	HttpResponse getChallenges(PluginArg arg);

	void __attribute__ ((constructor)) initPlugin();
	void __attribute__ ((destructor)) clean();
}
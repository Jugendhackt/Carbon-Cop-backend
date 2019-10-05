#include "httpServer.hpp"
#include "utils.hpp"

int main()
{
	sys::init();
	HttpServer *server = new HttpServer();
	Sqlite3DB *db = new Sqlite3DB("./data/main.db3");
	server->loadPlugins("./plugins.json", db);

#if defined(DEBUG)
	server->setCorsEnabled(true);
	server->setAcawEnabled(true);
#endif

	server->start();
	delete server;
	delete db;
	return 0;
}

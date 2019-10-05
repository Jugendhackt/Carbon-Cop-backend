#include "plugin.hpp"

HttpResponse plugin(PluginArg arg){
	/*
	"POST server.com/upload"
	...
	"username:abc"
	"password:123"
	"strecke:5km"
	"co2verbrauch/km:1kg"
*/
	return {404, "text/plain", "my plugin"};
}
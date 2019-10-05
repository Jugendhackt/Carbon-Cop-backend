#include "plugin.hpp"

HttpResponse plugin(PluginArg arg){
	return {404, "text/plain", "my plugin"};
}
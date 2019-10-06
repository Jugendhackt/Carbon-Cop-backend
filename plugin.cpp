#include "plugin.hpp"


Challenge::Challenge(std::function<bool(std::string, Sqlite3DB*)> unlocked, std::string name, std::string description, std::string icon){
	this->name = name;
	this->description = description;
	this->icon = icon;
	this->unlocked = unlocked;
}

cJSON* Challenge::toJson(std::string name, Sqlite3DB *db){
	cJSON *json = cJSON_CreateObject();

	cJSON_AddStringToObject(json, "name", this->name.c_str());
	cJSON_AddStringToObject(json, "description", description.c_str());
	cJSON_AddStringToObject(json, "icon", icon.c_str());
	cJSON_AddNumberToObject(json, "unlocked", int(unlocked(name, db)));

	return json;
}

unsigned long long time(){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;
}

std::vector<Challenge> challenges = {
	Challenge([&](std::string name, Sqlite3DB *db) -> bool {
		return calcDist(getUserId(name, db), "foot", db) >= 5;
	}, "walking bronze", "walk for 5 km", "icons/bronze_walk.png"),

	Challenge([&](std::string name, Sqlite3DB *db) -> bool {
		return calcDist(getUserId(name, db), "foot", db) >= 42.195;
	}, "marathon", "walk a marathon", "icons/marathon.png"),

	Challenge([&](std::string name, Sqlite3DB *db) -> bool {
		return calcDist(getUserId(name, db), "foot", db) >= 50;
	}, "walking silver", "walk for 50 km", "icons/silver_walk.png"),

	Challenge([&](std::string name, Sqlite3DB *db) -> bool {
		return calcDist(getUserId(name, db), "foot", db) >= 100;
	}, "walking gold", "walk for 100 km", "icons/gold_walk.png"),
	
	Challenge([&](std::string name, Sqlite3DB *db) -> bool {
		return calcDist(getUserId(name, db), "foot", db) >= 500;
	}, "walking platin", "walk for 500 km", "icons/platin_walk.png"),


	Challenge([&](std::string name, Sqlite3DB *db) -> bool {
		return calcDist(getUserId(name, db), "bike", db) >= 5;
	}, "biking bronze", "ride a bike for 5 km", "icons/bronze_bike.png"),

	Challenge([&](std::string name, Sqlite3DB *db) -> bool {
		return calcDist(getUserId(name, db), "bike", db) >= 50;
	}, "biking silver", "ride a bike for 50 km", "icons/silver_bike.png"),

	Challenge([&](std::string name, Sqlite3DB *db) -> bool {
		return calcDist(getUserId(name, db), "bike", db) >= 100;
	}, "biking gold", "ride a bike for 100 km", "icons/gold_bike.png"),
	
	Challenge([&](std::string name, Sqlite3DB *db) -> bool {
		return calcDist(getUserId(name, db), "bike", db) >= 500;
	}, "biking platin", "ride a bike for 500 km", "icons/platin_bike.png"),


	Challenge([&](std::string name, Sqlite3DB *db) -> bool {
		return time() - lastTrack(getUserId(name, db), "car", db) >= 5*24*60*60*1000;
	}, "streak bronze", "5 days no car", "icons/bronze_bike.png"),

	Challenge([&](std::string name, Sqlite3DB *db) -> bool {
		return time() - lastTrack(getUserId(name, db), "car", db) >= 10*24*60*60*1000;
	}, "streak silver", "10 days no car", "icons/silver_bike.png"),

	Challenge([&](std::string name, Sqlite3DB *db) -> bool {
		return time() - lastTrack(getUserId(name, db), "car", db) >= 20*24*60*60*1000;
	}, "streak gold", "20 days no car", "icons/gold_bike.png"),
	
	Challenge([&](std::string name, Sqlite3DB *db) -> bool {
		return time() - lastTrack(getUserId(name, db), "car", db) >= (unsigned long long)(50)*24*60*60*1000;
	}, "streak platin", "50 days no car", "icons/platin_bike.png")
};

bool checkUser(std::string userName, std::string password, Sqlite3DB *db){
	std::stringstream querry;
	querry<<"SELECT password FROM users WHERE name LIKE \'"<<stringToHex(userName)<<"\';";
	dbResult *result = db->exec(querry.str());
	bool valid = false;
	if(result->data.size() > 0 && result->columns > 0){
		if(result->data[0][0] == password){
			valid = true;
		}
	}
	delete result;
	return valid;
}

int getUserId(std::string userName, Sqlite3DB *db){
	std::stringstream querry;
	querry<<"SELECT id FROM users WHERE name LIKE \'"<<stringToHex(userName)<<"\'";
	dbResult *result = db->exec(querry.str());
	if(result->data.size() > 0 && result->columns > 0){
		std::string strid = result->data[0][0];
		delete result;
		return atoi(strid.c_str());
	}
	delete result;
	return -1;
}


float calcDist(int userId, std::string vehicle, Sqlite3DB *db){
	float val = 0;
	if(userId > 0){
		std::stringstream querry;
		querry<<"SELECT vehicle,sum(distance) FROM tracks_"<<userId<<" GROUP BY vehicle;";
		dbResult *result = db->exec(querry.str());
		for(unsigned i = 0; i < result->data.size(); i++){
			if(std::string(result->data[i][0]) == vehicle){
				val = strtof(result->data[i][1].c_str(), nullptr);
				break;
			}
		}
		delete result;
	}
	return val;
}

float calcScore(int userId, Sqlite3DB *db){
	float score = 	calcDist(userId, "walk", db)  * 10 + 
					calcDist(userId, "bike", db)  * 10 + 
					calcDist(userId, "bus", db)   *  3 + 
					calcDist(userId, "car", db)   * -3 + 
					calcDist(userId, "plane", db) * -10 + 
					calcDist(userId, "train", db) *  1.5;

	return score;
}

float calcCO2(float dist, std::string vehicle){
	float co2 = 0.0f;
	if(vehicle == "walk") co2 = 0.0f;
	else if(vehicle == "bike") co2 = 0.0f;
	else if(vehicle == "bus") co2 = 20.0f;
	else if(vehicle == "car") co2 = 150.0f;
	else if(vehicle == "plane") co2 = 380.0f;
	else if(vehicle == "train") co2 = 40.0f;

	return dist * co2;
}

float calcCO2(std::string userName, std::string vehicle, Sqlite3DB *db){
	int userId = getUserId(userName, db);
	float dist = calcDist(userId, vehicle, db);
	return calcCO2(dist, vehicle);
}

unsigned long long lastTrack(int userId, std::string vehicle, Sqlite3DB *db){
	unsigned long long val = 0;
	if(userId > 0){
		std::stringstream querry;
		querry<<"SELECT vehicle,max(date) FROM tracks_"<<userId<<" GROUP BY vehicle;";
		dbResult *result = db->exec(querry.str());
		for(unsigned i = 0; i < result->data.size(); i++){
			if(std::string(result->data[i][0]) == vehicle){
				val = std::stoull(result->data[i][1].c_str(), nullptr, 0);
				break;
			}
		}
		delete result;
	}
	return val;
}

//////////////////////////////////////////////
//////////////////////////////////////////////

HttpResponse login(PluginArg arg){
	std::string payload = arg.client->socket->recv(arg.payloadSize);

	cJSON *root = cJSON_Parse(payload.c_str());

	if(!cJSON_HasObjectItem(root, "username")){
		cJSON_Delete(root);
		return {HttpStatus_BadRequest,"application/json","{\"error\":\"username missing in json\"}"};
	}
	if(!cJSON_HasObjectItem(root, "password")){
		cJSON_Delete(root);
		return {HttpStatus_BadRequest,"application/json","{\"error\":\"password missing in json\"}"};
	}

	char *userNameRaw = cJSON_GetObjectItem(root, "userName")->valuestring;
	char *passwordRaw = cJSON_GetObjectItem(root, "password")->valuestring;

	std::string userName = (userNameRaw == nullptr) ? "" : userNameRaw;
	std::string password = (passwordRaw == nullptr) ? "" : passwordRaw;

	cJSON_Delete(root);

	Sqlite3DB *db = reinterpret_cast<Sqlite3DB*>(arg.arg);

	if(checkUser(userName, password, db)){
		return {200, "application/json", "{\"status\":\"login succsessful\"}"};
	}
	else{
		return {HttpStatus_Forbidden, "application/json", "{\"error\":\"wrong userName or password\"}"};
	}
}

HttpResponse signup(PluginArg arg){
	std::string payload = arg.client->socket->recv(arg.payloadSize);

	cJSON *root = cJSON_Parse(payload.c_str());

	if(!cJSON_HasObjectItem(root, "username")){
		cJSON_Delete(root);
		return {HttpStatus_BadRequest,"application/json","{\"error\":\"username missing in json\"}"};
	}
	if(!cJSON_HasObjectItem(root, "password")){
		cJSON_Delete(root);
		return {HttpStatus_BadRequest,"application/json","{\"error\":\"password missing in json\"}"};
	}

	char *userNameRaw = cJSON_GetObjectItem(root, "userName")->valuestring;
	char *passwordRaw = cJSON_GetObjectItem(root, "password")->valuestring;

	std::string userName = (userNameRaw == nullptr) ? "" : userNameRaw;
	std::string password = (passwordRaw == nullptr) ? "" : passwordRaw;

	cJSON_Delete(root);

	Sqlite3DB *db = reinterpret_cast<Sqlite3DB*>(arg.arg);

	std::stringstream querry;
	querry<<"SELECT name FROM users WHERE name LIKE \'"<<stringToHex(userName)<<"\';";
	dbResult *result = db->exec(querry.str());
	if(result->data.size() == 0){
		//create user
		querry.str("");
		querry<<"INSERT INTO users (name, password, score) VALUES (\'"<<stringToHex(userName)<<"\', \'"<<stringToHex(password)<<"\', 0.0);";
		delete result;

		result = db->exec(querry.str());
		int userId = result->rowId;
		delete result;

		querry.str("");
		querry<<"BEGIN TRANSACTION;CREATE TABLE IF NOT EXISTS \"tracks_"<<userId<<"\"";
		querry<<"(\"distance\" REAL, \"vehicle\" TEXT, \"date\"	INTEGER);COMMIT;";
		result = db->exec(querry.str());
	}
	else{
		delete result;
		return {HttpStatus_Forbidden, "application/json", "{\"error\":\"user already exists\"}"};
	}

	delete result;
	return {200, "application/json", "{\"status\":\"signup succsessful\"}"};
}

HttpResponse getStats(PluginArg arg){
	std::string url = arg.url;
	std::string name(url.begin() + std::min(url.rfind("?name="), url.length()-7) + 6, url.end());

	Sqlite3DB *db = reinterpret_cast<Sqlite3DB*>(arg.arg);

	cJSON *result = cJSON_CreateObject();

	int userId = getUserId(name, db);
	for(std::string str : {"walk", "bike", "bus", "car", "plane", "train"}){
		float dist = calcDist(userId, str, db);
		cJSON_AddNumberToObject(result, str.c_str(), dist);
	}
	cJSON_AddNumberToObject(result, "score", calcScore(userId, db));

	const char *data = cJSON_Print(result);
	cJSON_Delete(result);
	std::string json = data;
	delete data;
	return {200, "application/json", json};
}

HttpResponse postTrack(PluginArg arg){
	std::string payload = arg.client->socket->recv(arg.payloadSize);

	cJSON *root = cJSON_Parse(payload.c_str());

	if(!cJSON_HasObjectItem(root, "username")){
		cJSON_Delete(root);
		return {HttpStatus_BadRequest,"application/json","{\"error\":\"username missing in json\"}"};
	}
	if(!cJSON_HasObjectItem(root, "password")){
		cJSON_Delete(root);
		return {HttpStatus_BadRequest,"application/json","{\"error\":\"password missing in json\"}"};
	}
	if(!cJSON_HasObjectItem(root, "distance")){
		cJSON_Delete(root);
		return {HttpStatus_BadRequest,"application/json","{\"error\":\"distance missing in json\"}"};
	}
	if(!cJSON_HasObjectItem(root, "vehicle")){
		cJSON_Delete(root);
		return {HttpStatus_BadRequest,"application/json","{\"error\":\"vehicle missing in json\"}"};
	}

	char *userNameRaw = cJSON_GetObjectItem(root, "username")->valuestring;
	char *passwordRaw = cJSON_GetObjectItem(root, "password")->valuestring;
	char *distanceRaw = cJSON_GetObjectItem(root, "distance")->valuestring;
	char *vehicleRaw = cJSON_GetObjectItem(root, "vehicle")->valuestring;

	std::string userName = (userNameRaw == nullptr) ? "" : userNameRaw;
	std::string password = (passwordRaw == nullptr) ? "" : passwordRaw;
	std::string distance = (distanceRaw == nullptr) ? "" : distanceRaw;
	std::string vehicle = (vehicleRaw == nullptr) ? "" : vehicleRaw;

	cJSON_Delete(root);	
	Sqlite3DB *db = reinterpret_cast<Sqlite3DB*>(arg.arg);

	if(checkUser(userName, password, db)){
		int id = getUserId(userName, db);
		std::stringstream querry;
		querry<<"INSERT INTO tracks_"<<id<<" (distance, vehicle, date) VALUES ("<<distance<<", \'"<<stringToHex(vehicle)<<" \',"<<time()<<");";
		dbResult *result = db->exec(querry.str());
		delete result;

		float score = calcScore(id, db);
		querry.str("");
		querry<<"UPDATE users SET score = "<<score<<" WHERE name LIKE \'"<<stringToHex(userName)<<"\';";
		result = db->exec(querry.str());
		delete result;
	}
	else{
		return {HttpStatus_Forbidden,"application/json","{\"error\":\"login data invalid\"}"};
	}

	return {200, "application/json", "{\"status\":\"posted succsessful\"}"};
}

HttpResponse getToplist(PluginArg arg){
	std::string url = arg.url;
	Sqlite3DB *db = reinterpret_cast<Sqlite3DB*>(arg.arg);

	dbResult *result = db->exec("SELECT name,score FROM users ORDER BY score DESC;");
	cJSON *root = cJSON_CreateObject();
	cJSON *toplist = cJSON_AddArrayToObject(root, "top");
	
	int rank = 1;
	for(std::string *row : result->data){
		std::string name = row[0];
		float score = strtof(row[1].c_str(), nullptr);

		cJSON *entry = cJSON_CreateObject();
		cJSON_AddNumberToObject(entry, "rank", rank++);
		cJSON_AddNumberToObject(entry, "score", score);
		cJSON_AddStringToObject(entry, "name", name.c_str());

		cJSON_AddItemToArray(toplist, entry);
	}
	
	delete result;
	const char *data = cJSON_Print(root);
	cJSON_Delete(root);
	std::string json = data;
	delete data;
	return {200, "application/json", json};
}

HttpResponse getChallenges(PluginArg arg){
	std::string url = arg.url;
	std::string name(url.begin() + std::min(url.rfind("?name="), url.length()-7) + 6, url.end());

	Sqlite3DB *db = reinterpret_cast<Sqlite3DB*>(arg.arg);

	cJSON *result = cJSON_CreateObject();
	cJSON *list = cJSON_AddArrayToObject(result, "challenges");

	for(Challenge &c : challenges){
		cJSON_AddItemToArray(list, c.toJson(name, db));
	}

	const char *data = cJSON_Print(result);
	cJSON_Delete(result);
	std::string json = data;
	delete data;
	return {200, "application/json", json};
}

void __attribute__ ((constructor)) initPlugin(){
	Sqlite3DB db("data/main.db3");
	db.exec(File::readAll("./init.sql"));
}

void __attribute__ ((destructor)) clean(){
	;
}
#include "plugin.hpp"

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
	querry<<"SELECT name FROM users WHERE name LIKE \'"<<userName<<"\';";
	dbResult *result = db->exec(querry.str());
	if(result->data.size() == 0){
		//create user
		querry.str("");
		querry<<"INSERT INTO users (name, password, score) VALUES (\'"<<userName<<"\', \'"<<password<<"\', 0.0);";
		delete result;
		result = db->exec(querry.str());
		int userId = result->rowId;
		querry.str("");
		querry<<"BEGIN TRANSACTION;CREATE TABLE IF NOT EXISTS \"tracks_"<<userId<<"\"";
		querry<<"(\"distance\" REAL, \"vehicle\" TEXT, \"date\"	DATE);COMMIT;";
		delete result;
		result = db->exec(querry.str());
	}
	else{
		delete result;
		return {HttpStatus_Forbidden, "application/json", "{\"error\":\"user already exists\"}"};
	}

	delete result;
	return {200, "application/json", "{\"status\":\"signup succsessful\"}"};
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
		querry<<"INSERT INTO tracks_"<<id<<" (distance, vehicle) VALUES ("<<distance<<", \'"<<vehicle<<" \');";
		dbResult *result = db->exec(querry.str());
		delete result;
		float score = calcScore(id, db);
		std::cout<<score<<"\n";
		querry.str("");
		querry<<"UPDATE users SET score = "<<score<<" WHERE name LIKE \'"<<userName<<"\';";
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
	std::stringstream querry;
	querry<<"SELECT name,score FROM users ORDER BY score DESC;";

	dbResult *result = db->exec(querry.str());
	cJSON *root = cJSON_CreateObject();
	cJSON *toplist = cJSON_AddArrayToObject(root, "top");
	
	int rank = 1;
	for(std::string *row : result->data){
		std::string name = row[0];
		float score = strtof(row[1].c_str(), nullptr);
		cJSON *entry = cJSON_CreateObject();
		cJSON_AddNumberToObject(entry, "rank", rank);
		cJSON_AddNumberToObject(entry, "score", score);
		cJSON_AddStringToObject(entry, "name", name.c_str());

		cJSON_AddItemToArray(toplist, entry);
		rank += 1;
	}
	
	delete result;
	const char *data = cJSON_Print(root);
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




bool checkUser(std::string userName, std::string password, Sqlite3DB *db){
	std::stringstream querry;
	querry<<"SELECT password FROM users WHERE name LIKE \'"<<userName<<"\';";
	dbResult *result = db->exec(querry.str());
	if(result->data.size() > 0 && result->columns > 0){
		if(result->data[0][0] == password){
			delete result;
			return true;
		}
		delete result;
		return false;
	}
	delete result;
	return false;
}

int getUserId(std::string userName, Sqlite3DB *db){
	std::stringstream querry;
	querry<<"SELECT id FROM users WHERE name LIKE \'"<<userName<<"\'";
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
	std::stringstream querry;
	querry<<"SELECT vehicle,sum(distance) FROM tracks_"<<userId<<" GROUP BY vehicle;";
	dbResult *result = db->exec(querry.str());
	for(int i = 0; i < result->data.size(); i++){
		if(std::string(result->data[i][0]) == vehicle){
			std::cout<<vehicle<<": "<<result->data[i][1]<<"\n";
			float val = strtof(result->data[i][1].c_str(), nullptr);
			delete result;
			return val;
		}
	}
	delete result;
	return 0;
}

float calcScore(int userId, Sqlite3DB *db){
	float score = 	calcDist(userId, "bike", db)  * 10 + 
					calcDist(userId, "bus", db)   *  3 + 
					calcDist(userId, "car", db)   * -3 + 
					calcDist(userId, "plane", db) * -10 + 
					calcDist(userId, "train", db) *  1.5;

	return score;
}

float calcCO2(float dist, std::string vehicle){
	float co2 = 0.0f;
	if(vehicle == "bike") co2 = 0.0f;
	else if(vehicle == "bus") co2 = 20.0f;
	else if(vehicle == "car") co2 = 150.0f;
	else if(vehicle == "plane") co2 = 380.0f;
	else if(vehicle == "train") co2 = 40.0f;

	return dist * co2;
}

float calcCO2(std::string userName, std::string vehicle, Sqlite3DB *db){
	int userId = getUserId(userName, db);
	return calcCO2(calcDist(userId, vehicle, db), vehicle);
}
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
		querry<<"INSERT INTO users (name, password) VALUES (\'"<<userName<<"\', \'"<<password<<"\');";
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

	std::cout<<userName<<"\n"<<password<<"\n"<<distance<<"\n"<<vehicle<<"\n";

	cJSON_Delete(root);	
	Sqlite3DB *db = reinterpret_cast<Sqlite3DB*>(arg.arg);

	if(checkUser(userName, password, db)){
		int id = getUserId(userName, db);
		std::stringstream querry;
		querry<<"INSERT INTO tracks_"<<id<<" (distance, vehicle) VALUES ("<<distance<<", \'"<<vehicle<<" \');";
		dbResult *result = db->exec(querry.str());
		delete result;
	}
	else{
		return {HttpStatus_Forbidden,"application/json","{\"error\":\"login data invalid\"}"};
	}

	return {200, "application/json", "{\"status\":\"posted succsessful\"}"};
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


void __attribute__ ((constructor)) initPlugin(){
	Sqlite3DB db("data/main.db3");
	db.exec(File::readAll("./init.sql"));
}

void __attribute__ ((destructor)) clean(){
	;
}
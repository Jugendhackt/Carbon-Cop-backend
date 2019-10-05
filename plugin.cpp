#include "plugin.hpp"

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

	std::cout<<userName<<":"<<password<<"\n";

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
		return {HttpStatus_Forbidden, "text/plain", "user already exists"};
	}

	delete result;
	return {200, "text/plain", "signup succsessful"};
}


void __attribute__ ((constructor)) initPlugin(){
	Sqlite3DB db("data/main.db3");
	db.exec(File::readAll("./init.sql"));
}

void __attribute__ ((destructor)) clean(){
	;
}
Server for Carbon-Cop<br>
using:<br>
  * sqlite3 (https://www.sqlite.org),<br>
  * cJSON (https://github.com/DaveGamble/cJSON),<br>
  * tlse (https://github.com/eduardsui/tlse)<br>

API:<br>	
	-fail:<br>
		-forbidden(wrong userame / password)<br>
		-internal error<br>
<br>
	-create user:<br>
		-recv:<br>
			-username<br>
			-password<br>
		-send:<br>
			-already exists /<br>
			-success<br>
		-does:<br>
			-add user to DB/users if not exists<br>
			-create table "tracks_userid"<br>
<br>
	-get scores:<br>
		-recv:<br>
			-badget:<br>
				-name<br>
		-send:<br>
			-list of scores:<br>
				-count<br>
				-username<br>
				-rank<br>
<br>
	-get badgets:<br>
		-recv:<br>
			-username<br>
			-password<br>
		-send:<br>
			-fail /<br>
			-list of badgets:<br>
				-name<br>
				-description<br>
<br>
	-post track:<br>
		-recv:<br>
			-username<br>
			-password<br>
			-distance<br>
			-vehicle<br>
		-send:<br>
			-success /<br>
			-fail<br>
<br>
DB:<br>
	-users:<br>
		-id<br>
		-username<br>
		-password<br>
		-count of bagdet x<br>
<br>
	-tracks:		(per user: tracks_userid)<br>
		-distance<br>
		-vehicle<br>
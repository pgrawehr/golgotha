/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef G1_CLIENT_HH
#define G1_CLIENT_HH


#include "time/time.h"
#include "player_type.h"
#include "string/string.h"
#include "file/file.h"

// packet type  [ should be the first byte in each pack ]
enum
{
	G1_PK_I_WANNA_JOIN=128,
	G1_PK_YOU_HAVE_JOINED,
	G1_PK_WE_ARE_STARTING,
//  G1_PK_I_AM_READY,
	G1_PK_LOADING_DONE,
	G1_PK_GAME_START,
	G1_PK_PROCESS, //Not actually sent, merely a command to initiate sending
	G1_PK_GAME_DATA,
	G1_PK_ID_BASE=176, //All messages above this one are for the id manager
	G1_PK_ID_NEEDIDS,
	G1_PK_ID_HEREAREIDS,
	G1_PK_ID_IDACK,
	G1_PK_ID_RETURNINGIDS,
	G1_PK_ID_END
};



// sent to server

//  w16 [array of key down times in milliseconds]
//  troop build/movement commands, add on later..



// received from server
// w8 total_objects
//


class i4_net_address;
class i4_net_socket;
class i4_net_protocol;

class g1_client_class
{
	i4_net_address * server_address;
	i4_net_socket * listen;
	i4_net_socket * send;

	i4_time_class last_responce_time;
	int use_port;

	enum {
		JOIN_START,
		CONNECTING,
		LOADING,
		SYNCING,
		RUNNING
	} state;


	g1_player_type player_num;
	w32 num_players;
	i4_str * map_name;

public:
	g1_client_class(i4_net_address * server_address, int use_port, i4_net_protocol * protocol);
	void send_server(w8 * buf,i4_file_class * fp);
	int prepare_command(int command);
	int prepare_thinking(int for_who);
	int done_thinking();
	i4_bool poll();
	i4_bool is_ready();
	~g1_client_class();
};

extern g1_client_class * g1_client;

#endif

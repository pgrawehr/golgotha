/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_SERVER_HH
#define G1_SERVER_HH

#include "network/net_prot.h"
#include "network/net_addr.h"
#include "network/net_sock.h"
#include "g1_limits.h"
#include "time/time.h"
#include "file/ram_file.h"
#include "memory/hashtable.h"

#define PACKET(PACKET,R) w8 PACKET[MAX_PACKET_SIZE]; i4_ram_file_class R(PACKET,sizeof(PACKET));

extern i4_temp_file_class *network_file;
#define NE_ERROR_OK 0
#define NE_ERROR_NOTREADY -1
#define NE_ERROR_PAUSED -2
#define NE_ERROR_CONNECTIONLOST -3
#define NE_ERROR_NEEDRETRY -4
#define NE_ERROR_SERVERKILLED -5 
#define NE_ERROR_ID_MAN_MUST_SYNC -6
#define NE_ERROR_OUTOFIDS -7

class net_sync_entry{
	public:
	//w32 objid; //kept internally by the hashtable
	w32 ticksynced;
	w32 tickcalced;
	};

class g1_network_time_manager_class;
extern g1_network_time_manager_class *g1_network_time_man_ptr;
class g1_network_time_manager_class: public i4_init_class
	{
	public:
	w32 lasterror;

	enum timeerror
		{
		NO_NET_ERROR = 0,
		OUTOFDATE = 1,
		OUTOFSYNC = 2,
		OBJDOESNOTEXIST = 3,
		OWNSUPERTANK = 4
		};

    i4_hashtable<net_sync_entry> net_sync;
	g1_network_time_manager_class():net_sync(100,0)
		{
		lasterror=0;
		};
	i4_bool shouldupdate(w32 id, w32 totimestamp);
	void updatecomplete(w32 id, w32 attimestamp);
	void deletedobject(w32 id);
	virtual void init()
		{
		lasterror=0;
		g1_network_time_man_ptr=this;
		}
	virtual void uninit()
		{
		net_sync.reset(i4_T);
		}
	};

class g1_server_class
{
  friend class g1_server_start_window;
  friend int i4_network_send(w8 *buf, i4_file_class *fp, int send_to);
  enum {
	  NONE=0,
	  READY=1
	  };//client flags

  struct client
  {
    i4_net_address *addr;
    i4_time_class last_data;
    i4_str *username;
	int remote_player_num;
    i4_net_socket *send;
	w32 flags;
    
    void cleanup();
  };

  client clients[G1_MAX_PLAYERS];   // network address of each client

  i4_net_socket *udp_port;
  enum {
    WAITING_FOR_PLAYERS,
	LOADING,
	SYNCING,
    RUNNING,
    QUITING
  } state;
    
  i4_net_protocol *protocol;
  int local_player_num;
  void send_player_joined(int client_num);
  void process_client_packet(w8 *packet, int packet_length, int client_num);
  i4_str *map_name;
  i4_bool list_changed;

public:
  g1_server_class(int use_port, i4_net_protocol *protocol);
  void start_game();
  void send_to(w8 *buf, i4_file_class *f, int client_id);
  void send_to_all(w8 *buf, i4_file_class *f);
  int prepare_command(int command);
  int prepare_thinking(int for_who );
  int done_thinking();
  bool is_ready();
  void poll();  
  ~g1_server_class();
};

i4_bool i4_network_active();//is a network game running?
void i4_network_poll();
bool i4_network_mustwait();
void i4_network_shutdown();
int i4_network_prepare_command(int command);

//send_to is ignored if we are a client
//if we are the server, -1 sends to all clients
int i4_network_send(w8 *buf, i4_file_class *fp, int send_to);

extern class g1_server_class *g1_server;

#endif

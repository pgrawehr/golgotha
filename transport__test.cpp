#include "pch.h"
#include "transport/transport.h"
#include "init/init.h"
#include "main/main.h"
#include <conio.h>
node_manager * node_man;
link_manager * link_man;

//void *i4_stack_base=0;
//HINSTANCE i4_win32_instance=0;
int main(int argc, char * * argv)
{
	long t=0;

	i4_stack_base= (void *) (&t);
	i4_init();
	node_man=new node_manager();
	node_man->load_nodes("TNodes.dat");
	link_man=new link_manager(node_man);
	link_man->load_links("TLinks.dat");
	//scanf("\n");
	_getch();
	delete link_man;
	delete node_man;
	i4_uninit();
	return 0;
}

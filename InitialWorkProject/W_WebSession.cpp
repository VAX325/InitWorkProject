#include "P_HEADER.h"
#include "W_WebSession.h"

W_WebSession::W_WebSession()
{
	W_lastAccessedTime = 0;
	
	is_logged_in = false;
	is_admin = false;

	curr_page = 0;
	last_page = 0;

	open_page = -1;
}

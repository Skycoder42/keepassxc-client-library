#include "kpxcclient.h"
#include <sodium/randombytes.h>
#include <sodium/randombytes_sysrandom.h>
#include <sodium/core.h>

void KPXCClient::init()
{
	sodium_init();
	randombytes_set_implementation(&randombytes_sysrandom_implementation);
}

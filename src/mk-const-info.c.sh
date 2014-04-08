#!/bin/sh

cat > const-info.c << END_OF_HEAD
/* WARNING! This file is generated by the mk-const-info.c.sh program.
 * Do not edit manually!
 */
#include <getdns/getdns.h>
#include <stdlib.h>
#include "const-info.h"

static struct const_info consts_info[] = {
	{ -1, NULL, "/* <unknown getdns value> */" },
END_OF_HEAD
awk '/^[ 	]+GETDNS_[A-Z_]+[ 	]+=[ 	]+[0-9]+/{ print "\t{ "$3", \""$1"\", "$1"_TEXT }," }/^#define GETDNS_[A-Z_]+[ 	]+[0-9]+/ && !/^#define GETDNS_RRTYPE/ && !/_TEXT/{ print "\t{ "$3", \""$2"\", "$2"_TEXT },"}' getdns/getdns.h | sed 's/,,/,/g' >> const-info.c
cat >> const-info.c << END_OF_TAIL
};

static int const_info_cmp(const void *a, const void *b)
{
	return ((struct const_info *) a)->code - ((struct const_info *) b)->code;
}
struct const_info *
priv_getdns_get_const_info(int value)
{
	struct const_info key = { value, "", "" };
	struct const_info *i = bsearch(&key, consts_info,
	    sizeof(consts_info) / sizeof(struct const_info),
	    sizeof(struct const_info), const_info_cmp);
	if (i)
		return i;
	return consts_info;
}

END_OF_TAIL



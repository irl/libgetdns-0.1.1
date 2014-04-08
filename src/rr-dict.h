/**
 *
 * /brief getdns support functions for DNS Resource Records
 */
/*
 * Copyright (c) 2013, NLnet Labs, Verisign, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * * Neither the names of the copyright holders nor the
 *   names of its contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Verisign, Inc. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef RR_DICT_H_
#define RR_DICT_H_

#include <getdns/getdns.h>
#include <ldns/ldns.h>

getdns_return_t priv_getdns_create_dict_from_rr(
    struct getdns_context *context, ldns_rr *rr, struct getdns_dict** rr_dict);

getdns_return_t priv_getdns_create_reply_question_dict(
    struct getdns_context *context, ldns_pkt *pkt, struct getdns_dict** q_dict);

getdns_return_t priv_getdns_create_rr_from_dict(
    struct getdns_dict *rr_dict, ldns_rr **rr);

const char *priv_getdns_rr_type_name(int rr_type);

getdns_return_t priv_getdns_append_opt_rr(
    struct getdns_context *context, struct getdns_list* rdatas, ldns_pkt* pkt);

#endif

/* rrs.h */

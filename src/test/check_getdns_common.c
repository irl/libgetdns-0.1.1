/*
 * Copyright (c) 2013, NLNet Labs, Verisign, Inc.
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
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <check.h>
#include <getdns/getdns.h>
#include "config.h"
#include "check_getdns_common.h"
#include "check_getdns_eventloop.h"
#include <unistd.h>
#include <sys/time.h>

int callback_called = 0;
int callback_completed = 0;
int callback_canceled = 0;
uint16_t expected_changed_item = 0;
int event_loop_type = 0;

/*
 *  extract_response extracts all of the various information
 *  a test may want to look at from the response.
 */
void extract_response(struct getdns_dict *response, struct extracted_response *ex_response)
{
  ck_assert_msg(response != NULL, "Response should not be NULL");

  ASSERT_RC(getdns_dict_get_int(response, "answer_type", &ex_response->top_answer_type),
    GETDNS_RETURN_GOOD, "Failed to extract \"top answer_type\"");

  ASSERT_RC(getdns_dict_get_bindata(response, "canonical_name", &ex_response->top_canonical_name),
    GETDNS_RETURN_GOOD, "Failed to extract \"top canonical_name\"");

  ASSERT_RC(getdns_dict_get_list(response, "just_address_answers", &ex_response->just_address_answers),
    GETDNS_RETURN_GOOD, "Failed to extract \"just_address_answers\"");
  ck_assert_msg(ex_response->just_address_answers != NULL, "just_address_answers should not be NULL");

  ASSERT_RC(getdns_dict_get_list(response, "replies_full", &ex_response->replies_full),
    GETDNS_RETURN_GOOD, "Failed to extract \"replies_full\"");
  ck_assert_msg(ex_response->replies_full != NULL, "replies_full should not be NULL");

  ASSERT_RC(getdns_dict_get_list(response, "replies_tree", &ex_response->replies_tree),
    GETDNS_RETURN_GOOD, "Failed to extract \"replies_tree\"");
  ck_assert_msg(ex_response->replies_tree != NULL, "replies_tree should not be NULL");

  ASSERT_RC(getdns_list_get_dict(ex_response->replies_tree, 0, &ex_response->replies_tree_sub_dict),
    GETDNS_RETURN_GOOD, "Failed to extract \"replies_tree[0]\"");
  ck_assert_msg(ex_response->replies_tree_sub_dict != NULL, "replies_tree[0] dict should not be NULL");

  ASSERT_RC(getdns_dict_get_list(ex_response->replies_tree_sub_dict, "additional", &ex_response->additional),
    GETDNS_RETURN_GOOD, "Failed to extract \"additional\"");
  ck_assert_msg(ex_response->additional != NULL, "additional should not be NULL");

  ASSERT_RC(getdns_dict_get_list(ex_response->replies_tree_sub_dict, "answer", &ex_response->answer),
    GETDNS_RETURN_GOOD, "Failed to extract \"answer\"");
  ck_assert_msg(ex_response->answer != NULL, "answer should not be NULL");

  ASSERT_RC(getdns_dict_get_int(ex_response->replies_tree_sub_dict, "answer_type", &ex_response->answer_type),
    GETDNS_RETURN_GOOD, "Failed to extract \"answer_type\"");

  ASSERT_RC(getdns_dict_get_list(ex_response->replies_tree_sub_dict, "authority", &ex_response->authority),
    GETDNS_RETURN_GOOD, "Failed to extract \"authority\"");
  ck_assert_msg(ex_response->authority != NULL, "authority should not be NULL");

  ASSERT_RC(getdns_dict_get_bindata(ex_response->replies_tree_sub_dict, "canonical_name", &ex_response->canonical_name),
    GETDNS_RETURN_GOOD, "Failed to extract \"canonical_name\"");

  ASSERT_RC(getdns_dict_get_dict(ex_response->replies_tree_sub_dict, "header", &ex_response->header),
    GETDNS_RETURN_GOOD, "Failed to extract \"header\"");
  ck_assert_msg(ex_response->header != NULL, "header should not be NULL");

  ASSERT_RC(getdns_dict_get_dict(ex_response->replies_tree_sub_dict, "question", &ex_response->question),
    GETDNS_RETURN_GOOD, "Failed to extract \"question\"");
  ck_assert_msg(ex_response->question != NULL, "question should not be NULL");

  ASSERT_RC(getdns_dict_get_int(response, "status", &ex_response->status),
    GETDNS_RETURN_GOOD, "Failed to extract \"status\"");
}

/*
 *  assert_noerror asserts that the rcode is 0
 */
void assert_noerror(struct extracted_response *ex_response)
{
  uint32_t rcode;

  ASSERT_RC(ex_response->status, GETDNS_RESPSTATUS_GOOD, "Unexpected value for \"status\"");
  ASSERT_RC(getdns_dict_get_int(ex_response->header, "rcode", &rcode), GETDNS_RETURN_GOOD, "Failed to extract \"rcode\"");
  ck_assert_msg(rcode == 0, "Expected rcode == 0, got %d", rcode);
}

/*
 *  assert_nodata asserts that ancount in the header and the
 *  of the answer section (list) are both zero.
 */
void assert_nodata(struct extracted_response *ex_response)
{
  uint32_t ancount;
  size_t length;

  ASSERT_RC(getdns_dict_get_int(ex_response->header, "ancount", &ancount),
    GETDNS_RETURN_GOOD, "Failed to extract \"ancount\"");
  ck_assert_msg(ancount == 0, "Expected ancount == 0, got %d", ancount);

  ASSERT_RC(getdns_list_get_length(ex_response->answer, &length),
    GETDNS_RETURN_GOOD, "Failed to extract \"answer\" length");
  ck_assert_msg(length == 0, "Expected \"answer\" length == 0, got %d", length);
}

/*
 *  assert_address_records_in_answer asserts that ancount in the header
 *  is >= 1, ancount is equal to the length of "answer", and that all of
 *  the records in the answer section are A and/or AAAA resource records
 *  based on the value of the a/aaaa arguments.
 */
void assert_address_in_answer(struct extracted_response *ex_response, int a, int aaaa)
{
  uint32_t ancount;
  size_t length;
  struct getdns_dict *rr_dict;
  uint32_t type;
  uint32_t address_records = 0;
  size_t i;

  ASSERT_RC(getdns_dict_get_int(ex_response->header, "ancount", &ancount),
    GETDNS_RETURN_GOOD, "Failed to extract \"ancount\"");
  ck_assert_msg(ancount >= 1, "Expected ancount >= 1, got %d", ancount);

  ASSERT_RC(getdns_list_get_length(ex_response->answer, &length),
    GETDNS_RETURN_GOOD, "Failed to extract \"answer\" length");
  ck_assert_msg(length == ancount, "Expected \"answer\" length == ancount: %d, got %d", ancount, length);

  for(i = 0; i < length; i++)
  {
    ASSERT_RC(getdns_list_get_dict(ex_response->answer, i, &rr_dict),
      GETDNS_RETURN_GOOD, "Failed to extract \"answer\" record");
    ASSERT_RC(getdns_dict_get_int(rr_dict, "type", &type),
      GETDNS_RETURN_GOOD, "Failed to extract \"type\" from answer record");
    switch (type)
    {
      case GETDNS_RRTYPE_A:
        if(a && type == GETDNS_RRTYPE_A)
          address_records++;
      case GETDNS_RRTYPE_AAAA:
        if(aaaa && type == GETDNS_RRTYPE_AAAA)
          address_records++;
    }
  }
  ck_assert_msg(ancount == address_records, "ancount: %d address records mismatch: %d",
    ancount, address_records);
}

/*
 *  assert_nxdomain asserts that an NXDOMAIN response was
 *  was returned for the DNS query meaning:
 *  	rcode == 3
 */
void assert_nxdomain(struct extracted_response *ex_response)
{
  uint32_t rcode;

  ASSERT_RC(ex_response->status, GETDNS_RESPSTATUS_NO_NAME, "Unexpected value for \"status\"");
  ASSERT_RC(getdns_dict_get_int(ex_response->header, "rcode", &rcode), GETDNS_RETURN_GOOD, "Failed to extract \"rcode\"");
  ck_assert_msg(rcode == 3, "Expected rcode == 0, got %d", rcode);
}

/*
 *  assert_soa_in_authority asserts that a SOA record was
 *  returned in the authority sections.
 */
void assert_soa_in_authority(struct extracted_response *ex_response)
{
  uint32_t nscount;
  size_t length;
  struct getdns_dict *rr_dict;
  uint32_t type;
  uint32_t soa_records = 0;
  size_t i;

  ASSERT_RC(getdns_dict_get_int(ex_response->header, "nscount", &nscount),
    GETDNS_RETURN_GOOD, "Failed to extract \"nscount\"");
  ck_assert_msg(nscount >= 1, "Expected nscount >= 1, got %d", nscount);

  ASSERT_RC(getdns_list_get_length(ex_response->authority, &length),
    GETDNS_RETURN_GOOD, "Failed to extract \"authority\" length");
  ck_assert_msg(length == nscount, "Expected \"authority\" length == nscount: %d, got %d", nscount, length);

  for(i = 0; i < length; i++)
  {
    ASSERT_RC(getdns_list_get_dict(ex_response->authority, i, &rr_dict),
      GETDNS_RETURN_GOOD, "Failed to extract \"authority\" record");
    ASSERT_RC(getdns_dict_get_int(rr_dict, "type", &type),
      GETDNS_RETURN_GOOD, "Failed to extract \"type\" from authority record");
    if(type == GETDNS_RRTYPE_SOA)
      soa_records++;
  }

  ck_assert_msg(soa_records == 1, "Expected to find one SOA record in authority section, got %d", soa_records);
}

/*
 *  assert_ptr_in_answer asserts that a PTR record was
 *  returned in the answer sections.
 */
void assert_ptr_in_answer(struct extracted_response *ex_response)
{
  uint32_t ancount;
  size_t length;
  struct getdns_dict *rr_dict;
  uint32_t type;
  uint32_t ptr_records = 0;
  size_t i;

  ASSERT_RC(getdns_dict_get_int(ex_response->header, "ancount", &ancount),
    GETDNS_RETURN_GOOD, "Failed to extract \"nscount\"");
  ck_assert_msg(ancount >= 1, "Expected ancount >= 1, got %d", ancount);

  ASSERT_RC(getdns_list_get_length(ex_response->answer, &length),
    GETDNS_RETURN_GOOD, "Failed to extract \"answer\" length");
  ck_assert_msg(length == ancount, "Expected \"answer\" length == ancount: %d, got %d", ancount, length);

  for(i = 0; i < length; i++)
  {
    ASSERT_RC(getdns_list_get_dict(ex_response->answer, i, &rr_dict),
      GETDNS_RETURN_GOOD, "Failed to extract \"answer\" record");
    ASSERT_RC(getdns_dict_get_int(rr_dict, "type", &type),
      GETDNS_RETURN_GOOD, "Failed to extract \"type\" from answer record");
    if(type == GETDNS_RRTYPE_PTR)
      ptr_records++;
  }

  ck_assert_msg(ptr_records == 1, "Expected to find one PTR record in answer section, got %d", ptr_records);
}

void destroy_callbackfn(struct getdns_context *context,
                        getdns_callback_type_t callback_type,
                        struct getdns_dict *response,
                        void *userarg,
                        getdns_transaction_t transaction_id) {
    int* flag = (int*)userarg;
    *flag = 1;
    getdns_dict_destroy(response);
    getdns_context_destroy(context);
}

/*
 *  callbackfn is the callback function given to all
 *  asynchronous query tests.  It is expected to only
 *  be called for positive tests and will verify the
 *  response that is returned.
 */
void callbackfn(struct getdns_context *context,
                getdns_callback_type_t callback_type,
                struct getdns_dict *response,
                void *userarg,
                getdns_transaction_t transaction_id)
{
  typedef void (*fn_ptr)(struct extracted_response *ex_response);
  fn_ptr fn = userarg;

  /*
   *  If userarg is NULL, either a negative test case
   *  erroneously reached the query state, or the value
   *  in userarg (verification function) was somehow
   *  lost in transit.
   */
  ck_assert_msg(userarg != NULL, "Callback called with NULL userarg");

  /*
   *  We expect the callback type to be COMPLETE.
   */
  ASSERT_RC(callback_type, GETDNS_CALLBACK_COMPLETE, "Callback type");

  /*
  printf("DICT:\n%s\n", getdns_pretty_print_dict(response));
  */

  /*
   *  Extract the response.
   */
  EXTRACT_RESPONSE;

  /*
   *  Call the response verification function that
   *  was passed via userarg.
   */
  fn(&ex_response);

}

/*
 *  update_callbackfn is expected to only
 *  be called for positive tests and will verify the
 *  response that is returned.
 */
void update_callbackfn(struct getdns_context *context,
                getdns_context_code_t changed_item)
{

  ck_assert_msg(changed_item == expected_changed_item,
    "Expected changed_item == %d, got %d",
    changed_item, expected_changed_item);
}

void run_event_loop(struct getdns_context* context, void* eventloop) {
    run_event_loop_impl(context, eventloop);
}

void* create_event_base(struct getdns_context* context) {
    return create_eventloop_impl(context);
}
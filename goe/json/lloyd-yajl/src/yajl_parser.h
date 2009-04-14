/*
 * Copyright 2007, Lloyd Hilaiel.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 * 
 *  3. Neither the name of Lloyd Hilaiel nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */ 

#ifndef __YAJL_PARSER_H__
#define __YAJL_PARSER_H__

#include "api/yajl_parse.h"
#include "yajl_buf.h"

typedef enum {
    yajl_state_start = 0,
    yajl_state_parse_complete,
    yajl_state_parse_error,
    yajl_state_lexical_error,
    yajl_state_map_start,
    yajl_state_map_sep,    
    yajl_state_map_need_val,
    yajl_state_map_got_val,
    yajl_state_map_need_key,
    yajl_state_array_start,
    yajl_state_array_got_val,
    yajl_state_array_need_val
} yajl_state;

struct yajl_handle_t {
    const yajl_callbacks * callbacks;
    void * ctx;
    yajl_lexer lexer;
    const char * parseError;
    unsigned int errorOffset;
    /* temporary storage for decoded strings */
    yajl_buf decodeBuf;
    /* a stack of states.  access with yajl_state_XXX routines */
    yajl_buf stateBuf;
};

yajl_status
yajl_do_parse(yajl_handle handle, unsigned int * offset,
              const unsigned char * jsonText, unsigned int jsonTextLen);

unsigned char *
yajl_render_error_string(yajl_handle hand, const unsigned char * jsonText,
                         unsigned int jsonTextLen, int verbose);

yajl_state yajl_state_current(yajl_handle handle);

void yajl_state_push(yajl_handle handle, yajl_state state);

yajl_state yajl_state_pop(yajl_handle handle);

unsigned int yajl_parse_depth(yajl_handle handle);

void yajl_state_set(yajl_handle handle, yajl_state state);

#endif

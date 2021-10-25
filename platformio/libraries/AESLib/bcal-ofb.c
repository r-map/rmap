/* bcal-ofb.c */
/*
    This file is part of the AVR-Crypto-Lib.
    Copyright (C) 2010 Daniel Otte (daniel.otte@rub.de)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdint.h>
#include <string.h>
#include "bcal-ofb.h"
#include "bcal-basic.h"
#include "memxor.h"


uint8_t bcal_ofb_init(const bcdesc_t* desc, const void* key, uint16_t keysize_b, bcal_ofb_ctx_t* ctx){
	ctx->desc = (bcdesc_t*)desc;
	ctx->blocksize_B = (bcal_cipher_getBlocksize_b(desc)+7)/8;
	ctx->in_block=malloc(ctx->blocksize_B);
	if(ctx->in_block==NULL){
			return 0x11;
	}
	return bcal_cipher_init(desc, key, keysize_b, &(ctx->cctx));
}

void bcal_ofb_free(bcal_ofb_ctx_t* ctx){
	free(ctx->in_block);
	bcal_cipher_free(&(ctx->cctx));
}

void bcal_ofb_loadIV(const void* iv, bcal_ofb_ctx_t* ctx){
	if(iv){
		memcpy(ctx->in_block, iv, ctx->blocksize_B);
	}
}

void bcal_ofb_encNext(void* block, bcal_ofb_ctx_t* ctx){
	bcal_cipher_enc(ctx->in_block , &(ctx->cctx));
	memxor(block, ctx->in_block, ctx->blocksize_B);
}

void bcal_ofb_decNext(void* block, bcal_ofb_ctx_t* ctx){
	bcal_cipher_enc(ctx->in_block , &(ctx->cctx));
	memxor(block, ctx->in_block, ctx->blocksize_B);
}


void bcal_ofb_encMsg(const void* iv, void* msg, uint32_t msg_len_b, bcal_ofb_ctx_t* ctx){
	uint16_t block_len_b;
	block_len_b = ctx->blocksize_B*8;
	bcal_ofb_loadIV(iv, ctx);
	while(msg_len_b>block_len_b){
		bcal_ofb_encNext(msg, ctx);
		msg_len_b -= block_len_b;
		msg = (uint8_t*)msg + ctx->blocksize_B;
	}
	bcal_cipher_enc(ctx->in_block, &(ctx->cctx));
	ctx->in_block[msg_len_b/8] = 0xff00>>(msg_len_b&7);
	memxor(msg, ctx->in_block, (msg_len_b+7)/8);
}

void bcal_ofb_decMsg(const void* iv, void* msg, uint32_t msg_len_b, bcal_ofb_ctx_t* ctx){
	bcal_ofb_encMsg(iv, msg, msg_len_b, ctx);
}


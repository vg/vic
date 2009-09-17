/*
 * Copyright (c) 2008 University College London
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *  This product includes software developed by the MASH Research
 *  Group at the University of California Berkeley.
 * 4. Neither the name of the University nor of the Research Group may be
 *    used to endorse or promote products derived from this software without
 *    specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id$
 */

#include "assert.h"
#include "config.h"
#include "timer.h"
#include "rtp.h"
#include "inet.h"
#include "pktbuf-rtp.h"
#include "vic_tcl.h"
#include "module.h"
#include "transmitter.h"
#include "tfwc_rcvr.h"

TfwcRcvr::TfwcRcvr() :
	currseq_(0),
	prevseq_(0),
	ackofack_(0),
	begins_(0),
	ends_(0),
	currNumElm_(1),
	prevNumElm_(1),
	currNumVec_(1),
	prevNumVec_(1)
{
	// declare/initialize tfwcAV
	tfwcAV = (u_int16_t *) malloc(AVSZ * sizeof(u_int16_t));
	bzero(tfwcAV, AVSZ);
}

void TfwcRcvr::tfwc_rcvr_recv_aoa(u_int16_t type, u_int16_t *chunk, int num_chunks) 
{
	if (type == XR_BT_1) {
		// received ackofack 
		ackofack_ = ntohs(chunk[num_chunks-1]);
	}
	else if (type == XR_BT_2) {
		// set timestamp echo
		ts_echo_ = chunk[num_chunks-1];
	}
}

void TfwcRcvr::tfwc_rcvr_recv_seqno(u_int16_t seqno)
{
	// variables
	int numLoss		= 0;	// number of packet loss count
	int diffNumElm	= 0;	// difference of AckVec elements (curr vs. prev)
	int diffNumVec	= 0;	// difference of AckVec array (curr vs. prev)
	int addiNumVec	= 0;	// additional AckVec array required

	// received data packet seqno
	currseq_ = seqno;

	// number of AckVec element
	currNumElm_	= currseq_ - ackofack();
	diffNumElm	= currNumElm_ - prevNumElm_;
	int x = currNumElm_%BITLEN;
	int y = prevNumElm_%BITLEN;

	// number of chunks for building tfwcAV
	currNumVec_	= getNumVec(currNumElm_);
	diffNumVec	= currNumVec_ - prevNumVec_;

	// for debugging purpose
	printf("    [%s +%d] seqno:%d, ackofack:%d\n",
		__FILE__,__LINE__,currseq_,ackofack());
	printf("    [%s +%d] currNumElm:%d, prevNumElm:%d\n", 
		__FILE__,__LINE__,currNumElm_,prevNumElm_);
	printf("    [%s +%d] currNumVec:%d, prevNumVec:%d\n", 
		__FILE__,__LINE__,currNumVec_,prevNumVec_);

	// there is no packet loss (or reordering)
	if (currseq_ == prevseq_ + 1) {
		// we just need the same number of AckVec elements,
		// hence just left shift by one and clear the top bit
		if (diffNumElm == 0) {
			// set next bit to 1
			SET_BIT_VEC(tfwcAV[currNumVec_-1], 1);

			// and clear the top bit which we don't need it anymore
			if (x != 0)
				CLR_BIT_AT(tfwcAV[currNumVec_-1], x+1);
		}
		// we just need less number of AckVec elements,
		// hence first free unnecessary AckVec chunk(s) and set bit.
		else if (diffNumElm < 0) {
			// firstly, freeing unnecessary AcvVec chunk(s) 
			if (currNumVec_ != prevNumVec_) {
				for (int i = prevNumVec_; i > currNumVec_; i--) {
					for (int j = 1; j <= BITLEN; j++)
						SET_BIT_VEC(tfwcAV[i-1], 0);
				}
			}
			// set next bit to 1
			SET_BIT_VEC(tfwcAV[currNumVec_-1], 1);
			// and clear the bit(s) that we don't need it anymore
			int k = (x == 0) ? BITLEN: x;
			for (int i = BITLEN; i > k; i--)
				CLR_BIT_AT(tfwcAV[currNumVec_-1], i);
		}
		// otherwise, just set next bit to 1
		// (i.e., we need more AckVec elements)
		else
			SET_BIT_VEC(tfwcAV[currNumVec_-1], 1);
	} 
	// we have one or more packet losses (or reordering)
	else {
		// number of packet loss
		numLoss = currseq_ - prevseq_ - 1;
		int z = numLoss%BITLEN;

		// we need more AckVec chunks (maybe one or more)
		if (currNumVec_ > prevNumVec_) {
			// currently available spaces in the previous tfwcAV array
			int numAvail = BITLEN - y;

			// first, fill up zeros into those available spaces
			for (int i = 0; i < numAvail; i++) {
				SET_BIT_VEC(tfwcAV[prevNumVec_-1], 0);
				numLoss--;
			}

			// then, calculate "additional" AckVec chunks required
			addiNumVec = getNumVec(numLoss);

			// fill up zeros accordingly if addiNumVec is greater than 1
			for (int i = 0; i < (addiNumVec - 1); i++) {
				for (int j = 0; j < BITLEN; j++) {
					SET_BIT_VEC(tfwcAV[prevNumVec_+i], 0);
					numLoss--;
				}
			}

			// we need to update 'z' accordingly
			// (at this point, 'z' should be equal to 'numLoss')
			z = numLoss%BITLEN;

			// finally, fill up zeros at the latest AckVec chunk
			for (int i = 0; i < z; i++) {
				SET_BIT_VEC(tfwcAV[prevNumVec_+addiNumVec-1], 0);
			}
		}
		// current AckVeck chunk can cope with the elements
		else {
			// set next bit 0 into AckVec (# of packet loss)
			for (int i = 0; i < numLoss; i++) 
				SET_BIT_VEC(tfwcAV[currNumVec_-1], 0);
		}

		// then, set this packet as received (this is important)
		SET_BIT_VEC(tfwcAV[currNumVec_-1], 1);

		// and clear the top two bits which we don't need
		// (because we have pushed '0' and '1' at the end of this AckVec)
		// it doesn't really matter if diffNumElm is greater than 0.
		if ( (diffNumElm <= 0) && (x != 0) ) {
			int b = abs(diffNumElm) + x + z;
			for (int i = x + 1; i <= b; i++)
				CLR_BIT_AT(tfwcAV[currNumVec_-1], i);
		}
	}

	// print ackvec
	print_ackvec(tfwcAV);

	// start seqno that this AckVec is reporting
	if (ackofack() != 0)
		begins_ = ackofack() + 1;
	else
		begins_ = 1;

	// end seqno is current seqno plus one (according to RFC 3611)
	ends_ = currseq_ + 1;

	// store seqno, num of AckVec elem, and num of AckVec array
	prevseq_ = currseq_;
	prevNumElm_ = currNumElm_;
	prevNumVec_ = currNumVec_;
}

void TfwcRcvr::print_ackvec(u_int16_t *ackv) {
	// start sequence number
	int seqno = ackofack()+1;
	int x = currNumElm_%BITLEN;

	// printing...
	printf("\t>> AckVec: ");
	for (int i = 0; i < currNumVec_-1; i++) {
		printf("[%d] ( ", ackv[i]);
		for (int j = 0; j < BITLEN; j++) {
			if ( CHECK_BIT_AT(ackv[i], (j+1)) )
				printf("%d ", seqno);
			seqno++;
		}
	} printf (") ");

	int k = (x == 0) ? BITLEN: x;
	printf("[%d] ( ", ackv[currNumVec_-1]);
	for (int i = k; i > 0; i--) {
		if (CHECK_BIT_AT(ackv[currNumVec_-1], i))
			printf("%d ", seqno);
		seqno++;
	} printf(")...... %s +%d\n",__FILE__,__LINE__);
}

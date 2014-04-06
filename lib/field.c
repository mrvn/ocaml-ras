/* field.c - finite field math
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "field.h"

/* Finding a generator

void test16(uint16_t g) {
    uint16_t i = 1, n = 0;
    do {
	if (i & 0x8000) {
	    i = (i << 1) ^ g;
	} else {
	    i = (i << 1);
	}
	++n;
    } while(i != 1);
    if (n == 0xffff) printf("0x1%04x: 0x%04x\n", g, n);
}

int main() {
    uint32_t i;
    for(i = 1; i < 65536; i += 2) {
	test16(i);
    }
    return 0;
}
*/

// Lookup tables for log and exp
// exp is double the size so exp(log(a)+log(b)) works without overflow
static field_t exp_tbl[2 * FIELD_SIZE];
static field_t log_tbl[FIELD_SIZE];

// Initialize the f_exp and f_log arrays
void field_init() {
    field_t i;
    field2_t x = 1;

    for(i = 0; i < FIELD_SIZE - 1; ++i) {
	exp_tbl[i] = x;
	exp_tbl[i + FIELD_SIZE - 1] = x;
	log_tbl[x] = i;
	x <<= 1;
	if (x & FIELD_SIZE) x ^= FIELD_GENERATOR;
    }
    assert(x == 1);
    // Fill in remaining fields
    exp_tbl[2 * FIELD_SIZE - 2] = exp_tbl[0];
    exp_tbl[2 * FIELD_SIZE - 1] = exp_tbl[1];
    log_tbl[0] = FIELD_SIZE -1; // Bogus value
}

static inline field_t f_add(field_t x, field_t y) {
    return x ^ y;
}

static inline field_t f_sub(field_t x, field_t y) {
    return x ^ y;
}

static inline field_t f_log(field_t x) {
    return log_tbl[x];
}

static inline field_t f_exp(field2_t x) {
    return exp_tbl[x];
}

static inline field_t f_mul(field_t x, field_t y) {
    return f_exp((field2_t)f_log(x) + (field2_t)f_log(y));
}

// Initialize a multiplication row
void field_init_mul_row(field_t mul_row[FIELD_SIZE], field_t x) {
    field2_t i;
    mul_row[0] = 0;
    for(i = 1; i < FIELD_SIZE; ++i) {
	mul_row[i] = f_mul(x, i);
    }
}

// Compute coefficients for Lagrange polynomial interpolation
//      (x  - x0) (x  - x1) (x  - x2) ... (x  - xn)
// ci = -------------------------------------------
//      (xi - x0) (xi - x1) (xi - x2) ... (xi - xn)
// skipping the term (x - xi) / (xi - xn) where i == n
void field_init_coefficients(size_t count, field_t vec[],
			     field_t *points, field_t wanted) {
    size_t i, j;
    field_t xi, xj;
    field2_t numer = 0, denom;
    // accumulate the log of the numerator
    // include (x - xi)
    for(i = 0; i < count; ++i) {
	numer += f_log(f_sub(points[i], wanted));
    }
    numer = numer % (FIELD_SIZE - 1);
    // For each coefficient compute denominator
    for(i = 0; i < count; ++i) {
	xi = points[i];
	denom = 0;
	for(j = 0; j < count; ++j) {
	    // substitute (x - xi) for (xi - xj) when i == j
	    // cancles out the extra term in the numerator
	    xj = (i == j) ? wanted : points[j];
	    assert(xi != xj);
	    denom += f_log(f_sub(xi, xj));
	}
	// Compute log(numer / denom)
	denom = 2 * (FIELD_SIZE - 1) + numer - denom;
	denom = denom % (FIELD_SIZE - 1);
	// Save coefficient
	vec[i] = f_exp(denom);	
    }
}

// Compute multiplication matrix for coefficients
// Given yi [0 <= i < count]
// y_wanted = Sum(mul_tbl[i][yi]) [0 <= i < count]
void field_init_mul(size_t count, field_t mul_tbl[][FIELD_SIZE],
		    field_t *points, field_t wanted) {
    size_t i;
    // Compute coefficients
    field_t vec[count];
    field_init_coefficients(count, vec, points, wanted);
    // Compute multiplication rows
    for(i = 0; i < count; ++i) {
	field_init_mul_row(mul_tbl[i], vec[i]);
    }
}

// Evaluate polynomial over blocks of data
void field_poly8(size_t len, field_t *restrict res, size_t count,
		 field_t mul_tbl[][FIELD_SIZE],
		 field_t *restrict data[]) {
    size_t i, j;
    // Initialize with first block of data
    for(i = 0; i < len; i += 8) {
	res[i+0] = mul_tbl[0][data[0][i+0]];
	res[i+1] = mul_tbl[0][data[0][i+1]];
	res[i+2] = mul_tbl[0][data[0][i+2]];
	res[i+3] = mul_tbl[0][data[0][i+3]];
	res[i+4] = mul_tbl[0][data[0][i+4]];
	res[i+5] = mul_tbl[0][data[0][i+5]];
	res[i+6] = mul_tbl[0][data[0][i+6]];
	res[i+7] = mul_tbl[0][data[0][i+7]];
    }
    // Add other blocks of data
    for(i = 1; i < count; ++i) {
	for(j = 0; j < len; j += 8) {
	    res[j+0] = f_add(res[j+0], mul_tbl[i][data[i][j+0]]);
	    res[j+1] = f_add(res[j+1], mul_tbl[i][data[i][j+1]]);
	    res[j+2] = f_add(res[j+2], mul_tbl[i][data[i][j+2]]);
	    res[j+3] = f_add(res[j+3], mul_tbl[i][data[i][j+3]]);
	    res[j+4] = f_add(res[j+4], mul_tbl[i][data[i][j+4]]);
	    res[j+5] = f_add(res[j+5], mul_tbl[i][data[i][j+5]]);
	    res[j+6] = f_add(res[j+6], mul_tbl[i][data[i][j+6]]);
	    res[j+7] = f_add(res[j+7], mul_tbl[i][data[i][j+7]]);
	}
    }
}

void field_poly(size_t len, field_t *restrict res, size_t count,
		field_t mul_tbl[][FIELD_SIZE], field_t *restrict data[]) {
    size_t i, j;
    // Initialize with first block of data
    for(i = 0; i < len; ++i) {
	res[i] = mul_tbl[0][data[0][i]];
    }
    // Add other blocks of data
    for(i = 1; i < count; ++i) {
	for(j = 0; j < len; ++j) {
	    res[j] = f_add(res[j], mul_tbl[i][data[i][j]]);
	}
    }
}

void field_update(size_t len, field_t *restrict res,
		  field_t mul_tbl[][FIELD_SIZE], field_t row,
		  field_t *restrict old_data,
		  field_t *restrict new_data) {
    size_t i;
    // Sub old block and add new block of data
    for(i = 0; i < len; ++i) {
	res[i] = f_sub(res[i], mul_tbl[row][old_data[i]]);
	res[i] = f_add(res[i], mul_tbl[row][new_data[i]]);
    }
}

/*
int main2() {
    field_t data[9][FIELD_SIZE];
    int i, j;
    init();

    for(i = 0; i < FIELD_SIZE; ++i) {
	data[0][i] = i * 1;
	data[1][i] = i * 2;
	data[2][i] = i * 3;
	data[3][i] = i * 5;
    }

    // compute data[4]
    {
	field_t *pdata[4] = {data[0], data[1], data[2], data[3]};
	field_t mul_tbl[4][FIELD_SIZE];
	field_t points[4] = {0, 1, 2, 3};
	init_mul(4, mul_tbl, points, 4);
	poly(FIELD_SIZE, data[4], 4, mul_tbl, pdata);
    }
    
    // restore data[0] as data[5]
    {
	field_t *pdata[4] = {data[1], data[2], data[3], data[4]};
	field_t mul_tbl[4][FIELD_SIZE];
	field_t points[4] = {1, 2, 3, 4};
	init_mul(4, mul_tbl, points, 0);
	poly(FIELD_SIZE, data[5], 4, mul_tbl, pdata);
    }

    // restore data[1] as data[6]
    {
	field_t *pdata[4] = {data[0], data[2], data[3], data[4]};
	field_t mul_tbl[4][FIELD_SIZE];
	field_t points[4] = {0, 2, 3, 4};
	init_mul(4, mul_tbl, points, 1);
	poly(FIELD_SIZE, data[6], 4, mul_tbl, pdata);
    }

    // restore data[2] as data[7]
    {
	field_t *pdata[4] = {data[0], data[1], data[3], data[4]};
	field_t mul_tbl[4][FIELD_SIZE];
	field_t points[4] = {0, 1, 3, 4};
	init_mul(4, mul_tbl, points, 2);
	poly(FIELD_SIZE, data[7], 4, mul_tbl, pdata);
    }

    // restore data[3] as data[8]
    {
	field_t *pdata[4] = {data[0], data[1], data[2], data[4]};
	field_t mul_tbl[4][FIELD_SIZE];
	field_t points[4] = {0, 1, 2, 4};
	init_mul(4, mul_tbl, points, 3);
	poly(FIELD_SIZE, data[8], 4, mul_tbl, pdata);
    }

    // Output data
    printf("offset   0      1      2      3     parity r0     r1     r2     r3\n");
    for(i = 0; i < FIELD_SIZE; ++i) {
	printf("0x%04x: ", i);
	for(j = 0; j < 9; ++j) {
	    printf("0x%04x ", data[j][i]);
	}
	printf("\n");
	assert(data[0][i] == data[5][i]);
	assert(data[1][i] == data[6][i]);
	assert(data[2][i] == data[7][i]);
	assert(data[3][i] == data[8][i]);
    }
    
    return 0;
}

#define TOTAL (1024LLU*1024*1024*16)
// performance tests
//  8 Bits,  1G    :  27.44s user 0.00s system 99% cpu 27.446 total
// 16 Bits,  1G    :  15.52s user 0.00s system 99% cpu 15.531 total
// 16 Bits,  1G, 8x:  13.59s user 0.01s system 99% cpu 13.609 total
//  8 Bits, 16G    : 440.46s user 0.06s system 99% cpu 7:20.63 total
// 16 Bits, 16G    : 248.95s user 0.14s system 99% cpu 4:09.21 total
// 16 Bits, 16G, 8x: 220.11s user 0.02s system 99% cpu 3:40.23 total

int main() {
    field_t data[9][FIELD_SIZE];
    uint32_t i;
    init();

    for(i = 0; i < FIELD_SIZE; ++i) {
	data[0][i] = i * 1;
	data[1][i] = i * 2;
	data[2][i] = i * 3;
	data[3][i] = i * 5;
    }

    field_t *pdata[4] = {data[0], data[1], data[2], data[3]};
    field_t mul_tbl[4][FIELD_SIZE];
    field_t *pmul_tbl[4] = {mul_tbl[0], mul_tbl[1], mul_tbl[2], mul_tbl[3]};
    field_t points[4] = {0, 1, 2, 3};
    init_mul(4, mul_tbl, points, 4);
    
    for(i = 0; i < TOTAL / FIELD_SIZE / sizeof(field_t); ++i) {
	poly(FIELD_SIZE, data[4], 4, pmul_tbl, pdata);
    }
    
    return 0;
}
*/


/* Mail exchange with Nick Cleaton <nick@cleaton.net>:

From: Nick Cleaton <nick@cleaton.net>
Subject: Re: Re-licensing ras (Redundant Archive System) under LGPL-2.1+
To: Goswin von Brederlow <goswin-v-b@web.de>
Date: Tue, 6 Mar 2012 09:16:35 +0000

On 6 March 2012 07:30, Goswin von Brederlow <goswin-v-b@web.de> wrote:

> Now I'm thinking of possibly moving the redundancy calculations (just
> field.c from ras) into a library and I'm wondering if you would be
> willing to license the field.c file under the LGPL-2.1 (or any later
> version) for the purpose of making the library more compatible with
> other licenses.

I haven't looked at ras for many years.

Would you like to take over as maintainer ?  I think the maintainer
should be someone who is actively using the code.

I have no objection to LGPL for the library type files, or you could
change the whole thing to or more permissive license than GPL if you
prefer.  I don't have any strong opinions on which license to use.


Nick

*/

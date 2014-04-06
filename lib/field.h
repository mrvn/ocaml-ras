/* field.h - finite field math
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

#ifndef FIELD_H
#define FIELD_H

#include <stdint.h>
#include <unistd.h>

#define FILD_BITS 16
#if FIELD_BITS == 8
// Size of the finite field (8 bit)
#define FIELD_SIZE 256

// a random primitive polynomial of degree 8
#define FIELD_GENERATOR 0x169

// Type of members of the field
typedef uint8_t field_t;
// Next larger type for internal computations
typedef uint16_t field2_t;

#else

// Size of the finite field (16 bit)
#define FIELD_SIZE 65536

// a random primitive polynomial of degree 16, there are 2048 of them
#define FIELD_GENERATOR 0x17F0B

// Type of members of the field
typedef uint16_t field_t;
// Next larger type for internal computations
typedef uint32_t field2_t;

#endif

// Initialize the f_exp and f_log arrays
void field_init();

// Initialize a multiplication row
void field_init_mul_row(field_t mul_row[FIELD_SIZE], field_t x);

// Compute coefficients for Lagrange polynomial interpolation
//      (x  - x0) (x  - x1) (x  - x2) ... (x  - xn)
// ci = -------------------------------------------
//      (xi - x0) (xi - x1) (xi - x2) ... (xi - xn)
// skipping the term (x - xi) / (xi - xn) where i == n
void field_init_coefficients(size_t count, field_t vec[],
			     field_t *points, field_t wanted);

// Compute multiplication matrix for coefficients
// Given yi [0 <= i < count]
// y_wanted = Sum(mul_tbl[i][yi]) [0 <= i < count]
void field_init_mul(size_t count, field_t mul_tbl[][FIELD_SIZE],
		    field_t *points, field_t wanted);

// Evaluate polynomial over blocks of data
void field_poly8(size_t len, field_t *restrict res, size_t count,
		 field_t mul_tbl[][FIELD_SIZE],
		 field_t *restrict data[]);
void field_poly(size_t len, field_t *restrict res, size_t count,
		field_t mul_tbl[][FIELD_SIZE],
		field_t *restrict data[]);

// Update polynomial for row from old block to new block of data
void field_update(size_t len, field_t *restrict res,
		  field_t mul_tbl[][FIELD_SIZE], field_t row,
		  field_t *restrict old_data,
		  field_t *restrict new_data);
#endif

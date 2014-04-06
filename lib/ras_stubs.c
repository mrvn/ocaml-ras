/* ras_stubs.c - stub functions binding ras.ml to fields.c
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

#include "field.h"

#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/bigarray.h>

// Initialize the f_exp and f_log arrays
CAMLprim value field_init_stub() {
    CAMLparam0();
    field_init();
    CAMLreturn(Val_unit);
}

/*
// Initialize a multiplication row
CAMLprim value field_init_mul_row_stub(value ml_row, value ml_x) {
    CAMLparam2(ml_row, ml_x);
    field_t *row = (field_t*)Data_bigarray_val(ml_row);
    field_t x = Int_val(ml_x);
    field_init_mul_row(row, x);
    CAMLreturn(Val_unit);
}

// Compute coefficients for Lagrange polynomial interpolation
//      (x  - x0) (x  - x1) (x  - x2) ... (x  - xn)
// ci = -------------------------------------------
//      (xi - x0) (xi - x1) (xi - x2) ... (xi - xn)
// skipping the term (x - xi) / (xi - xn) where i == n
void field_init_coefficients(size_t count, field_t vec[],
			     field_t *points, field_t wanted);
*/

// Compute multiplication matrix for coefficients
// Given yi [0 <= i < count]
// y_wanted = Sum(mul_tbl[i][yi]) [0 <= i < count]
CAMLprim value field_init_mul_stub(value ml_count, value ml_matrix,
				   value ml_points, value ml_wanted) {
    CAMLparam4(ml_count, ml_matrix, ml_points, ml_wanted);
    int count = Int_val(ml_count);
    field_t (*matrix)[FIELD_SIZE] = (field_t (*)[FIELD_SIZE])Data_bigarray_val(ml_matrix);
    field_t *points = (field_t*)Data_bigarray_val(ml_points);
    field_t wanted = Int_val(ml_wanted);
    field_init_mul(count, matrix, points, wanted);
    CAMLreturn(Val_unit);
}

// Evaluate polynomial over blocks of data
CAMLprim value field_poly_stub(value ml_res, value ml_count,
			       value ml_matrix, value ml_data) {
    CAMLparam4(ml_res, ml_count, ml_matrix, ml_data);
    size_t len = caml_ba_byte_size(Caml_ba_array_val(ml_res)) / sizeof(field_t);
    size_t count = Int_val(ml_count);
    field_t *res = (field_t*)Data_bigarray_val(ml_res);
    field_t (*matrix)[FIELD_SIZE] = (field_t (*)[FIELD_SIZE])Data_bigarray_val(ml_matrix);
    field_t *data[count];
    for(unsigned int i = 0; i < count; ++i) {
	data[i] = (field_t*)Data_bigarray_val(Field(ml_data, i));
    }
    field_poly(len, res, count, matrix, data);
    CAMLreturn(Val_unit);
}

// Update polynomial at row from old to new block of data
CAMLprim value field_update_stub(value ml_res, value ml_matrix, value ml_row,
			       value ml_old_data, value ml_new_data) {
    CAMLparam5(ml_res, ml_row, ml_matrix, ml_old_data, ml_new_data);
    size_t len = caml_ba_byte_size(Caml_ba_array_val(ml_res)) / sizeof(field_t);
    field_t *res = (field_t*)Data_bigarray_val(ml_res);
    size_t row = Int_val(ml_row);
    field_t (*matrix)[FIELD_SIZE] = (field_t (*)[FIELD_SIZE])Data_bigarray_val(ml_matrix);
    field_t *old_data = (field_t *)Data_bigarray_val(ml_old_data);
    field_t *new_data = (field_t *)Data_bigarray_val(ml_new_data);
    field_update(len, res, matrix, row, old_data, new_data);
    CAMLreturn(Val_unit);
}

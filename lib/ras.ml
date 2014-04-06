(* field.c - finite field math
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
*)

external init : unit -> unit = "field_init_stub"
let () = init ()

module type M = sig
  val count : int
  val block_size : int
end

module type S = sig
  val field_bits : int

  val field_size : int

  val field_generator : int

  val count : int

  val block_size : int

  type points = private 
    (int, Bigarray.int16_unsigned_elt, Bigarray.c_layout) Bigarray.Array1.t

  type matrix = private
    (int, Bigarray.int16_unsigned_elt, Bigarray.c_layout) Bigarray.Array2.t

  type block = private
    (int, Bigarray.int16_unsigned_elt, Bigarray.c_layout) Bigarray.Array1.t

  type blocks = private block array

  type field_t = int

  val points_get : points -> int -> field_t

  val points_set : points -> int -> field_t -> unit

  val block_get : block -> int -> field_t

  val block_set : block -> int -> field_t -> unit

  val points_create : unit -> points

  val make_matrix : points -> field_t -> matrix

  val block_create : unit -> block

  val blocks_create : block -> blocks

  val blocks_make : block array -> blocks

  val compute : matrix -> blocks -> block

  val update : block -> matrix -> int -> block -> block -> unit

  module P : sig
    module Bigarray : sig
      module Array1 : sig
	val get : points -> int -> field_t
	val set : points -> int -> field_t -> unit
      end
    end
  end

  module B : sig
    module Bigarray : sig
      module Array1 : sig
	val get : block -> int -> field_t
	val set : block -> int -> field_t -> unit
      end
    end
  end
end
  
module MAKE = functor (M : M) ->
  (struct
    (* keep in sync with field.h *)
    let field_bits = 16
    let field_size = 65536
    let field_generator = 0x17F0B
    let count = M.count
    let block_size = M.block_size

    type points = (int, Bigarray.int16_unsigned_elt, Bigarray.c_layout) Bigarray.Array1.t
    type matrix = (int, Bigarray.int16_unsigned_elt, Bigarray.c_layout) Bigarray.Array2.t
    type block = (int, Bigarray.int16_unsigned_elt, Bigarray.c_layout) Bigarray.Array1.t
    type blocks = block array
    type field_t = int

    let points_get points i = points.{i}
    let points_set points i x = points.{i} <- x
    let block_get block i = block.{i}
    let block_set block i x = block.{i} <- x
      
    let points_create () =
      Bigarray.Array1.create
	Bigarray.int16_unsigned
	Bigarray.c_layout
	M.count
     
    external init_mul : int -> matrix -> points -> field_t -> unit = "field_init_mul_stub"

    let make_matrix points wanted =
      let matrix =
	Bigarray.Array2.create
	  Bigarray.int16_unsigned
	  Bigarray.c_layout
	  field_size
	  M.count
      in
      init_mul M.count matrix points wanted;
      matrix

    let block_create () =
      Bigarray.Array1.create
	Bigarray.int16_unsigned
	Bigarray.c_layout
	M.block_size
     
    let blocks_create block = Array.create M.count block

    let blocks_make blocks =
      assert (Array.length blocks = M.count);
      blocks
  
    external field_poly : block -> int -> matrix -> blocks -> unit = "field_poly_stub"

    let compute matrix blocks =
      let res = block_create ()
      in
      field_poly res M.count matrix blocks;
      res

    external field_update : block -> matrix -> int -> block -> block -> unit = "field_update_stub"

    let update block matrix row old_data new_data =
      field_update block matrix row old_data new_data

    module P = struct
      module Bigarray = struct
	module Array1 = struct
	  let get arr i = points_get arr i
	  let set arr i x = points_set arr i x
	end
      end
    end

    module B = struct
      module Bigarray = struct
	module Array1 = struct
	  let get arr i = block_get arr i
	  let set arr i x = block_set arr i x
	end
      end
    end
   end : S)

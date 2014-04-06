module M = struct
  let count = 4
  let block_size = 10
end
module Ras = Ras.MAKE(M)
module P = Ras.P
module B = Ras.B
  
let make_data x =
  let res = Ras.block_create ()
  in
  for i = 0 to Ras.block_size - 1 do
    B.(res.{i} <- (i * x));
  done;
  res

let block0 = make_data 1
let block1 = make_data 2
let block2 = make_data 3
let block3 = make_data 4

let points0123 =
  let res = Ras.points_create ()
  in
  for i = 0 to Ras.count - 1 do
    P.(res.{i} <- i);
  done;
  res

let matrix0123 = Ras.make_matrix points0123 4
let blocks = Ras.blocks_make [| block0; block1; block2; block3; |]
let block4 = Ras.compute matrix0123 blocks

let points1234 =
  let res = Ras.points_create ()
  in
  for i = 0 to 4 - 1 do
    P.(res.{i} <- (i + 1));
  done;
  res

let matrix1234 = Ras.make_matrix points1234 0
let blocks = Ras.blocks_make [| block1; block2; block3; block4; |]
let block0b = Ras.compute matrix1234 blocks

let () =
  Printf.printf "% 5d | % 5d | % 5d | % 5d || %5dX || % 5d restored\n" 0 1 2 3 4 0;
  Printf.printf "------+-------+-------+-------++--------++---------------\n";
  for i = 0 to Ras.block_size - 1 do
    Printf.printf "% 5d | % 5d | % 5d | % 5d || % 5d  || % 5d\n"
      B.(block0.{i})
      B.(block1.{i})
      B.(block2.{i})
      B.(block3.{i})
      B.(block4.{i})
      B.(block0b.{i});
    assert (B.(block0.{i}) = B.(block0b.{i}));
  done;
  Printf.printf "\n"

let block1b = make_data 5
let () = Ras.update block4 matrix0123 1 block1 block1b
let blocks = Ras.blocks_make [| block1b; block2; block3; block4; |]
let block0c = Ras.compute matrix1234 blocks

let () =
  Printf.printf "% 5d | % 5d | % 5d | % 5d || %5dX || % 5d restored\n" 0 1 2 3 4 0;
  Printf.printf "------+-------+-------+-------++--------++---------------\n";
  for i = 0 to Ras.block_size - 1 do
    Printf.printf "% 5d | % 5d | % 5d | % 5d || % 5d  || % 5d\n"
      B.(block0.{i})
      B.(block1b.{i})
      B.(block2.{i})
      B.(block3.{i})
      B.(block4.{i})
      B.(block0c.{i});
    assert (B.(block0.{i}) = B.(block0c.{i}));
  done

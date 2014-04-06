module type S = sig
  type t = private int
  val t : t
  val get : unit -> int
end;;
let fresh x = (module (struct type t = int let t = x let get () = x end) : S);;
let t1 = fresh 5 
let t2 = fresh 6;;
module T1 = (val t1 : S);;
let t1' = T1.t;;
module T2 = (val t2 : S);;
let t2' = T2.t;;

type 'a arr = int array;;
let make : type a . (module S with type t = a) -> a array = fun m ->
  let module M = (val m : S) in
  Array.create M.t 0;;

let make m =
  let module M = (val m : S) in
  let arr = Array.create (M.get ()) 0
  in
  (arr : M.t arr);;

let arr1 = make T1;;
let arr2 = make T2;;

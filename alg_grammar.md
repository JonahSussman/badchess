piece   -> 'P' | 'R' | 'N' | 'B' | 'Q' | 'K'

takes   -> 'x'

rank    -> '1'-'8'
file    -> 'a'-'h'

target  -> rank file

disamb  -> rank
        |  file
        |  rank file

promo   -> '=' ('N' | 'B' | 'R' | 'Q')

check   -> '+' | '#'

move    -> (piece) (disamb) rhs
rhs     -> target (promo) (check)

target takes higher precedence, but is right-associative

//move    -> (piece) (disamb) (takes) target (promo) (check)

#! /usr/bin/env python
#
#  Non-terminal symbols of Python grammar (from "graminit.h")
#
#  This file is automatically generated; please don't muck it up!
#
#  To update the symbols in this file, 'cd' to the top directory of
#  the python source tree after building the interpreter and run:
#
#    python Lib/symbol.py

#--start constants--
single_input = 256
file_input = 257
eval_input = 258
funcdef = 259
parameters = 260
varargslist = 261
fpdef = 262
fplist = 263
stmt = 264
simple_stmt = 265
small_stmt = 266
expr_stmt = 267
print_stmt = 268
del_stmt = 269
pass_stmt = 270
flow_stmt = 271
break_stmt = 272
continue_stmt = 273
return_stmt = 274
raise_stmt = 275
import_stmt = 276
dotted_name = 277
global_stmt = 278
exec_stmt = 279
assert_stmt = 280
compound_stmt = 281
if_stmt = 282
while_stmt = 283
for_stmt = 284
try_stmt = 285
except_clause = 286
suite = 287
test = 288
and_test = 289
not_test = 290
comparison = 291
comp_op = 292
expr = 293
xor_expr = 294
and_expr = 295
shift_expr = 296
arith_expr = 297
term = 298
factor = 299
power = 300
atom = 301
lambdef = 302
trailer = 303
subscriptlist = 304
subscript = 305
sliceop = 306
exprlist = 307
testlist = 308
dictmaker = 309
classdef = 310
arglist = 311
argument = 312
#--end constants--

sym_name = {}
for _name, _value in globals().items():
    if type(_value) is type(0):
        sym_name[_value] = _name


def main():
    import sys
    import token
    if len(sys.argv) == 1:
        sys.argv = sys.argv + ["Include/graminit.h", "Lib/symbol.py"]
    token.main()

if __name__ == "__main__":
    main()

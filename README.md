# cmm-toy-compiler

A project to learn about compilers

## Example of output:

./main --ast --token --symbols samples/c.cmm

```
int sum(int a, int b);

int main(void) {
    int res;
    res = sum(1, 2);
    return 0;
}

int sum(int a, int b) {
    return a + b;
}
==================================== Tokens ====================================
[1: TOKEN_INT         ]  'int'
[1: TOKEN_IDENT       ]  'sum'
[1: TOKEN_LEFT_PAREN  ]  '('
[1: TOKEN_INT         ]  'int'
[1: TOKEN_IDENT       ]  'a'
[1: TOKEN_COMMA       ]  ','
[1: TOKEN_INT         ]  'int'
[1: TOKEN_IDENT       ]  'b'
[1: TOKEN_RIGHT_PAREN ]  ')'
[1: TOKEN_SEMICOLON   ]  ';'
[3: TOKEN_INT         ]  'int'
[3: TOKEN_IDENT       ]  'main'
[3: TOKEN_LEFT_PAREN  ]  '('
[3: TOKEN_VOID        ]  'void'
[3: TOKEN_RIGHT_PAREN ]  ')'
[3: TOKEN_LEFT_BRACE  ]  '{'
[4: TOKEN_INT         ]  'int'
[4: TOKEN_IDENT       ]  'res'
[4: TOKEN_SEMICOLON   ]  ';'
[5: TOKEN_IDENT       ]  'res'
[5: TOKEN_EQUAL       ]  '='
[5: TOKEN_IDENT       ]  'sum'
[5: TOKEN_LEFT_PAREN  ]  '('
[5: TOKEN_NUMBER      ]  '1'
[5: TOKEN_COMMA       ]  ','
[5: TOKEN_NUMBER      ]  '2'
[5: TOKEN_RIGHT_PAREN ]  ')'
[5: TOKEN_SEMICOLON   ]  ';'
[6: TOKEN_RETURN      ]  'return'
[6: TOKEN_NUMBER      ]  '0'
[6: TOKEN_SEMICOLON   ]  ';'
[7: TOKEN_RIGHT_BRACE ]  '}'
[9: TOKEN_INT         ]  'int'
[9: TOKEN_IDENT       ]  'sum'
[9: TOKEN_LEFT_PAREN  ]  '('
[9: TOKEN_INT         ]  'int'
[9: TOKEN_IDENT       ]  'a'
[9: TOKEN_COMMA       ]  ','
[9: TOKEN_INT         ]  'int'
[9: TOKEN_IDENT       ]  'b'
[9: TOKEN_RIGHT_PAREN ]  ')'
[9: TOKEN_LEFT_BRACE  ]  '{'
[10: TOKEN_RETURN      ]  'return'
[10: TOKEN_IDENT       ]  'a'
[10: TOKEN_PLUS        ]  '+'
[10: TOKEN_IDENT       ]  'b'
[10: TOKEN_SEMICOLON   ]  ';'
[11: TOKEN_RIGHT_BRACE ]  '}'
[12: TOKEN_EOF         ]  ''
================================================================================

================================= Symbol Table =================================
Scope: <global>
    function | type: int    | sym: main                 | n_params: 0
    function | type: int    | sym: sum                  | n_params: 2
--------------------------------------------------------------------------------
Scope: main
    variable | type: int    | sym: res                 
--------------------------------------------------------------------------------
Scope: sum
    variable | type: int    | sym: a                   
    variable | type: int    | sym: b                   
================================================================================

========================== Abstract Syntax Tree (AST) ==========================
Line: 1 ->   stmts: (stmtslist)
Line: 1 ->     stmt: (funcdecl) type: int
Line: 1 ->       ident: (ident) value: 'sum'
Line: 1 ->       params: (paramdecl_list)
Line: 1 ->         param: (paramdecl) type: int 
Line: 1 ->           ident: (ident) value: 'a'
Line: 1 ->         param: (paramdecl) type: int 
Line: 1 ->           ident: (ident) value: 'b'
Line: 3 ->     stmt: (funcdecl) type: int
Line: 3 ->       ident: (ident) value: 'main'
Line: 4 ->       stmts: (stmtslist)
Line: 4 ->         stmt: (vardecl) type: int 
Line: 4 ->           ident: (ident) value: 'res'
Line: 5 ->         stmt: (assign)
Line: 5 ->           left: (ident) value: 'res'
Line: 5 ->           right: (funccall)
Line: 5 ->             ident: (ident) value: 'sum'
Line: 5 ->             params: (param_list)
Line: 5 ->               param: (int) value: 1
Line: 5 ->               param: (int) value: 2
Line: 6 ->         stmt: (return)
Line: 6 ->           expr: (int) value: 0
Line: 9 ->     stmt: (funcdecl) type: int
Line: 9 ->       ident: (ident) value: 'sum'
Line: 9 ->       params: (paramdecl_list)
Line: 9 ->         param: (paramdecl) type: int 
Line: 9 ->           ident: (ident) value: 'a'
Line: 9 ->         param: (paramdecl) type: int 
Line: 9 ->           ident: (ident) value: 'b'
Line: 10 ->       stmts: (stmtslist)
Line: 10 ->         stmt: (return)
Line: 10 ->           expr: (binop) op: +
Line: 10 ->             left: (ident) value: 'a'
Line: 10 ->             right: (ident) value: 'b'
================================================================================
```

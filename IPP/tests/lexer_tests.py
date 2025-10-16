from src.lexer import Lexer

big_code = """ 
class Main : Object {
    run "<- definice metody - bezparametrický selektor run"
        [ |
            " zaslání zprávy 'compute :and:and:' sobě samému - selektor se dvěma arg."
            x := self compute : 3 and: 2 and: 5.
            " zaslání zprávy 'plusOne :' sobě samému - selektor s jedním arg.
            Argumentem je výsledek po zaslání zprávy 'vysl ' objektu self."
            x := self plusOne : (self vysl).
            " zaslání zprávy 'asString ' objektu x - bezparam . selektor "
            y := x asString .
        ]

    plusOne : "<- definice metody - selektor s jedním parametrem "
        [ :x | r := x plus: 1. ]

    compute :and:and: "<- definice metody - selektor se třemi parametry "
        [ :x :y :z |
            " zaslání zpr. 'plus:' objektu x - selektor s jedním argumentem "
            a := x plus: y.
            " zaslání zpr. 'vysl:' sobě samému - nastaví instanční atribut 'vysl '"
            _ := self vysl: a.
            "zpráva 'vysl ' se zašle sobě , výsledkem je ref. na objekt vysl;
            tomuto objektu se pak zašle zpráva 'greaterThan :' s arg. 0."
            _ := (( self vysl) greaterThan : 0)
            " výsledkem je objekt typu True nebo False , kterému se zašle zpráva
            'ifTrue : ifFalse :', argumenty jsou bezparametrické bloky"
            ifTrue: [|u := self vysl: 1.]
            ifFalse : [|].
        ]
}

"""


# x := self compute: 3 and: 2 and: 5.
source_code = """
    class Main : Object {
        run [ |
            x := self compute: 3 and: 2.
            y := x plusOne: (self vysl).
        ]
       }
    """
mini_code= """
class Main : Object {
    run [ |
        x := 10 .
    ]
}
"""

lexer = Lexer()
tokens = lexer.tokenize(source_code)

for token in tokens:
    print(token)

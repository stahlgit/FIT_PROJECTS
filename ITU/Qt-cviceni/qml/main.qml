import QtQuick 2.1
import QtQuick.Window 2.0
import "../js/Theme.js" as Theme
import cz.vutbr.fit 1.0

Window {
    visible: true
    width: 400
    height: 338
    
    title: "ITU - Qt 5 / QML kalkulačka"

    // Definování datového modelu s operátory
    // 'op' - zkratka pro operaci
    // 'tog' - zkratka pro toggled, označení, která operace je vybrána
    ListModel {
        id: operations;
        ListElement { op: "+"; tog: false; }
        ListElement { op: "-"; tog: true; }
        // TODO
        // Rozšiřte model o další dvě základní matematické operace 

        ListElement { op: "*"; tog: false; }
        ListElement { op: "/"; tog: false; }
    }

    // TOG is toggled --> vybraný

    // Prvek pro rozvržení prvků do sloupce
    // http://en.wikipedia.org/wiki/Layout_%28computing%29
    // https://qmlbook.github.io/ch04-qmlstart/qmlstart.html#positioning-elements
    Column {

        // Vstupní hodnota - první operand výpočtu
        Rectangle {
            height: 35;
            width: 400;
            border.color: "#bbb";
            border.width: 3;
            anchors.margins: 2
            color: "#777"


            TextInput {
                anchors.fill: parent;
                anchors.margins: 2
                horizontalAlignment: TextInput.AlignLeft
                verticalAlignment: TextInput.AlignVCenter
                id: textA
                font.pointSize: 22
                text: "0"
                
            }
        }

        // Prvek pro rozvržení prvků do řádku
        // Více jak prvek Column (výše)
        Row {
            // Obdoba ListView (ale více obecný) nebo funkce foreach()
            // obsahuje _model_ a _delegate_
            Repeater {
                // Definování modelu, data pro zobrazení
                model: operations;

                // Delegování vzhledu v MVC
                // Jak má vypadat jeden prvek
                // @disable-check M301
                delegate: MyButton {
                    btnColor: Theme.btn_colour
                    
                    text: model.op
                    toggled: model.tog;
                    
                    onClicked: {
                        for (var i = 0; i < operations.count; i++) {
                            operations.setProperty( i, "tog", (i == index) );
                        }
                        slider.rectColor = "steelBlue"
                        result.color = "steelBlue"
                        textA.color = "black"
                    }
                }
            }

        }

        // "Vlastní" třída pro posuvník. Definice v MySlider.qml
        // @disable-check M301
        MySlider {
            id: slider
            color: Qt.darker(Theme.slider_color)
            rectColor: Theme.slider_color

            onValueChanged: {
                if (rectColor === "red") {
                    rectColor = Theme.slider_color
                }
            }
        }

        // TODO _ DONE
        // vložte nový textový prvek, který bude bude vizuálně 'zapadat'
        // do výsledné aplikace a bude zobrazoval vertikálně vycentrovaný
        // text 'LUT value: ' a hodnotu aktuálně vybrané položky z LUT

        Text {
            id: lutValueText
            text: "LUT value " + lut.getValue(slider.value)
            font.pixelSize: 16
            color: "#000000"
        }

        // Vlastní klikací tlačítko. Definice v MyClickButton.qml
        // @disable-check M301
        MyClickButton {
            width: 400;
            btnColor: Theme.btn_colour
            
            text: qsTr( "Compute" )
            
            function getOperation() {
                for (var i = 0; i < operations.count; i++) {
                    var item = operations.get(i);
                    if (item.tog) {
                        return item.op;
                    }
                }
                return "+";
            }

            // Obsluha události při zachycení signálu clicked
            onClicked: {
                var a = parseFloat(textA.text, 10);
                var error = false;
                var errorMsg = ""

                // TODO
                // Zkontrolujte jestli funkce parseFloat vrátila 
                // korektní výsledek (tj. ne NaN, ale číslo). Pokud 
                // je hodnota a NaN, změňte barvu vstupního pole
                // na červenou a vypište chybu do pole pro výsledek

                if (isNaN(a)){
                    textA.color = "red"
                    error = true
                    errorMsg = "CHYBA: neplatný vstup pre A"
                }
                else
                {
                    textA.color = "#000000"
                }
                
                // TODO
                // Upravte načtení hodnoty b tak, aby získal hodnotu b
                // z LUT (Look Up Table) podle vybrané hodnoty na 'slider'
                var b = lut.getValue(slider.value);

                // TODO
                // pokud se uživatel pokouší dělit nulou, změňte barvu
                // posuvníku na slideru na červenou a vypište chybu
                // do pole pro výsledek

                if (isNaN(b)){
                    textA.color = "red"
                    error = true
                    errorMsg = "CHYBA: neplatný vstup pre B"
                }


                var op = getOperation();
                console.log( a + " "+ op + " " + b + " = ?")

                
                // TODO
                // Vypočítejte výslednou hodnotu danou operandy a, b
                // a operátorem op, výsledek uložte do prvku result

                if (error) {
                    //textA.color = "red"
                    result.text = errorMsg
                    result.color = "red"
                    return
                }

                var calculationResult
                switch(op){
                case "+":
                    calculationResult = a + b
                    break

                case "-":
                    calculationResult = a - b
                    break
                case "*":
                    calculationResult = a * b
                    break
                case "/":
                    if ( b === 0) {
                        slider.rectColor = "red"
                        result.text = "CHYBA: delenie nulou"
                        result.color = "red"
                        return
                    }

                    calculationResult = a / b
                    break
                default:
                    calculationResult = NaN
                }

                result.text = calculationResult
            }
        }

        // Prvek pro zobrazení výsledné hodnoty
        Rectangle {
            height: 45;
            width: 400;
            border.color: "#bbb";
            border.width: 3;
            anchors.margins: 2
            color: "#777"
            
            Text {
                id:  result;
                anchors.centerIn: parent
                height: 35;
                font.pointSize: 22
                color: "#0066FF"
                text:"Result"
            }
        }

    }

    // Vytvoření objektu LUT, který je definován v C++
    // K danému se přistupuje pomocí jeho id
    LUT {
        id: lut
    }

}


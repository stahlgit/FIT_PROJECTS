/*
 * IJA (Seminář Java): 2024/25 Ukol 2
 * Spuštění presentéru (vizualizace) implementace modelu hry.
 * Author:  Radek Kočí, VUT FIT
 * Created: 03/2025
 */
package ija.ija2024.homework2;

import java.util.logging.Level;
import java.util.logging.Logger;

//--- Importy z implementovaneho reseni ukolu
import ija.ija2024.homework2.common.Position;
import ija.ija2024.homework2.common.Side;
import static ija.ija2024.homework2.common.Side.*;
import ija.ija2024.homework2.game.Game;

//--- Importy z baliku dodaneho nastroje
import ija.ija2024.tool.EnvPresenter;
//--- 

/**
 * Třída spustí vizualizaci implementace hry. 
 * Prezentér je implementován třídou ija.ija2024.tool.EnvPresenter, dále využívá prostředky definované 
 * v balíku ija.ija2024.tool, který je součástí dodaného nástroje.
 * Vytvoří hru, zobrazí ji v GUI a umožní otáčení políček z GUI.
 * @author Radek Kočí
 */
public class Homework2 {

    /**
     * Definice obsazených políček ve hře. Použito pro vytvoření testovací hry.
     * def_field = {Type, row, col, list_of_connectors}
     */
    private static final Object def[][]= {
        {"L", 4, 5, NORTH, EAST, SOUTH},
        {"L", 5, 5, NORTH, EAST, WEST},
        {"L", 5, 4, EAST, SOUTH},
        {"L", 4, 6, EAST, SOUTH},
        {"L", 5, 6, NORTH, SOUTH},
        {"L", 3, 6, EAST, WEST},
        {"L", 3, 4, EAST, WEST},
        {"L", 5, 7, EAST, SOUTH},
        {"B", 6, 4, NORTH},
        {"B", 3, 3, NORTH},
        {"B", 2, 6, SOUTH},
        {"B", 4, 7, WEST},
        {"P", 3, 5, EAST, SOUTH}
    };    
    
    /**
     * Vytvoří hru, otevře prezentér a provede sekvenci předem stanovených kroků.
     * @param args Seznam argumentů při spuštění.
     */
    public static void main(String... args) {
        
        Game game = Game.create(10, 12);

        for (Object[] n : def) {
            String type = (String)n[0];
            int row = (Integer)n[1];
            int col = (Integer)n[2];
            Position p = new Position(row, col);
            Side sides[] = new Side[n.length-3];
            for (int i = 3; i < n.length; i++) {
                sides[i-3] = (Side)n[i];
            }
            switch (type) {
                case "L" -> game.createLinkNode(p, sides);
                case "B" -> game.createBulbNode(p, sides[0]);
                case "P" -> game.createPowerNode(p, sides);
            }
        }
        
        game.init();
       
        EnvPresenter presenter = new EnvPresenter(game);
        presenter.open();
        
        turn(800, game,
                new Position(3,5),
                new Position(3,3),
                new Position(3,6),
                new Position(5,6),
                new Position(5,7),
                new Position(5,7),
                new Position(4,7),
                new Position(4,7),
                new Position(4,7),
                new Position(4,6),
                new Position(4,6),
                new Position(4,5)
            );        
    }

    /**
     * Provede sekvenci kroků (otočení) nad hrou. Mezi kroky je prodleva podle zadané hodnoty.
     * @param ms Prodleva v ms.
     * @param game Testovací hra.
     * @param pos Sekvence pozic, jejichž políčka se mají otočit (pomocí metody turn).
     */
    public static void turn(int ms, Game game, Position... pos) {
        for (Position p : pos) {
            sleep(ms);
            game.node(p).turn();
        }
    }
    
    /**
     * Uspani vlakna na zadany pocet ms.
     * @param ms Pocet ms pro uspani vlakna.
     */
    public static void sleep(int ms) {
        try {
            Thread.sleep(ms);
        } catch (InterruptedException ex) {
            Logger.getLogger(Homework2.class.getName()).log(Level.SEVERE, null, ex);
        }        
    }
}

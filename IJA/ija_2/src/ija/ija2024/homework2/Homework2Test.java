/*
 * IJA (Seminář Java): 2024/25 Ukol 2
 * Author:  Radek Kočí, VUT FIT
 * Created: 03/2025
 */
package ija.ija2024.homework2;

import ija.ija2024.homework2.common.GameNode;
import ija.ija2024.homework2.common.Position;
import ija.ija2024.homework2.common.Side;
import static ija.ija2024.homework2.common.Side.EAST;
import static ija.ija2024.homework2.common.Side.NORTH;
import static ija.ija2024.homework2.common.Side.SOUTH;
import static ija.ija2024.homework2.common.Side.WEST;
import ija.ija2024.homework2.game.Game;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.AfterAll;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;

/**
 *
 * @author Radek Koci <koci AT fit.vut.cz>
 */
public class Homework2Test {
    
    /**
     * Hra pro testování.
     */
    private Game game;

    /**
     * Definice obsazených políček ve hře. Použito pro vytvoření testovací hry.
     * def_field = {Type, row, col, list_of_connectors}
     */
    private final Object def[][]= {
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
     * Vytvoří hru včetně políček podle definice v atributu {@link #def} a provede její inicializaci.
     */
    @BeforeEach
    public void setUp() {
        game = Game.create(10, 12);
        
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

        // Inicializuje hru (overi, ze je pouze jeden zdroj, alespon jedna zarovka a rozsviti policka napojena na zdroj).
        game.init();
    }

    /**
     * Test vytvoření a inicializce hry - správně rozsvícená políčka.
     * 2b.
     */
    @Test
    public void test01() {
        Integer lights[][]= {
            {3, 5},
            {4, 5},
            {5, 5},
            {3, 6},
            {5, 4},
            {6, 4}
        };                
        this.testLight(lights);
    }

    /**
     * Test vytvoření a inicializce hry - správná reprezentace všech políček.
     * Využívá implementaci metody toString.
     * 2b.
     */
    @Test
    public void test02() {
        for (Object[] n : def) {
            String type = (String)n[0];
            int row = (Integer)n[1];
            int col = (Integer)n[2];
            Position p = new Position(row, col);
            StringBuilder sides = new StringBuilder();
            for (int i = 3; i < n.length; i++) {
                sides.append(n[i].toString());
                if (i+1 < n.length) sides.append(",");
            }
            GameNode node = game.node(p);
            assertEquals("{" + type + "["+row+"@"+col+"][" + sides + "]}", node.toString(), "Test spravne reprezentace stavu policka.");              
        }        
    }

    /**
     * Test otočení políčka - správně rozsvícená políčka.
     * 2b.
     */
    @Test
    public void test03() {
        Position pos;
        GameNode node;
        
        pos = new Position(3, 5);
        node = game.node(pos);
        node.turn();

        Integer lights[][]= {
            {3, 5},
            {4, 5},
            {5, 5},
            {3, 4},
            {5, 4},
            {6, 4}
        };                
        this.testLight(lights);
    }

    /**
     * Test otočení políčka - správná reprezentace políčka.
     * 2b.
     */
    @Test
    public void test04() {
        Position pos;
        GameNode node;
        
        pos = new Position(3, 5);
        node = game.node(pos);
        assertEquals("{P[3@5][EAST,SOUTH]}", node.toString(), "Test spravne reprezentace stavu policka.");   
        node.turn();

        for (Object[] n : def) {
            String type = (String)n[0];
            int row = (Integer)n[1];
            int col = (Integer)n[2];
            Position p = new Position(row, col);
            node = game.node(p);
            if (p.equals(pos)) {
                assertEquals("{P[3@5][SOUTH,WEST]}", node.toString(), "Test spravne reprezentace stavu policka.");   
                continue;
            }
            StringBuilder sides = new StringBuilder();
            for (int i = 3; i < n.length; i++) {
                sides.append(n[i].toString());
                if (i+1 < n.length) sides.append(",");
            }
            assertEquals("{" + type + "["+row+"@"+col+"][" + sides + "]}", node.toString(), "Test spravne reprezentace stavu policka.");              
        }      
        
    }
   
    /**
     * Test otočení políček - správná reprezentace políčka, správně rozsvícená políčka.
     * 2b.
     */
    @Test
    public void test05() {
        Position pos1, pos2, pos3, pos4;
        GameNode node;
        
        pos1 = new Position(3, 5);
        node = game.node(pos1);
        node.turn();
        pos2 = new Position(5, 5);
        node = game.node(pos2);
        node.turn();
        pos3 = new Position(5, 6);
        node = game.node(pos3);
        node.turn();
        pos4 = new Position(3, 3);
        node = game.node(pos4);
        node.turn();

        Integer lights[][]= {
            {3, 5},
            {4, 5},
            {5, 5},
            {5, 6},
            {3, 4},
            {3, 3}
        };                
        this.testLight(lights);
        
        assertEquals("{P[3@5][SOUTH,WEST]}", game.node(pos1).toString(), "Test spravne reprezentace stavu policka.");   
        assertEquals("{L[5@5][NORTH,EAST,SOUTH]}", game.node(pos2).toString(), "Test spravne reprezentace stavu policka.");   
        assertEquals("{L[5@6][EAST,WEST]}", game.node(pos3).toString(), "Test spravne reprezentace stavu policka.");   
        assertEquals("{B[3@3][EAST]}", game.node(pos4).toString(), "Test spravne reprezentace stavu policka.");   
    }

    /**
     * Pomocná testovací metoda - ověří svítivost políček. 
     * Ověří, zda zadaná políčka svítí a zda všechna ostatní políčka nesvítí.
     * @param lights Pole souřadnic [row,col] políček, která mají svítit.
     */
    private void testLight(Integer[][] lights) { 
        List<Position> allpos = new ArrayList<>();
        for (int r = 1; r <= game.rows(); r++) {
            for (int c = 1; c <= game.cols(); c++) {
                allpos.add(new Position(r, c));
            }
        }

        for (Integer[] coord : lights) {
            int row = coord[0];
            int col = coord[1];
            Position pos = new Position(row, col);    
            allpos.remove(pos);
            GameNode node = game.node(pos);
            assertTrue(node.light(), "Políčko " + pos + " má být pod proudem.");
        }
        
        for (Position pos : allpos) {
            GameNode node = game.node(pos);
            assertFalse(node.light(), "Políčko " + pos + " nemá být pod proudem.");
        }        
    }
}

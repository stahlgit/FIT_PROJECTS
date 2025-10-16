/*
 * IJA (Seminář Java): 2024/25 Ukol 1
 * Author:  Radek Kočí, VUT FIT
 * Created: 02/2025
 */
package ija.ija2024.homework1;

import ija.ija2024.homework1.common.GameNode;
import ija.ija2024.homework1.common.Position;
import ija.ija2024.homework1.common.Side;
import ija.ija2024.homework1.game.Game;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;


/**
 * IJA 2024/25, úkol 1, testovací třída, 
 * @author Radek Koci <koci AT fit.vut.cz>
 */
public class Homework1Test {
    
    /**
     * Test vytvoření prostředí (hry) s prázdnými políčky.
     * 2b.
     */
    @Test
    public void test01() {
        Game game;

        /* Chybne vytvoreni hry, generuje vyjimku.
         */
        assertThrows(
            IllegalArgumentException.class,
            () -> { Game.create(-1, 8); },
            "Chybne vytvoreni hry, musi generovat vyjimku."
        );
        
        /* Vytvori hru s rozmerem [15,10].        
        */
        game = Game.create(15, 10);
        assertEquals(15, game.rows(), "Test spravneho poctu radku.");
        assertEquals(10, game.cols(), "Test spravneho poctu sloupcu.");
        
        /* Overi spravne vytvorena prazdna policka.
        */
        for(int r = 1; r <= game.rows(); r++) {
            for(int c = 1; c <= game.cols(); c++) {
                GameNode node;
                Position p;
                p = new Position(r,c);
                node = game.node(p);
                assertFalse(node.isBulb() || node.isLink() || node.isPower(), "Test spravneho typu policka.");
                for(Side s : Side.values()) {
                    this.checkConnectors(node);
                }
            }
        }
    }
    
    /**
     * Test vytvoření jednoho typu políčka - žárovka. 
     * 2b.
     */
    @Test
    public void test02() {
        Game game;
        GameNode node;
        Position p;

        game = Game.create(15, 10);

        /* Chybne umisteni policka, vraci null.
        */
        node = game.createBulbNode(new Position(16,1), Side.NORTH);
        assertNull(node);

        /* Test vytvoreni policka
        */
        node = game.createBulbNode(new Position(2,1), Side.NORTH);
        assertFalse(node.isLink() || node.isPower(), "Test spravneho typu policka.");
        assertTrue(node.isBulb(), "Test spravneho typu policka.");
        p = new Position(2,1);
        assertEquals(p, node.getPosition());        
        this.checkConnectors(node, Side.NORTH);

        /* Test vytvoreni druheho policka
        */
        node = game.createBulbNode(new Position(2,5), Side.WEST);
        this.checkConnectors(node, Side.WEST);
    }
    
    /**
     * Test vytvoření jednoho typu políčka - vodič.
     * 2b.
     */
    @Test
    public void test03() {
        Game game;
        GameNode node;
        Position p;

        game = Game.create(15, 10);

        /* Chybne umisteni policka, vraci null.
        */
        node = game.createLinkNode(new Position(16,1), Side.NORTH, Side.WEST);
        assertNull(node);

        /* Chybny pocet konektoru, vraci null.
        */
        node = game.createLinkNode(new Position(6,1), Side.NORTH);
        assertNull(node);

        /* Test vytvoreni policka
        */
        node = game.createLinkNode(new Position(8,2), Side.EAST, Side.SOUTH, Side.WEST);
        assertFalse(node.isBulb() || node.isPower(), "Test spravneho typu policka.");
        assertTrue(node.isLink(), "Test spravneho typu policka.");
        p = new Position(8,2);
        assertEquals(p, node.getPosition());
        this.checkConnectors(node, Side.EAST, Side.SOUTH, Side.WEST);        

        /* Test vytvoreni druheho policka
        */
        node = game.createLinkNode(new Position(3,2), Side.EAST, Side.SOUTH);
        this.checkConnectors(node, Side.EAST, Side.SOUTH);        
    }
    
    /**
     * Test vytvoření jednoho typu políčka - zdroj.
     * 2b.
     */
    @Test
    public void test04() {
        Game game;
        GameNode node;
        Position p;

        game = Game.create(15, 10);

        /* Chybne umisteni policka, vraci null.
        */
        node = game.createPowerNode(new Position(16,1), Side.NORTH);
        assertNull(node);
        /* Chybny pocet konektoru, vraci null.
        */
        node = game.createPowerNode(new Position(6,1));
        assertNull(node);

        /* Test vytvoreni policka
        */
        node = game.createPowerNode(new Position(2,8), Side.NORTH, Side.SOUTH);
        assertFalse(node.isBulb() || node.isLink(), "Test spravneho typu policka.");
        assertTrue(node.isPower(), "Test spravneho typu policka.");
        p = new Position(2,8);
        assertEquals(p, node.getPosition());
        this.checkConnectors(node, Side.NORTH, Side.SOUTH);        

        /* Zdroj jiz existuje, nelze vytvorit dalsi, vraci null.
        */
        node = game.createPowerNode(new Position(2,9), Side.WEST);
        assertNull(node);
    }
    
    /**
     * Test otáčení políček.
     * 2b.
     */
    @Test
    public void test05() {
        Game game;
        GameNode node;
        Position p;

        /* Vytvori hru a nekolik policek.
        */
        game = Game.create(15, 10);
        game.createPowerNode(new Position(2,8), Side.NORTH, Side.WEST);
        game.createLinkNode(new Position(8,2), Side.EAST, Side.SOUTH, Side.WEST);
        game.createLinkNode(new Position(12,8), Side.EAST, Side.WEST);
        game.createBulbNode(new Position(2,1), Side.NORTH);

        /* Testuje otaceni policka na pozici [2,8].
        */
        node = game.node(new Position(2,8));
        this.checkConnectors(node, Side.NORTH, Side.WEST);        
        node.turn();
        this.checkConnectors(node, Side.NORTH, Side.EAST);        
        node.turn();
        this.checkConnectors(node, Side.EAST, Side.SOUTH);        

        /* Testuje otaceni policka na pozici [8,2].
        */
        node = game.node(new Position(8,2));
        this.checkConnectors(node, Side.EAST, Side.SOUTH, Side.WEST);        
        node.turn();
        this.checkConnectors(node, Side.NORTH, Side.SOUTH, Side.WEST);        

        /* Testuje otaceni policka na pozici [12,8].
        */
        node = game.node(new Position(12,8));
        this.checkConnectors(node, Side.EAST, Side.WEST);        
        node.turn();
        this.checkConnectors(node, Side.NORTH, Side.SOUTH);        

        /* Testuje otaceni policka na pozici [2,1].
        */
        node = game.node(new Position(2,1));
        this.checkConnectors(node, Side.NORTH);        
        node.turn();
        this.checkConnectors(node, Side.EAST);        
    }
    
    /**
     * Ověří, zda jsou správně nastavené konektory u políčka.
     * @param node Tetsované políčko
     * @param sides Seznam stran, které mají obsahovat konektor. Zbývající strany musí být prázdné.
     */
    private void checkConnectors(GameNode node, Side... sides) {
        List<Side> all = new ArrayList<>(Arrays.asList(Side.values()));
        for (Side s : sides) {
            all.remove(s);
            assertTrue(node.containsConnector(s), "Test spravne obsazenych konektoru policka: " + node + " --> " + s);
        }
        for (Side s : all) {
            assertFalse(node.containsConnector(s), "Test spravne obsazenych konektoru policka: " + node + " --> " + s);
        }
    }
}

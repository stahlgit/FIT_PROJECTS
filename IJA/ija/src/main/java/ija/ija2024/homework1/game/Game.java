package ija.ija2024.homework1.game;

import ija.ija2024.homework1.common.*;

import java.util.HashMap;
import java.util.Map;

public class Game {
    private final int rows; // cannot be cancelled after initialization
    private final int cols;
    private final Map<Position, GameNode> board;

    public Game(int rows, int cols) {
        if (rows <= 0 || cols <= 0) throw new IllegalArgumentException("Invalid dimensions");
        this.rows = rows;
        this.cols = cols;
        this.board = new HashMap<>();

        for (int r = 1; r <= rows; r++) {
            for (int c = 1; c <= cols; c++) {
                Position position = new Position(r, c);
                board.put(position, new EmptyNode(position));
            }
        }
    }

    public static Game create(int rows, int cols) {
        return new Game(rows, cols);
    }

    public int rows() {
        return rows;
    }

    public int cols() {
        return cols;
    }

    public GameNode node(Position p) {
        return board.get(p);
    }

    public GameNode createBulbNode(Position position, Side connectors) {
        if (!isValidPosition(position)) return null;
        BulbNode node = new BulbNode(position, connectors);
        board.put(position, node);
        return node;
    }

    public GameNode createLinkNode(Position position, Side... connectors) {
        if (!isValidPosition(position) || connectors.length < 2) return null;
        LinkNode node = new LinkNode(position, connectors);
        board.put(position, node);
        return node;
    }

    public GameNode createPowerNode(Position position, Side... connectors) {
        if (!isValidPosition(position) || connectors.length < 1) return null;
        if (hasPowerNode()) return null; // Ensure only one PowerNode exists
        PowerNode node = new PowerNode(position, connectors);
        board.put(position, node);
        return node;
    }

    private boolean isValidPosition(Position p) {
        return p.row() > 0 && p.col() > 0 && p.row() <= rows && p.col() <= cols;
    }

    private boolean hasPowerNode() {
        return board.values().stream().anyMatch(GameNode::isPower);
    }
}
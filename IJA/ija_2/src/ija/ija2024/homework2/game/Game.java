package ija.ija2024.homework2.game;

import ija.ija2024.homework2.common.*;
import ija.ija2024.tool.common.Observable;
import ija.ija2024.tool.common.ToolEnvironment;
import ija.ija2024.tool.common.ToolField;

import java.util.*;

public class Game implements ToolEnvironment, Observable.Observer {
    private final int rows; // cannot be cancelled after initialization
    private final int cols;
    private final Map<Position, GameNode> board;
    private boolean isRecalculating = false;

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
        node.addObserver(this);
        board.put(position, node);
        return node;
    }

    public GameNode createLinkNode(Position position, Side... connectors) {
        if (!isValidPosition(position) || connectors.length < 2) return null;
        LinkNode node = new LinkNode(position, connectors);
        node.addObserver(this);
        board.put(position, node);
        return node;
    }

    public GameNode createPowerNode(Position position, Side... connectors) {
        if (!isValidPosition(position) || connectors.length < 1) return null;
        if (hasPowerNode()) return null; // Ensure only one PowerNode exists
        PowerNode node = new PowerNode(position, connectors);
        node.addObserver(this);
        board.put(position, node);
        return node;
    }

    private boolean isValidPosition(Position p) {
        return p.row() > 0 && p.col() > 0 && p.row() <= rows && p.col() <= cols;
    }

    private boolean hasPowerNode() {
        return board.values().stream().anyMatch(GameNode::isPower);
    }

    @Override
    public ToolField fieldAt(int row, int col) {
        Position position = new Position(row, col);
        if(!isValidPosition(position)) {
            return null;
        }
        return board.get(position); //GameNode must implement tool field
    }

    private Position getAdjacentPosition(Position p, Side s) {
        int row = p.row();
        int col = p.col();
        switch (s){
            case NORTH:
                row --;
                break;
            case SOUTH:
                row ++;
                break;
            case EAST:
                col ++;
                break;
            case WEST:
                col --;
                break;
        }
        return new Position(row, col);
    }

    private Side getOpposite(Side side) {
        switch (side) {
            case NORTH:
                return Side.SOUTH;
            case SOUTH:
                return Side.NORTH;
            case WEST:
                return Side.EAST;
            case EAST:
                return Side.WEST;
            default:
                throw new IllegalArgumentException("Invalid side");
        }
    }

    private void performLightPropagation(){
        GameNode powerNode = null;
        // get powerNode
        for(GameNode node : board.values()) {
            if(node.isPower()){
                powerNode = node;
                powerNode.setLit(true);
                break;
            }
        }
        // check if exists
        if (powerNode == null) {
            return;
        }
        // Logic for traversing
        Queue<Map.Entry<GameNode, Side>> queue = new LinkedList<>();
        Map<Position, Set<Side>> visited = new HashMap<>();

        for (Side powerSide : powerNode.getConnectors()){
            // Get all position that are adjacent to powerNode
            Position adjacentPos = getAdjacentPosition(powerNode.getPosition(), powerSide);
            if (!isValidPosition(adjacentPos)) {
                continue;
            }
            GameNode adjacentNode = board.get(adjacentPos);
            if(adjacentNode == null) {
                continue;
            }
            // get opposite side of connecting node
            Side requiredSide = getOpposite(powerSide);
            if(adjacentNode.containsConnector(requiredSide)){
                queue.add(new AbstractMap.SimpleEntry<>(adjacentNode, requiredSide));
                visited.computeIfAbsent(adjacentPos, k -> new HashSet<>()).add(requiredSide);
            }
        }
        // traverse queue
        while (!queue.isEmpty()) {
            Map.Entry<GameNode, Side> entry = queue.poll();
            GameNode currentNode = entry.getKey();
            Side incomingSide = entry.getValue();
            // light up
            currentNode.setLit(true);

            if(currentNode.isBulb()){ // bulb should be dead end    
                continue;
            }
            for(Side connector : currentNode.getConnectors()){
                if(connector == incomingSide){
                    continue;
                }

                Position nextPos = getAdjacentPosition(currentNode.getPosition(), connector);
                if(!isValidPosition(nextPos)) {
                    continue;
                }
                GameNode nextNode = board.get(nextPos);
                if(nextNode == null) {
                    continue;
                }

                Side requiredNextSide = getOpposite(connector);
                if(!nextNode.containsConnector(requiredNextSide)){
                    continue;
                }
                Set<Side> visitedSides = visited.getOrDefault(nextPos, new HashSet<>());
                if(visitedSides.contains(requiredNextSide)){
                    continue;
                }
                queue.add(new AbstractMap.SimpleEntry<>(nextNode, requiredNextSide));
                visitedSides.add(requiredNextSide);
                visited.put(nextPos, visitedSides);
            }
        }
    }

    public void recalculateLight(){
        isRecalculating = true;
        for(GameNode node : board.values()) {
            node.setLit(false);
        }
        performLightPropagation();
        isRecalculating = false;
    }

    @Override
    public void update(Observable observable) {
        // ignore recalculations calls during
        if(!isRecalculating) {
            recalculateLight(); // Recalculate light when any node changes
        }
    }

    public void init(){
        long powerNodes = board.values().stream().filter(GameNode::isPower).count();
        if (powerNodes != 1){ //TODO check if true
            throw new IllegalArgumentException("Exactly one power node is required");
        }
        long bulbNodes = board.values().stream().filter(GameNode::isBulb).count();
        if (bulbNodes < 1){
            throw new IllegalArgumentException("At least one BulbNode is required");
        }
        recalculateLight();
    }
}
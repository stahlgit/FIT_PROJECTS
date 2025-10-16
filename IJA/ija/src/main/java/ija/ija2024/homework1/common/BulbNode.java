package ija.ija2024.homework1.common;

import java.util.HashSet;
import java.util.Set;

public class BulbNode extends GameNode {
    private final Set<Side> connectors;

    public BulbNode(Position position, Side side) {
        super(position);
        this.connectors = new HashSet<>();
        this.connectors.add(side);
    }

    @Override
    public boolean isBulb() {
        return true;
    }

    @Override
    public boolean isLink() {
        return false;
    }

    @Override
    public boolean isPower() {
        return false;
    }

    @Override
    public void turn() {
        // Rotate the connector
        Side current = connectors.iterator().next();
        connectors.clear();
        connectors.add(switch (current) {
            case NORTH -> Side.EAST;
            case SOUTH -> Side.WEST;
            case WEST -> Side.NORTH;
            case EAST -> Side.SOUTH;
        });
    }

    @Override
    public boolean containsConnector(Side s) {
        return connectors.contains(s);
    }
}
package ija.ija2024.homework1.common;

import java.util.HashSet;
import java.util.Set;

public class PowerNode extends GameNode {
    private Set<Side> connectors;

    public PowerNode(Position position, Side... connectors) {
        super(position);
        if (connectors.length < 1) {
            throw new IllegalArgumentException("PowerNode must have at least one connector");
        }
        this.connectors = new HashSet<>(Set.of(connectors));
    }

    @Override
    public boolean isPower() {
        return true;
    }

    @Override
    public boolean isLink() {
        return false;
    }

    @Override
    public boolean isBulb() {
        return false;
    }

    @Override
    public void turn() {
        Set<Side> newConnectors = new HashSet<>();
        for (Side s : connectors) {
            newConnectors.add(switch (s) {
                case NORTH -> Side.EAST;
                case SOUTH -> Side.WEST;
                case WEST -> Side.NORTH;
                case EAST -> Side.SOUTH;
            });
        }
        connectors = newConnectors;
    }

    @Override
    public boolean containsConnector(Side s) {
        return connectors.contains(s);
    }
}
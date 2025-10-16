package ija.ija2024.homework1.common;

import java.util.HashSet;
import java.util.Set;
import java.util.stream.Collectors;

public class LinkNode extends GameNode {
    private Set<Side> connectors;

    public LinkNode(Position position, Side... connectors) {
        super(position);
        if (connectors.length < 2) {
            throw new IllegalArgumentException("LinkNode must have at least two connectors");
        }
        this.connectors = new HashSet<>(Set.of(connectors));
    }

    @Override
    public boolean isLink() {
        return true;
    }

    @Override
    public boolean isBulb() {
        return false;
    }

    @Override
    public boolean isPower() {
        return false;
    }

    @Override
    public void turn() {
        // other way to do it using streamline solution
        connectors = connectors.stream().map(s -> switch (s) {
            case NORTH -> Side.EAST;
            case SOUTH -> Side.WEST;
            case WEST -> Side.NORTH;
            case EAST -> Side.SOUTH;
        }).collect(Collectors.toSet());
    }

    @Override
    public boolean containsConnector(Side s) {
        return connectors.contains(s);
    }
}
package ija.ija2024.homework2.common;

import java.util.StringJoiner;

public class BulbNode extends GameNode {

    public BulbNode(Position position, Side side) {
        super(position);
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
        notifyObservers();
    }

    @Override
    public String toString() {
        StringJoiner sj = new StringJoiner(",");
        for (Side side : new Side[] {Side.NORTH, Side.EAST, Side.SOUTH, Side.WEST}) {
            if (containsConnector(side)) {
                sj.add(side.name());
            }
        }
        return String.format("{B[%d@%d][%s]}", position.row(), position.col(), sj);
    }
}
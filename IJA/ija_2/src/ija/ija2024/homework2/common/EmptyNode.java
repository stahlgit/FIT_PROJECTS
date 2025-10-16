package ija.ija2024.homework2.common;

import java.util.Set;

public class EmptyNode extends GameNode {
    public EmptyNode(Position position) {
        super(position);
    }

    @Override
    public boolean isBulb() {
        return false;
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
    }

    @Override
    public boolean containsConnector(Side s) {
        return false;
    }

    @Override
    public Set<Side> getConnectors() {
        return Set.of();
    }

    @Override
    public String toString() {
        return String.format("{E[%d@%d][]}", position.row(), position.col());
    }
}